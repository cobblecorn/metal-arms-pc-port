//////////////////////////////////////////////////////////////////////////////////////
// wpr_levelcomplete.cpp - The End Of Level Screens
//
// Author: Michael Starich
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
// 03/30/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "wpr_levelcomplete.h"
#include "launcher.h"
#include "gameloop.h"
#include "player.h"
#include "game.h"
#include "fresload.h"
#include "floop.h"
#include "wpr_datatypes.h"
#include "gamepad.h"
#include "wpr_drawutils.h"
#include "frenderer.h"
#include "MultiplayerMgr.h"
#include "fperf.h"
#include "wpr_system.h"
#include "msgbox.h"
#include "fcamanim.h"
#include "eparticlepool.h"
#include "gamecam.h"
#include "flightgroup.h"
#include "fdatastreaming.h"

#define _SHOW_SELL_SHEET_INSTEAD	( LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS )	
#define _LEVEL_NAMES_CSV			"pick_level$"
#define _LEVEL_NAMES_TABLE			"Level_Names"
#define _CHARACTERS_PER_SECOND		25.0f
#define _SECS_TO_WAIT_FOR_VALUE		0.45f
#define _TEXT_LEFT_EDGE				0.10f
#define _LINE_1_Y					0.40f
#define _LINE_2_Y					0.45f
#define _LINE_3_Y					0.50f
#define _VALUES_X					0.48f
#define _VALUE_COLOR				L"85900575"
#define _LINE_COLOR					L"70757885"
#define _SECS_TO_FADE_IN			0.55f

typedef enum {
	// the saving states are drawn over a plain black background
	_STATE_SAVING = 0,
	_STATE_WAITING_FOR_SAVE,
	
	// the following states are over the 3d environment
	_STATE_FADE_IN,
	_STATE_TYPING_LINE_1,
	_STATE_WAITING_FOR_VALUE_1,
	_STATE_TYPING_LINE_2,
	_STATE_WAITING_FOR_VALUE_2,
	_STATE_TYPING_LINE3,
	_STATE_WAITING_FOR_VALUE_3,
	_STATE_WAITING_FOR_BUTTON,

	_STATE_COUNT
} _States_e;

#define _STRING_LEN		64

FCLASS_ALIGN_PREFIX class CScreenInfo {
public:
	CFMtx43A CamMtx;
	
	f32 fTimer;
	FViewport_t *pViewportOrtho3D;
	cwchar *pwszLevelName;
	GamepadMap_e nPrevGamePadMap;
	CCamManualInfo CamInfo;
	CFCamAnimInst *pCamAnimInst;
	CFAudioStream *pMusicStream;
	
#if _SHOW_SELL_SHEET_INSTEAD
	CFTexInst *pBackgroundTexInst;	
#endif
	wchar wszLocation[_STRING_LEN];
	wchar wszTagName[_STRING_LEN];
	BOOL8 bPrevGovernorState;
	u8 nState;
	u8 nLettersToPrint;	
	
	FCLASS_STACKMEM_ALIGN( CScreenInfo );
} FCLASS_ALIGN_SUFFIX;

/////////////////////
// private variables

// stats cached from the player before dumping the level (can't be dynamically allocated)
static u16 _nEnemiesDestroyed;
static u16 _nWashersCollected;
static u8 _nHoursToComplete;
static u8 _nMinsToComplete;
static u8 _nSecsToComplete;
static u8 _nControllerIndex;
static CPlayerProfile *_pPlayerProfile;
static FResFrame_t _ResFrame;
static FMemFrame_t _MemFrame;

static CScreenInfo *_pScreenInfo = NULL;// if non NULL then we are inside this system

//////////////////////
// private prototypes

#if _SHOW_SELL_SHEET_INSTEAD
	static BOOL _SellSheet_Init( void );
	static BOOL _SellSheet_UnloadAndExit( BOOL bLoadNextLevel );
	static BOOL _SellSheet_Work( void );
	static BOOL _SellSheet_Draw( void );
