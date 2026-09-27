//////////////////////////////////////////////////////////////////////////////////////
// e3menu.cpp - 
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
// 02/25/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "e3menu.h"
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS	

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
#include "player.h"
#include "launcher.h"
#include "CamManual.h"
#include "gamecam.h"
#include "fmovie2.h"
#include "fresload.h"
#include "fvis.h"
#include "wpr_system.h"
#include "wpr_datatypes.h"
#include "frenderer.h"
#include "wpr_drawutils.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif
#include "fcamanim.h"
#include "entity.h"
#include "fsh.h"
#include "eparticlepool.h"
#include "skybox.h"
#include "gcoll.h"
#include "flightgroup.h"
#include "ffile.h"
#include "fdatastreaming.h"

//====================
// private definitions

////////////////////
// general defines
#define _NUM_E3_SP_LEVELS						3
#define _NUM_E3_MP_LEVELS						2
#define _ATTRACT_MODE_TIMEOUT					45.0f
#define _FADE_IN_TIME							0.25f
#define _MENU_MUSIC_VOLUME						0.25f
#define _NUM_E3_MOVIES							1

static cchar *_pszLevelName = "wfhu_droid";
static cchar *_pszCSVFilename = "e3menu$";
static cchar *_pszMeshTableName = "Meshes";
static cchar *_pszCamFile = "cfhu_droid";
static cchar *_pszPhrasesTableName = "phrases";
static cchar *_pszMusicTableName = "music";

#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	static cchar *_apszDemoMovie[_NUM_E3_MOVIES] =  {
		"XB_Demo_Mov.bik"
	};
	static cchar * _pszTextureTableName = "XB_Textures";
#else
	static cchar *_apszDemoMovie[_NUM_E3_MOVIES] =  {
		"GC_Demo_Mov.bik"
	};
	static cchar * _pszTextureTableName = "GC_Textures";
#endif

typedef enum { 
	_MODE_PICK_SINGLE_PLAYER_LEVEL = 0,
	_MODE_CONTROLLER_CONFIG,
	_MODE_MULTIPLAYER_JOIN,
	_MODE_LEVEL_LOAD_ERROR,
	_MODE_ATTRACT_MOVIE,

	_MODE_COUNT
} _Modes_e;

typedef enum {
	_BOOT_MODE_CONSUMER_DEMO = 0,
	_BOOT_MODE_E3_SINGLE,
	_BOOT_MODE_E3_MULTI,

	_BOOT_MODE_COUNT
} _BootMode_e;

typedef enum {
	_TEXTURES_CONTROLLER = 0,
	_TEXTURES_A_BUTTON,
	_TEXTURES_B_BUTTON,
	_TEXTURES_Y_BUTTON,
	_TEXTURES_X_BUTTON,
	_TEXTURES_SAS_LOGO,
	_TEXTURES_SCREENSHOT_1,
	_TEXTURES_SCREENSHOT_2,
	_TEXTURES_SCREENSHOT_3,

	_TEXTURES_COUNT
} _Textures_e;

typedef enum {
	_PHRASES_CONFIGURATION = 0,
	_PHRASES_PRESS_A_TO_JOIN,
	_PHRASES_INVERTED,
	_PHRASES_NON_INVERTED,
	_PHRASES_PRESS_A_TO_PROCEED,
	_PHRASES_PRESS_A_TO_ACCEPT,
	_PHRASES_CHOOSE_CONTROLS,
	_PHRASES_WAITING_FOR_OTHER_PLAYERS,
	
	_PHRASES_COUNT
} _Phrases_e;

//////////////////////////////////////////////
// defines related to the level select screen
static cchar *_pszPLTextName = "Pick_Level_Text";
static cchar *_pszPLMeshName = "Pick_Level_Meshes";
static cchar *_pszPLButtonName = "Pick_Level_Buttons";

///////////////////////////////////////////////////
// defines related to the controller config screen
static cchar *_pszCCTextName = "controller_settings_text";
static cchar *_pszCCMeshName = "controller_settings_meshes";
static cchar *_pszCCButtonName = "controller_settings_buttons";
#define _DEMO_LEVEL_INDEX		0// THIS IS AN INDEX INTO THE E3 SELECTABLE LEVELS

//////////////////////////////////////////////////
// defines related to the multiplayer join screen
typedef enum {
	_MJ_STATE_ENTER = 0,
	_MJ_STATE_SELECT,
	_MJ_STATE_WAIT,

	_MJ_STATE_COUNT
} _MJ_States_e;
typedef struct {
	u16 nState;
	s16 nControls;
} _MJPlayerInfo_t;

static cchar *_pszMJButtonsName = "multi_join_Buttons";
static cchar *_pszMJTextName = "multi_join_text";
static cchar *_pszMJMeshName = "multi_join_meshes";

//=================
// public variables

//==================
// private variables

// general vars
static BOOL8 _bSystemInitialized = FALSE;
static BOOL8 _bNeedToUnload;
static u8 _nCurrentMode;
static u8 _nLastMode;
static BOOL8 _bInvertedControls;
static u8 _nControlMapIndex;
static BOOL8 _bNeedStartButtonPress;
static u8 _nMovieIndex;
static cchar *_pszLevelToLoad;
static FResFrame_t _ResFrame;
static FMemFrame_t _MemFrame;
static s16 _nCurSelection;
static s16 _nNumSelections;
static f32 _fTimer;
static CFTexInst *_paTexInsts;
static u32 _nNumMeshes;
static CFMeshInst *_paMeshInsts;
static CFStringTable *_pStringTable;
static CSkyBox *_pSkyBox;
static f32 _fFadeInTimer;
static BOOL8 _bPrevGovernorState;
static u8 _nBootMode;// will be set 1 time during InitSystem and will stay in that mode forever (see _BOOT_MODE_...)
static s8 _nControllerToLookAt;
static u8 _nCurrentMPLevel;

// sound vars
static CFAudioStream *_pAudioStream;
static FSndFx_FxHandle_t _hClick;
static FSndFx_FxHandle_t _hSelected;

// pick level vars
static Level_t *_apLevels[_NUM_E3_SP_LEVELS];
static Wpr_DataTypes_ScreenData_t _PLScreen;

// control config vars
static Wpr_DataTypes_ScreenData_t _CCScreen;
static WprSystem_ControllerConfigData_t _CCData;
static cwchar *_apwszPhrases[_PHRASES_COUNT];

// multiplayer join vars
static Wpr_DataTypes_ScreenData_t _MJScreen;
static Level_t *_apMultiplayerLevel[_NUM_E3_MP_LEVELS];
static _MJPlayerInfo_t _aMJPlayerInfo[MAX_PLAYERS];

// camera vars
static FViewport_t *_pViewportOrtho3D;
static CCamManualInfo _CamInfo;
static CFMtx43A _CamMtx;
static CFCamAnimInst *_pCamAnimInst;

//===================
// private prototypes

static BOOL _InitPickLevel( void );
static void _ResetVars( void );
static void _UpdateCurrentSelection( u32 nController, s32 nMaxSelections, BOOL bWrapAround, s16 &rnCurSelection );
static BOOL _Work( void );
static void _PickLevelWork( void );
static void _MovieWork( BOOL bNewControllerPluggedIn );
static void _ControllerConfigWork( void );
static void _MultiplayerWork( void );

static void _DrawPressStart( void );
static void _DrawScreenFadeIn( void );
static BOOL _Draw( void );
static void _DrawMovie( void );
static void _DrawSkyAndWorld( void );
static void _DrawLogos( f32 fScaleFactor );
static void _DrawPickLevelMenu( f32 fScaleFactor );
static void _DrawControllerConfigMenu( f32 fScaleFactor );
static void _DrawMultiPlayerMenu( f32 fScaleFactor );
static void _StartMovie( void );

//=================
// public functions

BOOL e3menu_InitSystem( void ) {

	// figure out what boot mode this is
	if( Launcher_bDemoLaunched ) {
		// we are a consumer single player demo
		_nBootMode = _BOOT_MODE_CONSUMER_DEMO;
	} else if( LAUNCHER_E3_SINGLE_PLAYER_VERSION ) {
		_nBootMode = _BOOT_MODE_E3_SINGLE;
	} else if( LAUNCHER_E3_MULTI_PLAYER_VERSION ) {
		_nBootMode = _BOOT_MODE_E3_MULTI;
	} else {
		// default to single player e3 version
		_nBootMode = _BOOT_MODE_E3_SINGLE;
	}

	_ResetVars();
	_bSystemInitialized = TRUE;
	
	return TRUE;
}

void e3menu_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		e3menu_End();
	}
	_ResetVars();
}

BOOL e3menu_Start( void ) {
	u32 i;

	FASSERT( _bSystemInitialized );

	// make sure a few items are reset
	_pszLevelToLoad = NULL;
	_nNumSelections = 0;
	for( i=0; i < _NUM_E3_SP_LEVELS; i++ ) {
		_apLevels[i] = NULL;
	}
	_paTexInsts = NULL;
	_nNumMeshes = 0;
	_paMeshInsts = NULL;
	_pStringTable = NULL;
	_pCamAnimInst = NULL;
	_pSkyBox = NULL;
	// stop any movie that may be playing
	fmovie2_Unload();

	// advance to the next mp level
	_nCurrentMPLevel++;
	if( _nCurrentMPLevel >= _NUM_E3_MP_LEVELS ) {
		_nCurrentMPLevel = 0;
	}
		
	gameloop_SetLoopHandlers( _Work, _Draw, _InitPickLevel );
	
	return TRUE;
}

void e3menu_End( void ) {
	u32 i;

	FASSERT( _bSystemInitialized );

	if( !_bNeedToUnload ) {
		return;
	}
	_bNeedToUnload = FALSE;

	fmovie2_Unload();
	fdelete_array( _paMeshInsts );
	_paMeshInsts = NULL;
	fdelete_array( _paTexInsts );
	_paTexInsts = NULL;
	_pStringTable = NULL;
	_pCamAnimInst = NULL;
	if( _pSkyBox ) {
		_pSkyBox->Destroy();
		fdelete( _pSkyBox );
		_pSkyBox = NULL;
	}

	gamecam_PostLevelCleanup();

	FDS_StreamMgr.Uninit();

	eparticlepool_UninitLevel();
	
	if( _pAudioStream ) {
		_pAudioStream->Destroy();
		_pAudioStream = NULL;
	}
	CGColl::ClearMaterialTable();

	fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( _MemFrame );

	fparticle_KillAllEmitters();

	_nNumSelections = 0;
	for( i=0; i < _NUM_E3_SP_LEVELS; i++ ) {
		_apLevels[i] = NULL;
	}
	
	floop_EnableGovernor( _bPrevGovernorState );
}

cchar *e3menu_GetNameOfSelectedLevel( void ) {
	FASSERT( _bSystemInitialized );

	return _pszLevelToLoad;
}

const GameInitInfo_t *e3menu_GetInitInfo( void ) {
	u32 i, nNumPlayers, nPlayerMask;
	GameInitInfo_t *pGameInitInfo;
	GameSave_MPRules_t *pRules;

	FASSERT( _bSystemInitialized );

	if( _nBootMode == _BOOT_MODE_E3_MULTI ) {
		nNumPlayers = 0;
		nPlayerMask = 0;
		for( i=0; i < MAX_PLAYERS; i++ ) {
			if( _aMJPlayerInfo[i].nState == _MJ_STATE_WAIT ) {
				// this player is in the game
				nPlayerMask |= (1 << i);
				nNumPlayers++;
			}
		}
		pGameInitInfo = (GameInitInfo_t *)wpr_system_CreateSimpleMultiPlayerGameInit( nNumPlayers, nPlayerMask, _apMultiplayerLevel[_nCurrentMPLevel]->nLevel );

		// fix up the profiles
		nNumPlayers = 0;
		for( i=0; i < MAX_PLAYERS; i++ ) {
			if( _aMJPlayerInfo[i].nState == _MJ_STATE_WAIT ) {
				// this player is in the game
				if( _aMJPlayerInfo[i].nControls ) {
					FMATH_CLEARBITMASK( pGameInitInfo->apProfile[nNumPlayers]->m_Data.nFlags, GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG );
				} else {
					FMATH_SETBITMASK( pGameInitInfo->apProfile[nNumPlayers]->m_Data.nFlags, GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG );
				}

				pGameInitInfo->apProfile[nNumPlayers]->m_Data.nControllerConfigIndex = 0;
				pGameInitInfo->apProfile[nNumPlayers]->m_nControllerIndex = i;

				nNumPlayers++;
			}
		}
		// set the MP rules
		pRules = (GameSave_MPRules_t *)pGameInitInfo->pMultiplayerRules;
		pRules->nFlags = GAMESAVE_RULE_FLAGS_RADAR_ON | GAMESAVE_RULE_FLAGS_MARKERS_ON;
		pRules->nLimitPrimaryWeapon = GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT;
		pRules->nLimitSecondaryWeapon = GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT;
		pRules->nDMKillsPerRound = 15;
		pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;
		pRules->nTimeLimit = 5;
		pRules->nTimeBetweenHillSwaps = 5;
		pRules->nLimitVehicles = GAMESAVE_VEHICLE_LIMIT_ALL_VEHICLES;
	} else {
		// all other modes are single player ones
		pGameInitInfo = (GameInitInfo_t *)wpr_system_CreateSimpleSinglePlayerGameInit( _bInvertedControls );

		if( pGameInitInfo ) {
			pGameInitInfo->bNewGame = FALSE;
			pGameInitInfo->bReplayingALevel = TRUE;
			pGameInitInfo->nDifficultyLevel = GAMESAVE_DIFFICULTY_NORMAL;
			pGameInitInfo->apProfile[0]->m_nControllerIndex = _nControllerToLookAt;
			pGameInitInfo->apProfile[0]->m_Data.nControllerConfigIndex = _nControlMapIndex;
		}
	}
	return pGameInitInfo;
}

//==================
// private functions