#endif

	static BOOL _SP_Init( void );
static void _UnloadAndExit( BOOL bLoadNextLevel );
static u32 _HowManyLettersShouldAppear( u32 nStringLen, f32 &rfTimer, f32 fCharsPerSec );
static BOOL _SP_Work( void );
static BOOL _SP_Draw( void );
static void _PrintLine( f32 fX, f32 fY, s32 nCharsToPrint, cwchar *pwszLine );
static void _PrintRegValue( f32 fX, f32 fY, u32 nValue );
static void _PrintPressButton( f32 fX, f32 fY, s32 nCharsToPrint );

////////////////////
// public functions

BOOL wpr_levelcomplete_InitSystem( void ) {
	_pScreenInfo = NULL;
	return TRUE;
}

void wpr_levelcomplete_UninitSystem( void ) {
#if _SHOW_SELL_SHEET_INSTEAD
	_SellSheet_UnloadAndExit( FALSE );
#else
	_UnloadAndExit( FALSE );
#endif
}

void wpr_levelcomplete_ScheduleSinglePlayerScreen( void ) {
	FASSERT( !_pScreenInfo );
	FASSERT( MultiplayerMgr.IsSinglePlayer() );

#if _SHOW_SELL_SHEET_INSTEAD
	gameloop_SetLoopHandlers( _SellSheet_Work, _SellSheet_Draw, _SellSheet_Init );
#else
	gameloop_SetLoopHandlers( _SP_Work, _SP_Draw, _SP_Init );
#endif
}

////////////////////
// private functions