static BOOL _InitPickLevel( void ) {
	FGameDataFileHandle_t hFile;
	CFQuatA Quat;
	CFVec3 Pos;
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTable;
	cwchar *pwszText;
	cchar *pszText;

	_pAudioStream = NULL;
	_hClick = fsndfx_GetFxHandle( "Hud Click" );
	_hSelected = fsndfx_GetFxHandle( "Hud Select" );

	// clear out any level that may be loaded
	game_UnloadLevel();

	// grab memory frames...
	_ResFrame = fres_GetFrame();
	_MemFrame = fmem_GetFrame();

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( "e3menu" );
#endif
	DEVPRINTF( "******** LOAD MARKER - START E3 WRAPPERS***********.\n" );

	_bNeedToUnload = TRUE;

	_fTimer = 0.0f;
	_bInvertedControls = FALSE;
	_nControlMapIndex = 0;
	_bNeedStartButtonPress = TRUE;
	_fFadeInTimer = 0.0f;
	_nMovieIndex = 0;
	_nControllerToLookAt = -1;

	floop_Reset();

	//////////////////////////////
	// fill in the allocated table
	s32 i;
	_nNumSelections = 0;
	for( i=1; i < Level_nCount; i++ ) {
		if( fclib_strstr( Level_aInfo[i].pszTitle, " E3 " ) ) {
			_apLevels[_nNumSelections] = &Level_aInfo[i];
			_nNumSelections++;
			if( _nNumSelections >= _NUM_E3_SP_LEVELS ) {
				break;
			}
		}
	}
	FASSERT( _nNumSelections );	

	// find the multiplayer level 
	u32 nCount = 0;
	for( i=LEVEL_SINGLE_PLAYER_COUNT; i < Level_nCount; i++ ) {
		if( Level_aInfo[i].nLevel == LEVEL_MULTIPLAYER_1 ) {
			_apMultiplayerLevel[nCount++] = &Level_aInfo[i];
		}
		if( Level_aInfo[i].nLevel == LEVEL_MULTIPLAYER_2 ) {
			_apMultiplayerLevel[nCount++] = &Level_aInfo[i];
		}
		if( nCount == _NUM_E3_MP_LEVELS ) {
			break;
		}
	}
	FASSERT( nCount == _NUM_E3_MP_LEVELS );

	CGColl::LoadMaterialTable( "mset_std" );

	//////////////////////////////////////////////////
	// load the wrapper csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _pszCSVFilename );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not load the e3menu csv file '%s'.\n", _pszCSVFilename );
		goto _EXIT_WITH_ERROR;
	}

	//////////////////////////////////////
	// load the memory cache data
	level_LoadMemoryCacheData( hFile );

	//////////////////////////////////////
	// load the fog info from the csv file
	level_LoadFogData( hFile );

	//////////////////////////////////////////////
	// load the reflection data from the csv file
	level_LoadReflectionData( hFile );

	//////////////////////
	// load the world mesh
	if( !fresload_Load( FWORLD_RESTYPE, _pszLevelName ) ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not load the wrapper world file '%s'.\n", _pszLevelName );
		goto _EXIT_WITH_ERROR;
	}

	eparticlepool_InitLevel();

	////////////////////
	// create a viewport
	_pViewportOrtho3D = fviewport_Create();
	if( !_pViewportOrtho3D ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not create an rendering viewport.\n" );
		goto _EXIT_WITH_ERROR;
	}
	fviewport_InitOrtho3D( _pViewportOrtho3D, 0.1f, 100.0f );

	/////////////////////////////
	// create and init a sky box
	_pSkyBox = fnew CSkyBox;
	if( !_pSkyBox ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not create a sky box.\n" );
		goto _EXIT_WITH_ERROR;
	}
	_pSkyBox->CreateFromCSV( hFile ); 

	///////////////////////////////////////////////
	// allocate enough room for the needed textures
	_paTexInsts = wpr_datatypes_InitTextureArray( hFile, _pszTextureTableName, _TEXTURES_COUNT );
	if( !_paTexInsts ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not load the required textures.\n" );
		goto _EXIT_WITH_ERROR;
	}

	/////////////////////////////////////////////
	// allocate enough room for the needed meshes
	_paMeshInsts = wpr_datatypes_InitMeshArray( hFile, _pszMeshTableName, _nNumMeshes );
	if( !_paMeshInsts ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not load the required meshes.\n" );
		goto _EXIT_WITH_ERROR;
	}

	/////////////////////////////////
	// create the string table to use
	_pStringTable = CFStringTable::CreateTable( _pszCSVFilename, TRUE );
	if( !_pStringTable ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not create a string table for all of the strings.\n" );
		goto _EXIT_WITH_ERROR;
	}

	//////////////////////////////////////////////////////////
	// find the phrases table and add them to the string table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszPhrasesTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not find the phrases table named '%s'.\n", _pszPhrasesTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTable );
	if( i != _PHRASES_COUNT ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : The phrases table in '%s' didn't contain %d strings as expected.\n", _pszPhrasesTableName, _PHRASES_COUNT );
		goto _EXIT_WITH_ERROR;
	}
	// walk the string table adding each string to the string table and recording a ptr to them
	for( i=0; i < _PHRASES_COUNT; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		_apwszPhrases[i] = _pStringTable->AddString( pwszText );
	}	

	//////////////////////////////////
	// init the pick level screen data

	// init the text layout array
	if( !wpr_datatypes_InitScreenTextLayoutArray( hFile, _pszPLTextName, _PLScreen, *_pStringTable ) ) { 
		// failed to init the text layout array
		goto _EXIT_WITH_ERROR;
	}	
	// init the mesh layout array
	if( !wpr_datatypes_InitScreenMeshLayoutArray( hFile, _pszPLMeshName, _PLScreen, _nNumMeshes, _paMeshInsts ) ) {
		// failed to init the mesh layout array
		goto _EXIT_WITH_ERROR;
	}
	// init the button layout array
	if( !wpr_datatypes_InitScreenButtonLayoutArray( hFile, _pszPLButtonName, _PLScreen, *_pStringTable,
													&_paTexInsts[_TEXTURES_A_BUTTON],
													&_paTexInsts[_TEXTURES_B_BUTTON],
													&_paTexInsts[_TEXTURES_Y_BUTTON],
													&_paTexInsts[_TEXTURES_X_BUTTON] ) ) { 
		// failed to init the text layout array
		goto _EXIT_WITH_ERROR;
	}

	////////////////////////////////////////
	// init the multiplayer join screen data

	// init the text layout array
	if( !wpr_datatypes_InitScreenTextLayoutArray( hFile, _pszMJTextName, _MJScreen, *_pStringTable ) ) { 
		// failed to init the text layout array
		goto _EXIT_WITH_ERROR;
	}	
	// init the mesh layout array
	if( !wpr_datatypes_InitScreenMeshLayoutArray( hFile, _pszMJMeshName, _MJScreen, _nNumMeshes, _paMeshInsts ) ) {
		// failed to init the mesh layout array
		goto _EXIT_WITH_ERROR;
	}
	// init the button layout array
	if( !wpr_datatypes_InitScreenButtonLayoutArray( hFile, _pszMJButtonsName, _MJScreen, *_pStringTable,
													&_paTexInsts[_TEXTURES_A_BUTTON],
													&_paTexInsts[_TEXTURES_B_BUTTON],
													&_paTexInsts[_TEXTURES_Y_BUTTON],
													&_paTexInsts[_TEXTURES_X_BUTTON] ) ) { 
		// failed to init the text layout array
		goto _EXIT_WITH_ERROR;
	}

	for( i=0; i < MAX_PLAYERS; i++ ) {
		_aMJPlayerInfo[i].nControls = 0;
		_aMJPlayerInfo[i].nState = _MJ_STATE_ENTER;
	}

	/////////////////////////////////////////
	// init the controller config screen data

	if( !wpr_system_InitControllerConfigData( hFile, &_CCData, *_pStringTable ) ) {
		goto _EXIT_WITH_ERROR;
	}
	// init the text layout array
	if( !wpr_datatypes_InitScreenTextLayoutArray( hFile, _pszCCTextName, _CCScreen, *_pStringTable ) ) { 
		// failed to init the text layout array
		goto _EXIT_WITH_ERROR;
	}	
	// init the mesh layout array
	if( !wpr_datatypes_InitScreenMeshLayoutArray( hFile, _pszCCMeshName, _CCScreen, _nNumMeshes, _paMeshInsts ) ) {
		// failed to init the mesh layout array
		goto _EXIT_WITH_ERROR;
	}
	// init the button layout array
	if( !wpr_datatypes_InitScreenButtonLayoutArray( hFile, _pszCCButtonName, _CCScreen, *_pStringTable,
													&_paTexInsts[_TEXTURES_A_BUTTON],
													&_paTexInsts[_TEXTURES_B_BUTTON],
													&_paTexInsts[_TEXTURES_Y_BUTTON],
													&_paTexInsts[_TEXTURES_X_BUTTON] ) ) { 
		// failed to init the text layout array
		goto _EXIT_WITH_ERROR;
	}

	////////////////
	// Music & Audio
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszMusicTableName );
	if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		pszText = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
		_pAudioStream = CFAudioStream::Create( pszText );
	}

	//////////////////////
	// init the 3d camera
	_pCamAnimInst = CFCamAnimInst::Load( _pszCamFile );
	if( !_pCamAnimInst ) {
		DEVPRINTF( "e3menu::_InitPickLevel() : Could not load the camera animation file %s.\n", _pszCamFile  );
		goto _EXIT_WITH_ERROR;
	}
	_pCamAnimInst->UpdateUnitTime( fmath_RandomFloat() * 0.75f );
	_pCamAnimInst->ComputeFrame( _CamInfo.m_fHalfFOV, Quat, Pos );
	Quat.BuildMtx( _CamMtx );
	_CamMtx.m_vPos.Set( Pos );
	
	_CamInfo.m_pmtxMtx = &_CamMtx;

	// init the gamecam system
	gamecam_InitLevelCameras();
	gamecam_SwitchPlayerToManualCamera( GAME_CAM_PLAYER_1, &_CamInfo );
	gamecam_SetActiveCamera( GAME_CAM_PLAYER_1 );

	floop_Reset();
	CEntity::ResolveEntityPointerFixups();

	// clear any pending text prints
	ftext_ClearAllPending();

	_bPrevGovernorState = floop_IsGovernorEnabled();
	floop_EnableGovernor( TRUE );

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( _MemFrame );

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END E3 WRAPPERS***********.\n" );

	return TRUE;