static BOOL _SP_Init( void ) {
	Level_e nLevel;
	u32 nTemp1, nTemp2, nTemp3;
	
	FASSERT( !_pScreenInfo );

	gameloop_ShowFPS( FALSE );
	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	// copy anything that we need from the player array before we dump the game
	Player_aPlayer[0].GetSPStats( nTemp1, nTemp2 );
	_nEnemiesDestroyed = nTemp1;
	_nWashersCollected = nTemp2;
	wpr_datatypes_BreakDownTimeUnits( Level_fSecsInLevel, nTemp1, nTemp2, nTemp3 );
	_nHoursToComplete = nTemp1;
	_nMinsToComplete = nTemp2;
	_nSecsToComplete = nTemp3;
	nLevel = Level_aInfo[Level_nLoadedIndex].nLevel;
	_nControllerIndex = Player_aPlayer[0].m_nControllerIndex;
	_pPlayerProfile = Player_aPlayer[0].m_pPlayerProfile;

	// dump the loaded game
	game_UnloadLevel();
	
	DEVPRINTF( "******** LOAD MARKER - START LOADING LEVEL COMPLETE SCREEN***********.\n" );
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( "Level Complete Screen" );
#endif
	
	// grab memory frames...
	_ResFrame = fres_GetFrame();
	_MemFrame = fmem_GetFrame();
	
	// allocate our screen info structure
	_pScreenInfo = fnew CScreenInfo;
	if( !_pScreenInfo ) {
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );	
		launcher_LoadFailure();
		return TRUE;
	}
	
	// reset some of our state vars
	_pScreenInfo->fTimer = 0.0f;
	_pScreenInfo->nState = _STATE_SAVING;
	_pScreenInfo->nLettersToPrint = 0;
	_pScreenInfo->pMusicStream = NULL;
	
	// setup the controller mapping
	_pScreenInfo->nPrevGamePadMap = gamepad_SetMapping( _nControllerIndex, GAMEPAD_MAP_MENU );

	// load the assets
	CGColl::LoadMaterialTable( "mset_std" );
	FDS_StreamMgr.Init( (u32)FDS_DEFAULT_LOCAL_CACHE_SIZE );
	fsh_Fog_Enable( FALSE );
	
	// load the world and camera file
	if( !fresload_Load( FWORLD_RESTYPE, "we_1victory" ) ) {
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	eparticlepool_InitLevel();
	
	_pScreenInfo->pCamAnimInst = CFCamAnimInst::Load( "CE_1victory" );
	if( !_pScreenInfo->pCamAnimInst ) {
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	CFQuatA Quat;
	CFVec3 Pos;
	_pScreenInfo->pCamAnimInst->UpdateUnitTime( 0.0f );
	_pScreenInfo->pCamAnimInst->ComputeFrame( _pScreenInfo->CamInfo.m_fHalfFOV, Quat, Pos );
	Quat.BuildMtx( _pScreenInfo->CamMtx );
	_pScreenInfo->CamMtx.m_vPos.Set( Pos );

	_pScreenInfo->CamInfo.m_pmtxMtx = &_pScreenInfo->CamMtx;

	// init the gamecam system
	gamecam_InitLevelCameras();
	gamecam_SwitchPlayerToManualCamera( GAME_CAM_PLAYER_1, &_pScreenInfo->CamInfo );
	gamecam_SetActiveCamera( GAME_CAM_PLAYER_1 );
	
	CEntity::ResolveEntityPointerFixups();

	// load the level name csv file
	if( nLevel < 0 || nLevel > LEVEL_SINGLE_PLAYER_COUNT ) {
		_pScreenInfo->pwszLevelName = NULL;
	} else {
		if( !wpr_system_CreateLoadHeading( nLevel, 
										NULL, 0, 
										_pScreenInfo->wszLocation, _STRING_LEN,
										_pScreenInfo->wszTagName, _STRING_LEN ) ) {
#if !GAMELOOP_EXTERNAL_DEMO
			ffile_LogStop();
#endif
			DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );											
			_UnloadAndExit( FALSE );
			launcher_LoadFailure();
			return TRUE;									
		}
		_pScreenInfo->pwszLevelName = _pScreenInfo->wszTagName;			
	}
		
	// load the font that we are going to use for the print outs
	if( ftext_Load( 'Z', "TFHarials$" ) == FTEXT_ERROR ) {
		DEVPRINTF( "wpr_levelcomplete::_SP_Init() : Could not load the font: 'TFHarials$'\n" );
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );			
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	
	_pScreenInfo->pViewportOrtho3D = fviewport_Create();
	if( !_pScreenInfo->pViewportOrtho3D ) {
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	fviewport_InitOrtho3D( _pScreenInfo->pViewportOrtho3D, 0.1f, 100.0f );

	_pScreenInfo->bPrevGovernorState = floop_IsGovernorEnabled();
	floop_EnableGovernor( TRUE );

	wpr_system_InitInGame();

	// The retail victory world has no music entry, and unloading the level stopped its stream.
	// Reuse the available game theme so the single-player results screen is not silent. The stream
	// loads on the audio worker; _SP_Work starts it once it is ready, as the front end's menus do.
	_pScreenInfo->pMusicStream = CFAudioStream::Create( "MA_Theme" );
	if( !_pScreenInfo->pMusicStream ) {
		DEVPRINTF( "wpr_levelcomplete::_SP_Init() : Could not create the results-screen music stream.\n" );
	}
	
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		
	
	return TRUE;
}

static void _UnloadAndExit( BOOL bLoadNextLevel ) {

	if( !_pScreenInfo ) {
		return;
	}
	if( _pScreenInfo->pMusicStream ) {
		_pScreenInfo->pMusicStream->Stop( FALSE );
		_pScreenInfo->pMusicStream->Destroy();
		_pScreenInfo->pMusicStream = NULL;
	}
	wpr_system_End();

	_pScreenInfo->pCamAnimInst = NULL;
	gamecam_PostLevelCleanup();
	FDS_StreamMgr.Uninit();
	eparticlepool_UninitLevel();
	CGColl::ClearMaterialTable();

	floop_EnableGovernor( _pScreenInfo->bPrevGovernorState );
	
	// set the controller mapping back to its original state
	gamepad_SetMapping( _nControllerIndex, _pScreenInfo->nPrevGamePadMap );
	
	// delete our screen info object, this must be done right before releasing the frame
	fdelete( _pScreenInfo );
	_pScreenInfo = NULL;

	// release the mem frames
	fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( _MemFrame );

	fparticle_KillAllEmitters();

	gameloop_ShowFPS( TRUE );

	// finally move to the next level
	if( bLoadNextLevel ) {
		launcher_EndOfGameDecisions( LAUNCHER_DECISION_LOAD_NEXT );
	}
}

static u32 _HowManyLettersShouldAppear( u32 nStringLen, f32 &rfTimer, f32 fCharsPerSec ) {
	u32 nChars;

	_pScreenInfo->fTimer += FLoop_fPreviousLoopSecs;

	nChars = (u32)( _pScreenInfo->fTimer * fCharsPerSec );
	FMATH_CLAMP( nChars, 0, nStringLen );
	
	return nChars;
}

static BOOL _SP_Work( void ) {
	u32 nStringLen;

	// Start (and loop) the results music once its stream has loaded, at the menus' music level.
	if( _pScreenInfo->pMusicStream && _pScreenInfo->pMusicStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
		_pScreenInfo->pMusicStream->SetVolume( 0.25f );
		_pScreenInfo->pMusicStream->Play( 0 );
	}

	if( _pScreenInfo->nState >= _STATE_FADE_IN ) {
		gamepad_Sample();
				
		CEntity::CallAllAutoWorks();

		// do our camera work
		_pScreenInfo->pCamAnimInst->DeltaTime( FLoop_fPreviousLoopSecs * 0.1f, FALSE );

		CFQuatA Quat;
		CFVec3 Pos;
		_pScreenInfo->pCamAnimInst->ComputeFrame( _pScreenInfo->CamInfo.m_fHalfFOV, Quat, Pos );
		Quat.BuildMtx( _pScreenInfo->CamMtx );
		_pScreenInfo->CamMtx.m_vPos.Set( Pos );

		fsh_RegisterPlayerPos( _pScreenInfo->CamMtx.m_vPos );

		gamecam_Work();	

		CEntity::CallAllMarkedRemoveFromWorld();
		fparticle_Work();
		CFLightGroupMgr::Work();
	}
	
	switch( _pScreenInfo->nState ) {
	
	case _STATE_SAVING:
#if FANG_WINGC
		if( coopsave_IsSessionActive() ) {
			// campaign co-op: every player's progress goes to the co-op save
			coopsave_SaveSession();
		} else
#endif
		if( _pPlayerProfile && !_pPlayerProfile->IsVirtual() ) {
			wpr_system_IG_SaveGame( _pPlayerProfile );
		}
		_pScreenInfo->nState++;    
		break;

	case _STATE_WAITING_FOR_SAVE:
		// should automatically advance...
		_pScreenInfo->nState = _STATE_FADE_IN;
		_pScreenInfo->fTimer = 0.0f;
		break;
		
	case _STATE_FADE_IN:
		_pScreenInfo->fTimer += FLoop_fPreviousLoopSecs;
		if( _pScreenInfo->fTimer >= _SECS_TO_FADE_IN ) {
			_pScreenInfo->nState++;
			_pScreenInfo->fTimer = 0.0f;
			_pScreenInfo->nLettersToPrint = 0;
		}
		break;	
	
	case _STATE_TYPING_LINE_1:
		nStringLen = fclib_wcslen( Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );

		_pScreenInfo->nLettersToPrint = _HowManyLettersShouldAppear( nStringLen, _pScreenInfo->fTimer, _CHARACTERS_PER_SECOND );
		if( _pScreenInfo->nLettersToPrint >= nStringLen ) {
			_pScreenInfo->nState++;
			_pScreenInfo->fTimer = 0.0f;
			_pScreenInfo->nLettersToPrint = 0;
		}
		break;
		
	case _STATE_WAITING_FOR_VALUE_1:
	case _STATE_WAITING_FOR_VALUE_2:
	case _STATE_WAITING_FOR_VALUE_3:
		// wait for a spell before the value comes on
		_pScreenInfo->fTimer += FLoop_fPreviousLoopSecs;
		if( _pScreenInfo->fTimer >= _SECS_TO_WAIT_FOR_VALUE ) {
			_pScreenInfo->fTimer = 0.0f;
			// move to the next state
			_pScreenInfo->nState++;
		}
		break;

	case _STATE_TYPING_LINE_2:
		nStringLen = fclib_wcslen( Game_apwszPhrases[GAMEPHRASE_WASHERS_COLLECTED] );

		_pScreenInfo->nLettersToPrint = _HowManyLettersShouldAppear( nStringLen, _pScreenInfo->fTimer, _CHARACTERS_PER_SECOND );
		if( _pScreenInfo->nLettersToPrint >= nStringLen ) {
			_pScreenInfo->nState++;
			_pScreenInfo->fTimer = 0.0f;
			_pScreenInfo->nLettersToPrint = 0;
		}
		break;

	case _STATE_TYPING_LINE3:
		nStringLen = fclib_wcslen( Game_apwszPhrases[GAMEPHRASE_TIME_TO_COMPETE] );

		_pScreenInfo->nLettersToPrint = _HowManyLettersShouldAppear( nStringLen, _pScreenInfo->fTimer, _CHARACTERS_PER_SECOND );
		if( _pScreenInfo->nLettersToPrint >= nStringLen ) {
			_pScreenInfo->nState++;
			_pScreenInfo->fTimer = 0.0f;
			_pScreenInfo->nLettersToPrint = 0;
		}
		break;

	case _STATE_WAITING_FOR_BUTTON:
		nStringLen = fclib_wcslen( game_GetPromptPhrase( GAMEPHRASE_PRESS_A_TO_CONTINUE, _nControllerIndex ) );

		_pScreenInfo->nLettersToPrint = _HowManyLettersShouldAppear( nStringLen, _pScreenInfo->fTimer, _CHARACTERS_PER_SECOND );
		if( _pScreenInfo->nLettersToPrint >= nStringLen && _pScreenInfo->fTimer > 4.0f ) {
			_pScreenInfo->fTimer = 0.0f;
			_pScreenInfo->nLettersToPrint = 0;
		}
		
		if( Gamepad_aapSample[_nControllerIndex][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			// done
			_UnloadAndExit( TRUE );
			return TRUE;
		}
		break;
	}

	return TRUE;
}

static BOOL _SP_Draw( void ) {
	FViewport_t *pPrevViewport;
	
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	if( _pScreenInfo->nState >= _STATE_FADE_IN ) {
		fvis_FrameBegin();
		ftex_HandleRenderTargets();
		// setup the 3d viewport and init the camera stack
		gamecam_SetViewportAndInitCamStack();
		fvis_Draw( NULL, FALSE, 0 );
		fvis_SetupShadows();
		fvis_FrameEnd();
		
		pPrevViewport = fviewport_SetActive( _pScreenInfo->pViewportOrtho3D );
		FXfm_Identity.InitStackWithView();

		// print the level name
		if( _pScreenInfo->pwszLevelName ) {
			ftext_Printf( _TEXT_LEFT_EDGE, 0.075f, "~f3~w1~aL~C30207599~s1.35%ls", _pScreenInfo->pwszLevelName ); 
		} else {
			ftext_Printf( _TEXT_LEFT_EDGE, 0.075f, "~f3~w1~aL~C30207599~s1.35Development Level" ); 
		}

		// print the screen heading
		ftext_Printf( _TEXT_LEFT_EDGE, 0.16f,
						L"~f3~C90502099~w1~aL~s1.05%ls",
						Game_apwszPhrases[GAMEPHRASE_LEVEL_COMPLETE] );
	}

	switch( _pScreenInfo->nState ) {
	
	case _STATE_SAVING:
	case _STATE_WAITING_FOR_SAVE:
		// nothing to draw, cause we aren't calling the draw function
		break;
		
	case _STATE_FADE_IN:
		ftext_ClearAllPending();
		game_DrawSolidFullScreenOverlay( _pScreenInfo->fTimer * (1.0f/_SECS_TO_FADE_IN), 0.0f );
		break;

	case _STATE_TYPING_LINE_1:
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, _pScreenInfo->nLettersToPrint, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		break;

	case _STATE_WAITING_FOR_VALUE_1:
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, -1, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		break;

	case _STATE_TYPING_LINE_2:
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, -1, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		_PrintRegValue( _VALUES_X, _LINE_1_Y, _nEnemiesDestroyed );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_2_Y, _pScreenInfo->nLettersToPrint, Game_apwszPhrases[GAMEPHRASE_WASHERS_COLLECTED] );
		break;
		
	case _STATE_WAITING_FOR_VALUE_2:
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, -1, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		_PrintRegValue( _VALUES_X, _LINE_1_Y, _nEnemiesDestroyed );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_2_Y, -1, Game_apwszPhrases[GAMEPHRASE_WASHERS_COLLECTED] );
		break;
		
	case _STATE_TYPING_LINE3:
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, -1, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		_PrintRegValue( _VALUES_X, _LINE_1_Y, _nEnemiesDestroyed );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_2_Y, -1, Game_apwszPhrases[GAMEPHRASE_WASHERS_COLLECTED] );
		_PrintRegValue( _VALUES_X, _LINE_2_Y, _nWashersCollected );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_3_Y, _pScreenInfo->nLettersToPrint, Game_apwszPhrases[GAMEPHRASE_TIME_TO_COMPETE] );
		break;

	case _STATE_WAITING_FOR_VALUE_3:
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, -1, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		_PrintRegValue( _VALUES_X, _LINE_1_Y, _nEnemiesDestroyed );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_2_Y, -1, Game_apwszPhrases[GAMEPHRASE_WASHERS_COLLECTED] );
		_PrintRegValue( _VALUES_X, _LINE_2_Y, _nWashersCollected );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_3_Y, -1, Game_apwszPhrases[GAMEPHRASE_TIME_TO_COMPETE] );
		break;

	case _STATE_WAITING_FOR_BUTTON:
		// print everything
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_1_Y, -1, Game_apwszPhrases[GAMEPHRASE_ENEMIES_DESTROYED] );
		_PrintRegValue( _VALUES_X, _LINE_1_Y, _nEnemiesDestroyed );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_2_Y, -1, Game_apwszPhrases[GAMEPHRASE_WASHERS_COLLECTED] );
		_PrintRegValue( _VALUES_X, _LINE_2_Y, _nWashersCollected );
		_PrintLine( _TEXT_LEFT_EDGE, _LINE_3_Y, -1, Game_apwszPhrases[GAMEPHRASE_TIME_TO_COMPETE] );
		ftext_Printf( _VALUES_X, _LINE_3_Y, 
				L"~fZ~C%ls~w0~aL~s0.95%02d:%02d:%02d",
				_VALUE_COLOR,
				_nHoursToComplete, _nMinsToComplete, _nSecsToComplete );

		// print "Press A To Continue"
		_PrintPressButton( 0.3f, 0.65f, _pScreenInfo->nLettersToPrint );
		break;
	}	

	CMsgBox::DrawAll();

	if( _pScreenInfo->nState >= _STATE_FADE_IN ) {
		// restore the viewport
		fviewport_SetActive( pPrevViewport );
	}

	return TRUE;
}

static void _PrintLine( f32 fX, f32 fY, s32 nCharsToPrint, cwchar *pwszLine ) {
	if( nCharsToPrint > 0 ) {
		wchar wszTemp[128];

		FMATH_CLAMPMAX( nCharsToPrint, 128 );
		fclib_wcsncpy( wszTemp, pwszLine, nCharsToPrint );
		wszTemp[nCharsToPrint] = 0;
	
		// print only a portion of the text
		ftext_Printf( fX, fY,
			L"~fZ~C%ls~w0~aL~s0.95%ls",
			_LINE_COLOR,
			wszTemp );
	} else if( nCharsToPrint < 0 ) {
		// print all of the text
		ftext_Printf( fX, fY,
			L"~fZ~C%ls~w0~aL~s0.95%ls",
			_LINE_COLOR,
			pwszLine );
	}
}

static void _PrintRegValue( f32 fX, f32 fY, u32 nValue ) {
	ftext_Printf( fX, fY,
		L"~fZ~C%ls~w0~aL~s0.95%d",
		_VALUE_COLOR,
		nValue );
}

static void _PrintPressButton( f32 fX, f32 fY, s32 nCharsToPrint ) {

	if( nCharsToPrint > 0 ) {
		wchar wszTemp[128];

		FMATH_CLAMPMAX( nCharsToPrint, 128 );
		fclib_wcsncpy( wszTemp, game_GetPromptPhrase( GAMEPHRASE_PRESS_A_TO_CONTINUE, _nControllerIndex ), nCharsToPrint );
		wszTemp[nCharsToPrint] = 0;
	
		// print only a portion of the text
		ftext_Printf( fX, fY,
			L"~fZ~C%ls~w0~al~s1.20%ls",
			WprDataTypes_pwszPressStartColor,
			wszTemp );
	} else if( nCharsToPrint < 0 ) {
		// print all of the text
		ftext_Printf( fX, fY,
			L"~fZ~C%ls~w0~al~s1.20%ls",
			WprDataTypes_pwszPressStartColor,
			game_GetPromptPhrase( GAMEPHRASE_PRESS_A_TO_CONTINUE, _nControllerIndex ) );
	}
}