_EXIT_WITH_ERROR:
	fdelete_array( _paMeshInsts );
	_paMeshInsts = NULL;
	fdelete_array( _paTexInsts );
	_paTexInsts = NULL;
	if( _pSkyBox ) {
		_pSkyBox->Destroy();
		fdelete( _pSkyBox );
		_pSkyBox = NULL;
	}
	_pStringTable = NULL;
	CGColl::ClearMaterialTable();
    fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( _MemFrame );

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END E3 WRAPPERS***********.\n" );

	launcher_LoadFailure();
	return TRUE;
	//return FALSE;// don't return this because it will break out of the gameloop, we will stay in a message box state
}

static void _ResetVars( void ) {
	u32 i;

	_bSystemInitialized = FALSE;
	_bNeedToUnload = FALSE;

	switch( _nBootMode ) {
	
	case _BOOT_MODE_CONSUMER_DEMO:
		_nCurrentMode = _MODE_CONTROLLER_CONFIG;
		_nLastMode = _MODE_CONTROLLER_CONFIG;
		break;

	case _BOOT_MODE_E3_MULTI:
		_nCurrentMode = _MODE_MULTIPLAYER_JOIN;
		_nLastMode = _MODE_MULTIPLAYER_JOIN;
		break;

	default:
		FASSERT_NOW;
	case _BOOT_MODE_E3_SINGLE:
		_nCurrentMode = _MODE_PICK_SINGLE_PLAYER_LEVEL;
		_nLastMode = _MODE_PICK_SINGLE_PLAYER_LEVEL;
		break;
	}

	_pszLevelToLoad = NULL;
	_pAudioStream = NULL;
	_nCurSelection = 0;
	_nNumSelections = 0;
	for( i=0; i < _NUM_E3_SP_LEVELS; i++ ) {
        _apLevels[i] = NULL;
	}
    _fTimer = 0.0f;
	_paTexInsts = NULL;
	_nNumMeshes = 0;
	_paMeshInsts = NULL;
	_pStringTable = NULL;
	_pCamAnimInst = NULL;
	_pSkyBox = NULL;
	_bNeedStartButtonPress = FALSE;
	_fFadeInTimer = 0.0f;
	_pViewportOrtho3D = NULL;
	_pCamAnimInst = NULL;
	_nMovieIndex = 0;

	_apMultiplayerLevel[0] = _apMultiplayerLevel[1] = NULL;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		_aMJPlayerInfo[i].nControls = 0;
		_aMJPlayerInfo[i].nState = _MJ_STATE_ENTER;
	}
	_nControllerToLookAt = -1;
	_nCurrentMPLevel = (_NUM_E3_MP_LEVELS-1);
}

static void _UpdateCurrentSelection( u32 nController, s32 nMaxSelections, BOOL bWrapAround, s16 &rnCurSelection ) {
	if( Gamepad_aapSample[nController][GAMEPAD_MENU_LEFT_ANALOG_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[nController][GAMEPAD_MENU_LEFT_ANALOG_X]->fCurrentState > 0.1f ) {
			rnCurSelection++;			
		} else {
			rnCurSelection--;
		}		
	} else if( Gamepad_aapSample[nController][GAMEPAD_MENU_DPAD_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[nController][GAMEPAD_MENU_DPAD_X]->fCurrentState > 0.1f ) {
			rnCurSelection++;
		} else {
			rnCurSelection--;
		}
	}

	if( rnCurSelection < 0 ) {
		rnCurSelection = (bWrapAround) ? nMaxSelections-1 : 0;
	} else if( rnCurSelection >= nMaxSelections ) {
		rnCurSelection = (bWrapAround) ? 0 : nMaxSelections-1;
	}
}