//////////////////////////////////////////////////////////////
// sell sheet functions
#if _SHOW_SELL_SHEET_INSTEAD

static BOOL _SellSheet_Init( void ) {
	Level_e nLevel;
	FGameDataFileHandle_t hFile;
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTable;
	u32 nTemp1, nTemp2, nTemp3;

	gameloop_ShowFPS( FALSE );
	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	// copy anything that we need from the player array before we dump the game
	_nControllerIndex = Player_aPlayer[0].m_nControllerIndex;
	
	// dump the loaded game
	game_UnloadLevel();
	
	DEVPRINTF( "******** LOAD MARKER - START LOADING LEVEL COMPLETE SCREEN***********.\n" );
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( "Level Complete Screen" );
#endif	
	
	// grab memory frames...
	_ResFrame = fres_GetFrame();
	_MemFrame = fmem_GetFrame();
	
	// allocate our screen info structure
	_pScreenInfo = fnew CScreenInfo;
	if( !_pScreenInfo ) {
		launcher_LoadFailure();
		return TRUE;
	}
	
	// reset some of our state vars
	_pScreenInfo->fTimer = 0.0f;
	_pScreenInfo->nLettersToPrint = 0;
	
	// setup the controller mapping
	_pScreenInfo->nPrevGamePadMap = gamepad_SetMapping( _nControllerIndex, GAMEPAD_MAP_MENU );

	// load the assets
	_pScreenInfo->pBackgroundTexInst = fnew CFTexInst;
	if( !_pScreenInfo->pBackgroundTexInst ) {
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		
		_SellSheet_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	_pScreenInfo->pBackgroundTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, "TFH2sell_01" ) );
	
	// load the font that we are going to use for the print outs
	if( ftext_Load( 'Z', "TFHarials$" ) == FTEXT_ERROR ) {
		DEVPRINTF( "wpr_levelcomplete::_SellSheet_Init() : Could not load the font: 'TFHarials$'\n" );
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );			
		_SellSheet_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	
	_pScreenInfo->pViewportOrtho3D = fviewport_Create();
	if( !_pScreenInfo->pViewportOrtho3D ) {
#if !GAMELOOP_EXTERNAL_DEMO
		ffile_LogStop();
#endif
		DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		
		_SellSheet_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	fviewport_InitOrtho3D( _pScreenInfo->pViewportOrtho3D, 0.1f, 100.0f );

	_pScreenInfo->bPrevGovernorState = floop_IsGovernorEnabled();
	floop_EnableGovernor( TRUE );
	
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END OF LEVEL COMPLETE SCREEN***********.\n" );		

	return TRUE;
}