static BOOL _Work( void ) {

	FASSERT( _bSystemInitialized );

	BOOL bNewControllerPluggedIn = FALSE;
	u32 nOldPortMask = Gamepad_nPortOnlineMask;
	gamepad_Sample();
	if( Gamepad_nPortOnlineMask != nOldPortMask ) {
		bNewControllerPluggedIn = TRUE;
	}

	if( _nCurrentMode != _MODE_ATTRACT_MOVIE ) {
		CEntity::CallAllAutoWorks();

		smoketrail_WorkAll();

		// do our camera work
		_pCamAnimInst->DeltaTime( FLoop_fPreviousLoopSecs * 0.1f, FALSE );
		
		CFQuatA Quat;
		CFVec3 Pos;
		_pCamAnimInst->ComputeFrame( _CamInfo.m_fHalfFOV, Quat, Pos );
		Quat.BuildMtx( _CamMtx );
		_CamMtx.m_vPos.Set( Pos );

		// update the 3d location of the audio listener
		CFXfm xfmListener;
		xfmListener.BuildFromMtx( _CamMtx );
		faudio_SetListenerOrientation( 0, &xfmListener );
	    
		fsh_RegisterPlayerPos( _CamMtx.m_vPos );

		_pSkyBox->Work();

		gamecam_Work();	

		CEntity::CallAllMarkedRemoveFromWorld();
		fparticle_Work();
		CFLightGroupMgr::Work();
	}

	switch( _nCurrentMode ) {
	
	case _MODE_PICK_SINGLE_PLAYER_LEVEL:
		// start the music
		if( _pAudioStream &&
			_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );		
			_pAudioStream->Play( 0 );
		}
		_PickLevelWork();	
		break;

	case _MODE_CONTROLLER_CONFIG:
		// start the music
		if( _pAudioStream &&
			_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );		
			_pAudioStream->Play( 0 );
		}
		_ControllerConfigWork();
		break;

	case _MODE_MULTIPLAYER_JOIN:
		// start the music
		if( _pAudioStream &&
			_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );		
			_pAudioStream->Play( 0 );
		}
		_MultiplayerWork();
		break;

	case _MODE_ATTRACT_MOVIE:
		_MovieWork( bNewControllerPluggedIn );
		break;

	case _MODE_LEVEL_LOAD_ERROR:
		if( Gamepad_aapSample[_nControllerToLookAt][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			_nCurrentMode = _nLastMode;
		}
		break;
		
	default:
		FASSERT_NOW;
		break;
	}
		
	return TRUE;	
}

static void _PickLevelWork( void ) {
	BOOL bAccept, bBack;
	s32 nOldSelection = _nCurSelection;

	if( _bNeedStartButtonPress ) {
		_nControllerToLookAt = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		
		if( _nControllerToLookAt > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
			_bNeedStartButtonPress = FALSE;
		}
	} else {
		// make sure the controller is still plugged in
		if( (Gamepad_nPortOnlineMask & (1 << _nControllerToLookAt)) == 0 ) {
			// controller was pulled
			_nControllerToLookAt = -1;
			_bNeedStartButtonPress = TRUE;
			return;
		}
		bAccept = Gamepad_aapSample[_nControllerToLookAt][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;
		bBack = Gamepad_aapSample[_nControllerToLookAt][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;

		// see if a selection has been made
		if( bAccept || bBack ) {
			// record the currently selected level name
			fsndfx_Play2D( _hSelected );
			_pszLevelToLoad = _apLevels[_nCurSelection]->pszTitle;
			_bInvertedControls = (bAccept) ? TRUE : FALSE;
			if( !launcher_StartGame( LAUNCHER_FROM_E3, FALSE ) ) {
				// setup an error message and go back into the menus
				gameloop_SetLoopHandlers( _Work, _Draw, _InitPickLevel );
				_nLastMode = _nCurrentMode;
				_nCurrentMode = _MODE_LEVEL_LOAD_ERROR;
				_InitPickLevel();
			}
			return;
		}	

		// see if we should change the current selection (left/right)
		_UpdateCurrentSelection( _nControllerToLookAt, _nNumSelections, TRUE, _nCurSelection );		
	}

	if( nOldSelection != _nCurSelection ) {
		fsndfx_Play2D( _hClick );
		_fTimer = 0.0f;
	} else {
		if( wpr_system_FindActiveControllerPort( FALSE, TRUE, TRUE ) > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
			// the stick moved, reset the timer
			_fTimer = 0.0f;
		} else {
			_fTimer += FLoop_fPreviousLoopSecs;
			if( _fTimer >= _ATTRACT_MODE_TIMEOUT ) {
				// switch into attract mode
				_StartMovie();
			}
		}
	}
}

static void _MovieWork( BOOL bNewControllerPluggedIn ) {
	BOOL bExitMovie = FALSE;
	u32 i;
	BOOL bOtherButtonPressed = FALSE;

	_nControllerToLookAt = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
	if( _nControllerToLookAt > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
		_bNeedStartButtonPress = TRUE;
		bExitMovie = TRUE;
	} else {
		// check all other buttons for press to exit movie
		for( i=0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) {
            if( Gamepad_aapSample[i][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
				Gamepad_aapSample[i][GAMEPAD_MENU_LEFT_SHOULDER]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
				Gamepad_aapSample[i][GAMEPAD_MENU_RIGHT_SHOULDER]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
				Gamepad_aapSample[i][GAMEPAD_MENU_BUTTON_TOP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
				Gamepad_aapSample[i][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
				Gamepad_aapSample[i][GAMEPAD_MENU_EXTRA_BUTTON]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
				Gamepad_aapSample[i][GAMEPAD_MENU_BACK2]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
				// a button was pressed
				bOtherButtonPressed = TRUE;
				break;
			}
		}
	}
	if( fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING ||
		bNewControllerPluggedIn ||
		bOtherButtonPressed ) {
		bExitMovie = TRUE;
		_bNeedStartButtonPress = TRUE;
		_nControllerToLookAt = -1;
	}
	if( bExitMovie ) {
		// either a button was pressed or the movie is done, go back to the menu
		fmovie2_Unload();
		_fTimer = 0.0f;
		_fFadeInTimer = 0.0f;

		if ( Launcher_bDemoLaunched ) {
			// restart the idle timer again
			gameloop_EnableIdleTimer( TRUE );
		}

		for( i=0; i < MAX_PLAYERS; i++ ) {
			_aMJPlayerInfo[i].nControls = 0;
			_aMJPlayerInfo[i].nState = _MJ_STATE_ENTER;
		}

		if( _pAudioStream ) {
			_pAudioStream->Pause( FALSE );
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );
		}

		// return to the last mode
		_nCurrentMode = _nLastMode;

	} else {
		_fTimer += FLoop_fPreviousLoopSecs;
	}
}

static void _ControllerConfigWork( void ) {
	BOOL bAccept, bBack, bYButton;
	s32 nOldSelection = _nCurSelection;

	if( _bNeedStartButtonPress ) {
		_nControllerToLookAt = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		
		if( _nControllerToLookAt > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
			_bNeedStartButtonPress = FALSE;
		}
	} else {
		// make sure the controller is still plugged in
		if( (Gamepad_nPortOnlineMask & (1 << _nControllerToLookAt)) == 0 ) {
			// controller was pulled
			_nControllerToLookAt = -1;
			_bNeedStartButtonPress = TRUE;
			return;
		}

		bAccept = Gamepad_aapSample[_nControllerToLookAt][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;
		bBack = Gamepad_aapSample[_nControllerToLookAt][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;
		bYButton = Gamepad_aapSample[_nControllerToLookAt][GAMEPAD_MENU_BUTTON_TOP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;

		// see if a selection has been made
		if( bAccept || bBack ) {
			// record the currently selected level name
			fsndfx_Play2D( _hSelected );
			_pszLevelToLoad = _apLevels[_DEMO_LEVEL_INDEX]->pszTitle;
			_bInvertedControls = (bAccept) ? TRUE : FALSE;
			_nControlMapIndex = (u8)_nCurSelection;
			if( !launcher_StartGame( LAUNCHER_FROM_E3, FALSE ) ) {
				// setup an error message and go back into the menus
				gameloop_SetLoopHandlers( _Work, _Draw, _InitPickLevel );
				_nLastMode = _nCurrentMode;
				_nCurrentMode = _MODE_LEVEL_LOAD_ERROR;
				_InitPickLevel();
			}
			return;
		}

		if( bYButton ) {
			// exit the gameloop
			gameloop_ScheduleExit();
			return;
		}

#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
		// see if we should change the current selection (left/right)
		_UpdateCurrentSelection( _nControllerToLookAt, _CCData.nCCNumConfigs, FALSE, _nCurSelection );		
#endif
	}

	if( nOldSelection != _nCurSelection ) {
		fsndfx_Play2D( _hClick );
		_fTimer = 0.0f;
	} else {
		if( wpr_system_FindActiveControllerPort( FALSE, TRUE, TRUE ) > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
			// the stick moved, reset the timer
			_fTimer = 0.0f;
		} else {
			_fTimer += FLoop_fPreviousLoopSecs;
			if( _fTimer >= _ATTRACT_MODE_TIMEOUT ) {
				// switch into attract mode
				_StartMovie();
			}
		}
	}
}

static void _MultiplayerWork( void ) {
	BOOL bAccept, bBack;
	BOOL bPlaySound = FALSE;
	u32 i, nLast;
	BOOL bFinished = FALSE;

	if( _bNeedStartButtonPress ) {
		if( wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE ) > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
			_bNeedStartButtonPress = FALSE;
		}
	} else {
		// do work for each player
		for( i=0; i < MAX_PLAYERS; i++ ) {
			bAccept = Gamepad_aapSample[i][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;
			bBack = Gamepad_aapSample[i][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK;

			if( bBack ) {
				bPlaySound = TRUE;
				// move this player back a state
				if( _aMJPlayerInfo[i].nState ) {
					_aMJPlayerInfo[i].nState--;
				}
			} else if( bAccept ) {
				bPlaySound = TRUE;

				// move this player up a state
				switch( _aMJPlayerInfo[i].nState ) {
				case _MJ_STATE_ENTER:
					_aMJPlayerInfo[i].nControls = 0;
					_aMJPlayerInfo[i].nState = _MJ_STATE_SELECT;
					break;
				case _MJ_STATE_SELECT:
					_aMJPlayerInfo[i].nState = _MJ_STATE_WAIT;
					break;
				case _MJ_STATE_WAIT:
					bFinished = TRUE;
					break;
				}				
			} else if( _aMJPlayerInfo[i].nState == _MJ_STATE_SELECT ) {
				// check the L/R controls
				nLast = _aMJPlayerInfo[i].nControls;
				_UpdateCurrentSelection( i, 2, FALSE, _aMJPlayerInfo[i].nControls );	
				if( _aMJPlayerInfo[i].nControls != nLast ) {
					bPlaySound = TRUE;
				}
			}
		}

		// see if we are finished
		if( bFinished ) {
			// make sure that everyone else is either in enter or in wait
			for( i=0; i < MAX_PLAYERS; i++ ) {
				if( _aMJPlayerInfo[i].nState == _MJ_STATE_SELECT ) {
					bFinished = FALSE;
					break;
				}
			}
		}

        // see if we should finish
		if( bFinished ) {
			// record the currently selected level name
			fsndfx_Play2D( _hSelected );
			_pszLevelToLoad = _apMultiplayerLevel[_nCurrentMPLevel]->pszTitle;
			
			if( !launcher_StartGame( LAUNCHER_FROM_E3, FALSE ) ) {
				// setup an error message and go back into the menus
				gameloop_SetLoopHandlers( _Work, _Draw, _InitPickLevel );
				_nLastMode = _nCurrentMode;
				_nCurrentMode = _MODE_LEVEL_LOAD_ERROR;
				_InitPickLevel();
			}
			return;
		}
	}

	if( bPlaySound ) {
		// something changed so play a sound and reset the timer
		fsndfx_Play2D( _hClick );
		_fTimer = 0.0f;
	} else {
		if( wpr_system_FindActiveControllerPort( FALSE, TRUE, TRUE ) > WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN ) {
			// the stick moved, reset the timer
			_fTimer = 0.0f;
		} else {
			_fTimer += FLoop_fPreviousLoopSecs;
			if( _fTimer >= _ATTRACT_MODE_TIMEOUT ) {
				// switch into attract mode
				_StartMovie();
			}
		}
	}
}

static void _DrawPressStart( void ) {
	u32 nSecs = (u32)_fTimer;
	if( nSecs & 0x3 ) {
		if( Gamepad_nPortOnlineMask ) {
			cwchar *pwszStartPrompt = L"Press START";
#if defined(MA_PC_INPUT)
			const PcPromptStyle nStyle = pcinput_PromptStyleForPort( _nControllerToLookAt >= 0 ? (u32)_nControllerToLookAt : pcinput_KeyboardPort() );
			if( nStyle == PCINPUT_PROMPT_STYLE_KEYBOARD ) {
				pwszStartPrompt = L"Press Enter";
			} else if( nStyle == PCINPUT_PROMPT_STYLE_PLAYSTATION ) {
				pwszStartPrompt = L"Press OPTIONS";
			}
#endif
			ftext_Printf( 0.5f, 0.65f, 
						L"~f1~C%ls~w0~ac~s1.20%ls",
						WprDataTypes_pwszPressStartColor,
						pwszStartPrompt );
		} else {
			ftext_Printf( 0.5f, 0.65f, 
						L"~f1~C%ls~w0~ac~s1.20%ls",
						WprDataTypes_pwszPressStartColor,
						L"Insert Controller" );
		}
	}
}

static void _DrawScreenFadeIn( void ) {

	if( _fFadeInTimer >= _FADE_IN_TIME ) {
		return;
	}
	f32 fUnitPercent = _fFadeInTimer * (1.0f/_FADE_IN_TIME);
	FMATH_CLAMPMAX( fUnitPercent, 1.0f );
	fUnitPercent = fmath_UnitLinearToSCurve( fUnitPercent );

	game_DrawSolidFullScreenOverlay( fUnitPercent, 0.0f );
	
	_fFadeInTimer += FLoop_fPreviousLoopSecs;
}

static BOOL _Draw( void ) {
	FViewport_t *pPrevViewport;
	f32 fScaleFactor;
	
	FASSERT( _bSystemInitialized );

	//If rendering the scene, handle render targets for shadows and liquid reflection.
	if ( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL || _nCurrentMode == _MODE_CONTROLLER_CONFIG || _nCurrentMode == _MODE_MULTIPLAYER_JOIN ) {
		//Handle render targets.
		ftex_HandleRenderTargets();
	}

	// setup the 3d viewport and init the camera stack
	gamecam_SetViewportAndInitCamStack();

	// Clear the viewport to black...
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	fScaleFactor = _pViewportOrtho3D->Res.x * (1.0f/640.0f);	

	switch( _nCurrentMode ) {
		
	case _MODE_LEVEL_LOAD_ERROR:
#if defined(MA_PC_INPUT)
		{
			const PcPromptStyle nStyle = pcinput_PromptStyleForPort( _nControllerToLookAt >= 0 ? (u32)_nControllerToLookAt : pcinput_KeyboardPort() );
			cwchar *pwszAccept = nStyle == PCINPUT_PROMPT_STYLE_KEYBOARD ? L"Space" :
				( nStyle == PCINPUT_PROMPT_STYLE_PLAYSTATION ? L"Cross" : L"A" );
			ftext_Printf( 0.5f, 0.20f, "~f0~C92929299~w1~ac~s0.85Error: Could not load\n\n%s.\n\nPress %ls to continue.", _pszLevelToLoad, pwszAccept );
		}
#else
		ftext_Printf( 0.5f, 0.20f, "~f0~C92929299~w1~ac~s0.85Error: Could not load\n\n%s.\n\nPress 'A' to continue.", _pszLevelToLoad );
#endif
		break;

	case _MODE_ATTRACT_MOVIE:
		_DrawMovie();	
		break;

	case _MODE_PICK_SINGLE_PLAYER_LEVEL:	
	case _MODE_CONTROLLER_CONFIG:
	case _MODE_MULTIPLAYER_JOIN:
		_DrawSkyAndWorld();
		
		// set our ortho viewport as active
		pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );

		FXfm_Identity.InitStackWithView();

		// clear the z buffer so that none of our stuff clips into anything
		fviewport_Clear( FVIEWPORT_CLEARFLAG_DEPTH | FVIEWPORT_CLEARFLAG_STENCIL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

		if( !_bNeedStartButtonPress && _fFadeInTimer >= _FADE_IN_TIME ) {	

			switch( _nBootMode ) {

			case _BOOT_MODE_CONSUMER_DEMO:
				_DrawControllerConfigMenu( fScaleFactor );
				break;

			case _BOOT_MODE_E3_MULTI:
				_DrawMultiPlayerMenu( fScaleFactor );
				break;

			case _BOOT_MODE_E3_SINGLE:
				_DrawPickLevelMenu( fScaleFactor );
				break;
			}

			_DrawScreenFadeIn();
		} else {
			_DrawLogos( fScaleFactor );
			_DrawScreenFadeIn();
			_DrawPressStart();
		}
		// restore the viewport
		fviewport_SetActive( pPrevViewport );
		break;

	default:
		FASSERT_NOW;
		break;
	}

	if( _nCurrentMode != _MODE_ATTRACT_MOVIE &&
		_nBootMode != _BOOT_MODE_CONSUMER_DEMO ) {
		// draw the version number in the lower right corner of the screen
		ftext_Printf( 0.68f, 0.67f, "~f1~C92929299~w1~al~s0.69Ver %d.%d.%d", fversion_GetToolMajorVer(), 
																			fversion_GetToolMinorVer(),
																			fversion_GetToolSubVer() );
	}

	return TRUE;
}

static void _DrawMovie( void ) {
	FViewport_t *pPrevViewport;

	FXfm_Identity.InitStackWithView();
	pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );
	
	//fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 1.0f, 1.0f, 1.0f, 1.0f, 0 );

	fmovie2_Draw();			

	_DrawPressStart();

	fviewport_SetActive( pPrevViewport );
}

static void _DrawSkyAndWorld( void ) {
	
	// draw the sky box
	frenderer_Push( FRENDERER_MESH, NULL );
	_pSkyBox->Draw( &_CamMtx.m_vPos );
	frenderer_Pop();

	// draw the world
	fvis_FrameBegin();
	fvis_Draw( NULL, FALSE, 0 );
	fvis_FrameEnd();
}

static void _DrawLogos( f32 fScaleFactor ) {

	frenderer_Push( FRENDERER_MESH, NULL );

	// DRAW THE MA LOGO
	Wpr_DataTypes_MeshLayout_t Layout;
	Layout.pMeshInst = wpr_datatypes_FindMeshInst( "gfh_logo01", _nNumMeshes, _paMeshInsts );
	if( Layout.pMeshInst ) {
		Layout.fBiPolarUnitX = 0.0f;
		Layout.fBiPolarUnitY = 0.65f;
		Layout.fDrawZ = WPR_DATATYPES_FURTHEST_LAYER_Z;
		Layout.fScale = 1.35f; 
				
		wpr_drawutils_DrawMesh_WithRot( &Layout, 
			fScaleFactor, 
			_pViewportOrtho3D->HalfRes.x, 
			_pViewportOrtho3D->HalfRes.y, 
			0.0f, 0.0f, 0.0f );
	}
	frenderer_Pop();

	// draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );
    // DRAW THE SAS LOGO
	wpr_drawutils_DrawTextureToScreen( TRUE,
						&_paTexInsts[_TEXTURES_SAS_LOGO],
						-0.73f, -0.64f,
						0.5f,
						fScaleFactor,
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.y );
	// push the fdraw renderer off 
	frenderer_Pop();
}

static void _DrawPickLevelMenu( f32 fScaleFactor ) {
	u32 i;
	f32 fX;

	// push the mesh renderer
	frenderer_Push( FRENDERER_MESH, NULL );
	fmesh_Ambient_Set( 0.5f, 0.5f, 0.5f, 1.0f );
	wpr_system_DrawBasicScreen( &_PLScreen,
		1 + _nCurSelection,
		FALSE,
		fScaleFactor,
		_pViewportOrtho3D->HalfRes.x,
		_pViewportOrtho3D->HalfRes.y );
	// pop the mesh renderer off
	frenderer_Pop();

	// draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );

	// draw the current screenshot
	fX = -0.45f;
	for( i=0; i < (u32)_nNumSelections; i++ ) {
		if( i == _nCurSelection ) {
			wpr_drawutils_DrawTextureToScreen( TRUE,
						&_paTexInsts[_TEXTURES_SCREENSHOT_1 + i],
						fX, 0.0f,
						0.65f,
						fScaleFactor,
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.y );
		} else {
			wpr_drawutils_DrawTextureToScreen( FALSE,
						&_paTexInsts[_TEXTURES_SCREENSHOT_1 + i],
						fX * 1.1f, -0.08f,
						0.45f,
						fScaleFactor,
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.y );
		}
		fX += 0.45f;
	}

	wpr_drawutils_DrawButtonOverlay( &_PLScreen, 
		WPR_DATATYPES_DRAW_AB_BUTTONS,
		fScaleFactor,
		_pViewportOrtho3D->HalfRes.x,
		_pViewportOrtho3D->HalfRes.y,
		(u32)(_nControllerToLookAt >= 0 ? _nControllerToLookAt : 0) );

	// push the fdraw renderer off 
	frenderer_Pop();
}

static void _DrawControllerConfigMenu( f32 fScaleFactor ) {
	
	// push the mesh renderer
	frenderer_Push( FRENDERER_MESH, NULL );
	fmesh_Ambient_Set( 0.5f, 0.5f, 0.5f, 1.0f );
	wpr_system_ControllerConfig_DrawOrtho( &_CCScreen, 
										&_CCData.paCCConfigs[_nCurSelection],
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
										_apwszPhrases[_PHRASES_CONFIGURATION],
#else
										NULL,
#endif
										_nCurSelection,
										fScaleFactor,
										_pViewportOrtho3D->HalfRes.x,
										_pViewportOrtho3D->HalfRes.y );
	// pop the mesh renderer off
	frenderer_Pop();

	// draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );

	wpr_system_ControllerConfig_DrawFDraw( &_CCData.paCCConfigs[_nCurSelection],
										&_paTexInsts[_TEXTURES_CONTROLLER],
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
										TRUE,
#else
										FALSE,
#endif
										fScaleFactor,
										_pViewportOrtho3D->HalfRes.x,
										_pViewportOrtho3D->HalfRes.y,
										(u32)(_nControllerToLookAt >= 0 ? _nControllerToLookAt : 0) );
	wpr_drawutils_DrawButtonOverlay( &_CCScreen, 
		WPR_DATATYPES_DRAW_ABY_BUTTONS,
		fScaleFactor,
		_pViewportOrtho3D->HalfRes.x,
		_pViewportOrtho3D->HalfRes.y,
		(u32)(_nControllerToLookAt >= 0 ? _nControllerToLookAt : 0) );
	// push the fdraw renderer off 
	frenderer_Pop();
}

static void _DrawMultiPlayerMenu( f32 fScaleFactor ) {
	u32 i;
	Wpr_DataTypes_TextLayout_t *pTextLayout;
	BOOL bSomeoneStillSelecting;

	// push the mesh renderer
	frenderer_Push( FRENDERER_MESH, NULL );
	fmesh_Ambient_Set( 0.5f, 0.5f, 0.5f, 1.0f );
	wpr_system_DrawBasicScreen( &_MJScreen,
		-1,
		FALSE,
		fScaleFactor,
		_pViewportOrtho3D->HalfRes.x,
		_pViewportOrtho3D->HalfRes.y );

	// determine if anyone is still selecting
	bSomeoneStillSelecting = FALSE;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		if( _aMJPlayerInfo[i].nState == _MJ_STATE_SELECT ) {
			bSomeoneStillSelecting = TRUE;
			break;
		}
	}

	// draw the state specific text
	for( i=0; i < MAX_PLAYERS; i++ ) {
		pTextLayout = &_MJScreen.pText[i + 2];

		switch( _aMJPlayerInfo[i].nState ) {

		case _MJ_STATE_ENTER:
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.07f, 
				L"~f8~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszInstructionTextColor,
				pTextLayout->fScale + 0.10f,
				_apwszPhrases[_PHRASES_PRESS_A_TO_JOIN] );
			break;

		case _MJ_STATE_SELECT:
			// print the heading
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.04f, 
				L"~f1~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszBlueTextColor,
				pTextLayout->fScale * 0.98f,
				_apwszPhrases[_PHRASES_CHOOSE_CONTROLS] );			

			// print the control type
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.07f, 
				L"~f1~w0~ac~s%.2f~C%ls{ ~C%ls~B7%ls~B0 ~C%ls}",
				pTextLayout->fScale,
				WprDataTypes_pwszBlueTextColor,
				WprDataTypes_pwszWhiteTextColor,
				_aMJPlayerInfo[i].nControls ? _apwszPhrases[_PHRASES_NON_INVERTED ] : _apwszPhrases[_PHRASES_INVERTED], 
				WprDataTypes_pwszBlueTextColor );
			
			// print the accept instructions
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.13f, 
				L"~f8~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszInstructionTextColor,
				pTextLayout->fScale,
				_apwszPhrases[_PHRASES_PRESS_A_TO_ACCEPT] );
			break;

		case _MJ_STATE_WAIT:
			// print the profile name
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.06f, 
				L"~f1~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszBlueTextColor,
				pTextLayout->fScale + 0.02f,
				_aMJPlayerInfo[i].nControls ? _apwszPhrases[_PHRASES_NON_INVERTED ] : _apwszPhrases[_PHRASES_INVERTED] );

			// print the instructions to move on
			if( !bSomeoneStillSelecting ) {
				ftext_Printf( pTextLayout->fUnitX,
					pTextLayout->fUnitY + 0.13f, 
					L"~f8~C%ls~w0~ac~s%.2f%ls",
					WprDataTypes_pwszInstructionTextColor,
					pTextLayout->fScale,
					_apwszPhrases[_PHRASES_PRESS_A_TO_PROCEED] );
			} else {
				ftext_Printf( pTextLayout->fUnitX,
					pTextLayout->fUnitY + 0.13f, 
					L"~f8~C%ls~w0~ac~s%.2f%ls",
					WprDataTypes_pwszInstructionTextColor,
					pTextLayout->fScale * 0.81f,
					_apwszPhrases[_PHRASES_WAITING_FOR_OTHER_PLAYERS] );				
			}
			break;

		}
	}
	
	// pop the mesh renderer off
	frenderer_Pop();

	// draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );

	wpr_drawutils_DrawButtonOverlay( &_MJScreen, 
		WPR_DATATYPES_DRAW_ABY_BUTTONS,
		fScaleFactor,
		_pViewportOrtho3D->HalfRes.x,
		_pViewportOrtho3D->HalfRes.y,
		(u32)(_nControllerToLookAt >= 0 ? _nControllerToLookAt : 0) );

	// push the fdraw renderer off 
	frenderer_Pop();
}

static void _StartMovie( void ) {
	FASSERT( _nMovieIndex < _NUM_E3_MOVIES );

#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	fmovie2_Play( _apszDemoMovie[_nMovieIndex], 0.35f );
#else
	fmovie2_Play( _apszDemoMovie[_nMovieIndex], 0.5f );
#endif
	if( _pAudioStream ) {
		_pAudioStream->SetVolume( 0.0f );
		_pAudioStream->Pause( TRUE );
	}
	_nLastMode = _nCurrentMode;
	_nCurrentMode = _MODE_ATTRACT_MOVIE;
	_nControllerToLookAt = -1;

	if ( Launcher_bDemoLaunched ) {
		// stop the idle timer during the movie
		gameloop_EnableIdleTimer( FALSE );
	}

	_nMovieIndex++;
	if( _nMovieIndex >= _NUM_E3_MOVIES ) {
		_nMovieIndex = 0;
	}
}
#else
BOOL e3menu_InitSystem( void ) {
	return TRUE;
}
void e3menu_UninitSystem( void ) {

}
#endif