static BOOL _SellSheet_UnloadAndExit( BOOL bLoadNextLevel ) {

	if( !_pScreenInfo ) {
		return FALSE;
	}
	if( _pScreenInfo->pBackgroundTexInst ) {
		fdelete( _pScreenInfo->pBackgroundTexInst );
		_pScreenInfo->pBackgroundTexInst = NULL;		
	}
	
	floop_EnableGovernor( _pScreenInfo->bPrevGovernorState );
	
	// set the controller mapping back to its original state
	gamepad_SetMapping( _nControllerIndex, _pScreenInfo->nPrevGamePadMap );

	// delete our screen info object, this must be done right before releasing the frame
	fdelete( _pScreenInfo );
	_pScreenInfo = NULL;
	
	// release the mem frames
	fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( _MemFrame );

	gameloop_ShowFPS( TRUE );

	// finally move to the next level
	if( bLoadNextLevel ) {
		launcher_EndOfGameDecisions( LAUNCHER_DECISION_LOAD_NEXT );
	}

	return TRUE;
}

static BOOL _SellSheet_Work( void ) {

	gamepad_Sample();

	u32 nStringLen = fclib_wcslen( game_GetPromptPhrase( GAMEPHRASE_PRESS_A_TO_CONTINUE, _nControllerIndex ) );

	_pScreenInfo->nLettersToPrint = _HowManyLettersShouldAppear( nStringLen, _pScreenInfo->fTimer, _CHARACTERS_PER_SECOND );
	if( _pScreenInfo->nLettersToPrint >= nStringLen && _pScreenInfo->fTimer > 4.0f ) {
		_pScreenInfo->fTimer = 0.0f;
		_pScreenInfo->nLettersToPrint = 0;
	}

	if( Gamepad_aapSample[_nControllerIndex][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		// done
		_SellSheet_UnloadAndExit( TRUE );
		return TRUE;
	}

	return TRUE;
}

static BOOL _SellSheet_Draw( void ) {

	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
	FViewport_t *pPrevViewport = fviewport_SetActive( _pScreenInfo->pViewportOrtho3D );
	FXfm_Identity.InitStackWithView();

	// draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );
	// draw the background texture map
	wpr_drawutils_DrawTextureToScreen( TRUE,
		_pScreenInfo->pBackgroundTexInst,
		0.0f, -0.1f,
		2.05f,
		_pScreenInfo->pViewportOrtho3D->Res.x * (1.0f/640.0f),
		_pScreenInfo->pViewportOrtho3D->HalfRes.x,
		_pScreenInfo->pViewportOrtho3D->HalfRes.y );
	// push the fdraw renderer off 
	frenderer_Pop();

	_PrintPressButton( 0.3f, 0.65f, _pScreenInfo->nLettersToPrint );

	// restore the viewport
	fviewport_SetActive( pPrevViewport );

	return TRUE;
}

#endif


	
