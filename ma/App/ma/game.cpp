//////////////////////////////////////////////////////////////////////////////////////
// game.cpp - Main gameplay mode.
//
// Author: Steve Ranck     
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
// 02/18/02 Ranck       Created.
// 06/14/02	Link		Added LaserBeam system.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "game.h"

#include "fviewport.h"
#include "fworld.h"
#include "fdebris.h"
#include "fverlet.h"
#include "fclib.h"
#include "fsh.h"
#include "frenderer.h"
#include "FScriptSystem.h"
#include "FScalarObj.h"
#include "floop.h"
#include "fstringtable.h"
#include "fxfm.h"
#include "ffile.h"
#include "fvid.h"
#include "fvis.h"
#include "ftex.h"
#include "ftextmon.h"
#include "fshadow.h"
#include "fmovie2.h"
#if FANG_PLATFORM_WIN
	#include "dx/fdx8vid.h"
#endif

#include "eboomer.h"
#include "botfx.h"
#include "level.h"
#include "gamepad.h"
#include "reticle.h"
#include "fphysics.h"
#include "Hud2.h"
#include "PauseScreen.h"
#include "Workable.h"
#include "gamecam.h"
#include "tracer.h"
#include "potmark.h"
#include "gamecam.h"
#include "player.h"
#include "explosion.h"
#include "botswarmer.h"
#include "gamesave.h"
#include "gstring.h"
#include "damage.h"
#include "vehiclerat.h"
#include "loadingscreen.h"
#include "AlarmSys.h"
#include "AI/AIMain.h"
#include "AI/AIBrainman.h"	// for special player brain	functions
#include "letterbox.h"
#include "sas_user.h"
#include "gameloop.h"
#include "sas_user.h"
#include "tripwire.h"
#include "user_albert.h"
#include "user_john.h"
#include "user_mike.h"
#include "user_steve.h"
#include "user_elliott.h"
#include "MAScriptTypes.h"
#include "Door.h"
#include "ESwitch.h"
#include "Protrack.h"
#include "TalkSystem2.h"
#include "BarterSystem.h"
#include "smoketrail.h"
#include "spawnsys.h"
#include "LaserBeam.h"
#include "MuzzleFlash.h"
//#include "BlinkShell.h"
#include "BlinkSpeed.h"
#include "BlinkGlow.h"
#include "flamer.h"
#include "tether.h"
#include "GameError.h"
#include "eproj_cleaner.h"
#include "zipline.h"
#include "fxshockwave.h"
#include "splat.h"
#include "testbots.h"
#include "fxempblast.h"
#include "botslosh.h"
#include "boteliteguard.h"
#include "sloshtank.h"
#include "fxmagmabomb.h"
#include "botkrunk.h"
#include "vehicleloader.h"
#include "eparticlepool.h"
#include "launcher.h"
#include "FXSlower.h"
#include "Ebotfire.h"
#include "collectable.h"
#include "botmozer.h"
#include "botglitch.h"
#include "botscout.h"
#include "botgrunt.h"
#include "LightPool.h"
#include "SplitScreen.h"
#include "MultiplayerMgr.h"
#include "wpr_levelcomplete.h"
#include "flightgroup.h"
#include "wpr_system.h"
#include "mg_holdyourground.h"
#include "msgbox.h"
#include "weapon_recruiter.h"
#include "boteliteguard.h"
#include "botscientist.h"
#include "fRenderSort.h"
#include "difficulty.h"
#include "AI/AiApi.h"

#define _USE_FDRAW_IN_SCENE_RENDER 1

/////////////////////////////////////////
// private definitions:

#if FANG_PLATFORM_DX
	#define _DEBRIS_COUNT			200
	#define _DEBRIS_SPAWNER_COUNT	40
#else
	#define _DEBRIS_COUNT			100
	#define _DEBRIS_SPAWNER_COUNT	20
#endif


#define _DEBRIS_GROUP_FILENAME		"deb_group"
#define _DEBRIS_MESH_SET_FILENAME	"deb_meshset"
#define _EXPLOSION_SET_FILENAME		"explsn_set"
#define _EXPLOSION_GROUPS_FILENAME	"explsn_grps"

static cchar *_pszGamePhrasesTableName = "common_phrases";
#if FANG_WINGC
	static cchar *_pszPlatformPhrasesTableName = "gc_phrases";
#elif FANG_PLATFORM_DX
	static cchar *_pszPlatformPhrasesTableName = "xbox_phrases";
#elif FANG_PLATFORM_GC
	static cchar *_pszPlatformPhrasesTableName = "gc_phrases";
#elif FANG_PLATFORM_PS2
	static cchar *_pszPlatformPhrasesTableName = "ps2_phrases";	
#endif
static cchar *_pszGamePhrasesCSVFilename = "gamephrase$";
static cchar *_pszGlobalSettingsTableName = "global_settings";
static cchar *_pszGlobalSettingsCSVFilename = "ma";

typedef enum {
	_CONTROLLER_PAUSE_STATE_OK = 0,
	_CONTROLLER_PAUSE_STATE_WAIT_FOR_INSERT,
	_CONTROLLER_PAUSE_STATE_WAIT_FOR_PRESS,
	_CONTROLLER_PAUSE_STATE_WAIT_FOR_RELEASE,

	_CONTROLLER_PAUSE_STATE_COUNT
} _ControllerPauseState_e;

typedef enum {
	_FADE_STATE_NONE = 0,
	_FADE_STATE_IN,
	_FADE_STATE_OUT,

	_FADE_STATE_COUNT
} _FadeState_e;

#define _ALL_CONTROLLERS_PLUGGED_IN	-1

#if FANG_PLATFORM_PS2
	#define _UPDATE_LOADSCREEN
#else
	#define _UPDATE_LOADSCREEN			loadingscreen_Update();// turned into a macro to break up the look of the code a bit in an effort to make it easier to read - Starich
#endif


enum {
	_GLOBALSETTINGS_POSTPOSSESSION_BOTPOWERDOWNTIME = 0,

	_GLOBALSETTINGS_COUNT
};


/////////////////////////////////////////
// public vars:
BOOL Game_bCameraPOVVisibility = FALSE;
BOOL Game_bEnterPauseModeImmediatelyForCutsceneCreation = FALSE;
CFTexInst *Game_pFullscreenRenderTarget;
GameFullscreenRenderTargetReadyCallback_t *Game_pFcnFullscreenRenderTargetReadyCallback;
cwchar *Game_apwszPhrases[ GAMEPHRASE_COUNT ];

/////////////////////////////////////////
// private vars:
static BOOL8 _bSystemInitialized = FALSE;
static BOOL8 _bGameUnloadedNeeded = FALSE;// prevents game_unload from being called twice
static BOOL8 _bAllowCutSceneSkip;
static BOOL8 _bCompletedLevel;
static ControlMode_e _aeControlMode[MAX_PLAYERS];
static CFStringTable *_pCutSceneTable = NULL;
static cchar *_pszCurCutSceneName = NULL;
static const GameInitInfo_t *_pCurrentGameInit=NULL;
static FResFrame_t _hResFrame;
static ListenerOrientationCallback_t *_pListenerCallback = NULL;
static s8 _nControllerToPlugIn;
static u8 _nControllerMaskRequired;
static u8 _nControllerPauseState;
static u8 _nFadeState;// see _FadeState_e...
static f32 _fFade_Timer;

// Async Read Error variables
static FFileAsyncReadErrorCode_e _eAsyncHaltError = FFILE_ASYNC_READ_ERROR_NONE;
static BOOL _bIssueBeginOnAsyncErrorResume;
static FAudio_PauseLevel_e _PreAsyncErrorHaltStreamPauseLevel;
static FAudio_PauseLevel_e _PreAsyncErrorHaltEmitterPauseLevel;
static cwchar *_pwszAsyncErrorHaltDisplayString;

////////////////////////////////////////
// private prototypes:
static void _SetupCameraAndClearViewport( void );
static void _DrawMainScene_Persp( void );
static void _DrawDebugStuff_Persp( void );
static void _DrawPlayerText_Ortho( s32 nPlayer );
static void _DrawDebugStuffForPlayer_Persp( s32 nPlayer );
static void _DrawHUD_Persp( int nPlayer );
static void _DrawReticles( void );
static void _DrawHUD_OrthoForPlayer( int nPlayer );
static void _DrawHUD_OrthoForAll( void );
static void _FullscreenRenderTargetCallback( void *pUserData );
static BOOL _CreatePlayerBot( int nPlayer );
static void _DisablePlayerControls(s32 nPlayer);
static void _EnablePlayerControls(s32 nPlayer);
static BOOL _PostWorldLoadGameInit( const GameInitInfo_t *pGameInit );
static BOOL _LoadGlobalSettings( void );
static BOOL _LoadPhraseTable( void );
static void _RenderSortCallbackPrePointSprites( CFMtx43A *pRenderCamMtx, FViewport_t *pRenderViewport );
static void _FFileAsyncReadErrorHaltCallback( FFileAsyncReadErrorCode_e eErrorCode, BOOL bHandleSwap );
static void _FFileAsyncReadErrorResumeCallback( FFileAsyncReadErrorCode_e eErrorCode, BOOL bHandleSwap );

#if FANG_ENABLE_DEV_FEATURES
	static f32 _fLoadTestTimer;	
#endif

//////////////////////////////////////
// public functions:

// Initializes the game system. Call this only one time at the startup of the game.
BOOL game_InitSystem( void ) {
	FASSERT_MSG( !_bSystemInitialized, "game_InitSystem(): Cannot call this function more than once.\n" );

	_bSystemInitialized = FALSE;
	
#if FANG_PLATFORM_GC
	ffile_SetAsyncReadErrorHaltCallback( _FFileAsyncReadErrorHaltCallback );
	ffile_SetAsyncReadErrorResumeCallback( _FFileAsyncReadErrorResumeCallback );
#endif	

	// zero out memory
	Game_bEnterPauseModeImmediatelyForCutsceneCreation = FALSE;
	fang_MemZero( &Game_apwszPhrases, sizeof( Game_apwszPhrases ) );

	if( !_LoadGlobalSettings() ) {
		return FALSE;
	}

	Game_bCameraPOVVisibility = FALSE;

	_pCutSceneTable = CFStringTable::CreateTable("cutscene names");
	if( _pCutSceneTable == NULL ) {
		DEVPRINTF( "game_InitSystem(): Could not create string table for cutscene names.\n" );
		return FALSE;
	}

	MultiplayerMgr.InitSystem();

	for (int nP = 0; nP < MAX_PLAYERS; nP++) {
		_aeControlMode[nP] = CONTROLMODE_NORMAL;
	}

	Game_pFullscreenRenderTarget = NULL;
	Game_pFcnFullscreenRenderTargetReadyCallback = NULL;
	_pCurrentGameInit = NULL;

	_bSystemInitialized = TRUE;
	_bGameUnloadedNeeded = FALSE;

	_pListenerCallback = NULL;

	return TRUE;
}


// Uninitializes the game system and frees resources. Call this when preparing to exit the game.
void game_UninitSystem( void ) {
	if( _bSystemInitialized ) {

		_bSystemInitialized = FALSE;
		_bGameUnloadedNeeded = FALSE;
	}
}


// Returns TRUE if game_InitSystem() has been called, and FALSE otherwise.
BOOL game_IsInitialized( void ) {
	return _bSystemInitialized;
}


// Loads and initializes everything needed for the specified level.
// Returns TRUE if successful, or FALSE otherwise.
BOOL game_LoadLevel( cchar *pszLevelTitle,
					 BOOL bShowLoadingScreen/*=TRUE*/,
					 const GameInitInfo_t *pGameInit/*=NULL*/ ) {
	u32 i;

	FASSERT( _bSystemInitialized );

	DEVPRINTF( "******** LOAD MARKER - START LOADING LEVEL***********.\n" );
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( pszLevelTitle );
#endif

	// Set difficulty level...
	if( pGameInit ) {
		CDifficulty::SetLevel( pGameInit->nDifficultyLevel );
//	} else {
//		CDifficulty::Reset();
	}

    _hResFrame = fres_GetFrame();

	_bCompletedLevel = FALSE;
	_bGameUnloadedNeeded = TRUE;

    // set the player profile array and control modes
	for( i=0; i < MAX_PLAYERS; i++ ) {
		if( pGameInit && i < pGameInit->nNumPlayers ) {
			playerprofile_Set( i, pGameInit->apProfile[i] );
		} else {
			playerprofile_Set( i, NULL );
		}

		_aeControlMode[i] = CONTROLMODE_NORMAL;
	}
	MultiplayerMgr.PreLoadInitLevel( pGameInit );

	// Init spawner system...
	if( !CFDebrisSpawner::InitSpawnerSystem( _DEBRIS_SPAWNER_COUNT ) ) {
		goto _ExitWithError;
	}

	if( !level_Load( pszLevelTitle, bShowLoadingScreen, (pGameInit) ? pGameInit->pwszLevelDisplayHeading : NULL ) ) {
		// Error loading level...
		goto _ExitWithError;
	}
	_UPDATE_LOADSCREEN
		aimain_InitSystemPostWorldLoad();
	_UPDATE_LOADSCREEN

		if( !_PostWorldLoadGameInit( pGameInit ) ) {
			goto _ExitWithError;
		}

	// finally, kill the load screen
	loadingscreen_Uninit();

	DEVPRINTF( "******** LOAD MARKER - END OF LOADING '%s'***********.\n", Level_aInfo[Level_nLoadedIndex].pszWorldResName );
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif

	// Level loaded successfully...
	return TRUE;

_ExitWithError:
	loadingscreen_Uninit();
	game_UnloadLevel();
	fres_ReleaseFrame( _hResFrame );
#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	return FALSE;
}


// Loads and initializes everything needed for the generic level, using the specified world file.
// Returns TRUE if successful, or FALSE otherwise.
BOOL game_LoadGenericDebugLevel( cchar *pszWorldResName ) {
	FASSERT( _bSystemInitialized );
	
	_hResFrame = fres_GetFrame();

	// I PULLED THE LOAD SCREEN MOVIE WHILE LOADING THIS LEVEL TO SIMPLIFIY THE CODE A BIT, IT IS ONLY A DEBUG LEVEL AFTER ALL

	_bCompletedLevel = FALSE;
	_bGameUnloadedNeeded = TRUE;
	CFScriptSystem::SetMonitorsOn( Gameloop_bDrawDebugInfo );

	for (int nP = 0; nP < MAX_PLAYERS; nP++) {
		_aeControlMode[nP] = CONTROLMODE_NORMAL;
	}

	MultiplayerMgr.PreLoadInitLevel( NULL );

	// Init spawner system...
	if( !CFDebrisSpawner::InitSpawnerSystem( _DEBRIS_SPAWNER_COUNT ) ) {
		goto _ExitWithError;
	}

	if( !level_LoadGenericLevel( pszWorldResName ) ) {
		// Error loading level...
		goto _ExitWithError;
	}
		
	if( !_PostWorldLoadGameInit( NULL ) ) {
		goto _ExitWithError;
	}

	// Level loaded successfully...
	return TRUE;

_ExitWithError:
	level_Unload();
	game_UnloadLevel();
	fres_ReleaseFrame( _hResFrame );
	return FALSE;
}

// Unloads the current level. This must be called before another level can be loaded.
void game_UnloadLevel( void ) {
	if( !_bSystemInitialized ) {
		return;
	}

	if( !_bGameUnloadedNeeded ) {
		return;
	}
	_bGameUnloadedNeeded = FALSE;

	CFDebrisSpawner::UninitSpawnerSystem();

	// clear the script error display system so that it doesn't bleed over to the load screen or next level
	CFScriptSystem::ClearErrorMonitor();
	CFScriptSystem::ClearMessageMonitor();
	CFScriptSystem::SetMonitorsOn( FALSE );

	#if SAS_ACTIVE_USER == SAS_USER_STEVE
		user_steve_PrelevelShutdown();
	#endif

	CFDebris::UninitDebrisSystem();	
	
	testbots_Cleanup();

	fexplosion_UninitExplosionSystem();

	for( int nP = 0; nP < CPlayer::m_nPlayerCount; nP++ ) {
		if( Player_aPlayer[nP].m_pEntityOrig ) {
			fdelete( Player_aPlayer[nP].m_pEntityOrig );
			Player_aPlayer[nP].m_pEntityOrig = NULL;
			Player_aPlayer[nP].m_pEntityCurrent = NULL;
		}
		Player_aPlayer[nP].m_Reticle.Destroy();
		Player_aPlayer[nP].m_Hud.Destroy( nP );

		// NKM - Clean up any weapons that we may have created
		CInventory *pInv = CPlayer::GetInventory( nP, FALSE );
		if( pInv ) {
			for( u32 i = 0; i < pInv->m_uNumPickupWeapons; ++i ) {
				// If we have been used, we will get deleted in another way
				if( !pInv->m_PickupWeaponInfo[i].bUsed ) {
					fdelete( pInv->m_PickupWeaponInfo[i].pWeapon );
					pInv->m_PickupWeaponInfo[i].pItem = NULL;
					pInv->m_PickupWeaponInfo[i].pWeapon = NULL;
				}
			}
		}
	}

	Game_pFullscreenRenderTarget = NULL;
	
	CFScriptSystem::DoEndScripts();

	AlarmSys_UninitLevel();
	CBotPowerupFx::UninitLevel();
	CBlinkGlow::UninitLevel();
	CBlinkSpeed::UninitLevel();
	CLightningBoltGroup::UninitLevel();
//	CBlinkShell::UninitLevel();
	CLaserBeam::UninitLevel();
	CEBoomer::UninitLevel();
	CESwitch::UninitLevel();
	CDoorEntity::UninitLevel();
	bartersystem_UninitLevel();
	CTalkSystem2::UninitLevel();
	CMAScriptTypes::UninitLevel();
	CFMotionObj::LevelUninit();
	CPauseScreen::LevelUninit();
	CHud2::EndLevel();
	CWorkable::LevelUninit();
	CCollectable::UnInitLevel();
	CSpawnSys::UninitLevel();

	// this must happen before level_Unload(), that way the level index is still valid
	CPlayer::UninitLevel( Level_aInfo[Level_nLoadedIndex].nLevel, _bCompletedLevel );
	CBot::UninitLevel();
	CBotSwarmer::UninitLevel();
	CFPhysicsObject::UninitLevel();
	CVehicleLoader::UninitLevel();
	eparticlepool_UninitLevel();
	CLightPool::ReturnAllToFreePool();
	MultiplayerMgr.UninitLevel();
	CFXMeshBuildPartMgr::UninitLevel();
	wpr_system_End();

	// this is were the frame that was gotten before the world load will be released
	level_Unload();

//	CSplat::KillAll();
	tracer_DestroyAllTracers();
//	potmark_KillAll();
	CFDecal::KillAll();
	CFXStreamerEmitter::KillAllStreamers();
	CFXMagmaBomb::EndAllEffects();
	CEBotFire::EndAll();

	gamecam_PostLevelCleanup();

	// release the initial frame so memory is exactly where it was on loading the game...
	fres_ReleaseFrame( _hResFrame );

	// finally, reset the game init pointer that we used to setup this level
	_pCurrentGameInit = NULL;
}

GameTypes_e game_GetCurrentType( void ) {
	if( !_pCurrentGameInit ) {
		// not really a valid game type
		return GAME_TYPES_DEVELOPMENT;
	}

	if( !_pCurrentGameInit->bSinglePlayer ) {
		return GAME_TYPES_MULTIPLAYER;
	}

	if( _pCurrentGameInit->bReplayingALevel ) {
		return GAME_TYPES_REPLAY_LEVEL;
	}
	
	return GAME_TYPES_ADVENTURE;
}

BOOL game_IsCurrentGameAReplay( void ) {

	GameTypes_e nGameType = game_GetCurrentType();
	if( nGameType == GAME_TYPES_REPLAY_LEVEL ||
		nGameType == GAME_TYPES_DEVELOPMENT ) {
		return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// game_ControlModeWork is called once per game loop. We check for control'
// mode transitions for every active player.
void game_ControlModeWork() {

#if 0
	if( _nFadeState != _FADE_STATE_NONE ) {
		// don't allow state changes while fading
		return;
	}
#endif
	
	for( s32 nPlayer = 0; nPlayer< (s32)CPlayer::m_nPlayerCount; nPlayer++ ) {
		CPlayer::SetCurrent( nPlayer );
		CHud2* pHud = CHud2::GetHudForPlayer(nPlayer);
		u32 nControlIndex = Player_aPlayer[nPlayer].m_nControllerIndex;

		switch( _aeControlMode[nPlayer] ) {
		
		///////////////////////////////////////////////////////////////////////////////////////////////////
		case CONTROLMODE_NORMAL:
			if( ( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_PAUSE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) ||
				Game_bEnterPauseModeImmediatelyForCutsceneCreation ) {
				// Clear this variable back to false
				Game_bEnterPauseModeImmediatelyForCutsceneCreation = FALSE;

				if( MultiplayerMgr.IsSinglePlayer() ) {
					if( CPauseScreen::Start( ((CBot *)Player_aPlayer[nPlayer].m_pEntityCurrent)->m_pInventory ) ) {
						_aeControlMode[nPlayer] = CONTROLMODE_PAUSESCREEN;
						break;
					}
				} else {
					MultiplayerMgr.StartMenu(nPlayer);
					_aeControlMode[nPlayer] = CONTROLMODE_MULTIPLAYER_OPTIONS;
					break;
				}
			} else {
				if( (Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
					&& pHud->StartWeaponSelect(0, ((CBot *)Player_aPlayer[nPlayer].m_pEntityCurrent)->m_pInventory, TRUE) ) {
					_aeControlMode[nPlayer] = CONTROLMODE_WEAPONSELECT;
					break;
				}
				if( (Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
					&& pHud->StartWeaponSelect(1, ((CBot *)Player_aPlayer[nPlayer].m_pEntityCurrent)->m_pInventory, TRUE) ) {
					_aeControlMode[nPlayer] = CONTROLMODE_WEAPONSELECT;
					break;
				}
			}
			break;
			
		///////////////////////////////////////////////////////////////////////////////////////////////////
		case CONTROLMODE_PAUSESCREEN:
			if( !CPauseScreen::IsActive() ) {
				if( bartersystem_IsActive() ) {
					_aeControlMode[nPlayer] = CONTROLMODE_BARTERSYSTEM;
				} else {
					_aeControlMode[nPlayer] = CONTROLMODE_NORMAL;
				}
			}
			break;
		
		///////////////////////////////////////////////////////////////////////////////////////////////////
		case CONTROLMODE_MULTIPLAYER_OPTIONS:
			if( MultiplayerMgr.MenuDone(nPlayer) ) {
				_aeControlMode[nPlayer] = CONTROLMODE_NORMAL;
			}
			break;
		
		///////////////////////////////////////////////////////////////////////////////////////////////////
		case CONTROLMODE_WEAPONSELECT:
			if( !pHud->IsWSActive() ) {
				_aeControlMode[nPlayer] = CONTROLMODE_NORMAL;
			}
			break;
			
		///////////////////////////////////////////////////////////////////////////////////////////////////
		case CONTROLMODE_LETTERBOX:
			// We get into this mode through game_EnterLetterBox().
			if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_PAUSE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
				// the start button was pressed...should we abort the cutscene?
				if( _bAllowCutSceneSkip ) {
					if( (_pszCurCutSceneName == NULL) || game_IsCurrentGameAReplay() ||
						(_pCutSceneTable->FindString( _pszCurCutSceneName ) != NULL) ) {
						// The cutscene name was not found in the table or none was specified, so we'll let them skip it.
						gameloop_SetDrawEnabled( FALSE );
						gameloop_SetSwapEnabled( FALSE );
						floop_SetTimeScale( 10.0f );
	//					floop_EnableFixedLoopTimeMode(TRUE);
	//					floop_SetTargetFramesPerSec(4.0f);
	//					gameloop_SetFrameTimeOverride(FALSE);

						_aeControlMode[nPlayer] = CONTROLMODE_LETTERBOXFF;

						CHud2 *pHud2 = CHud2::GetCurrentHud();
						if( pHud2->Transmission_GetAbortWithCutSceneFlag() ) {
							pHud2->TransmissionMsg_Stop( FALSE );
						}
					}
				}
			}
			// Going from this mode back to NORMAL is through game_LeaveLetterBox.
			break;

		///////////////////////////////////////////////////////////////////////////////////////////////////	
		case CONTROLMODE_LETTERBOXFF:
			// We get out of this mode through game_LeaveLetterBox().
			break;
	
		///////////////////////////////////////////////////////////////////////////////////////////////////
		case CONTROLMODE_NONAMBIENTTALK:
			if( !CTalkSystem2::ControlsAreCaptive() ) {
				_EnablePlayerControls( nPlayer );

				_aeControlMode[nPlayer] = CONTROLMODE_NORMAL;
			}
			break;
		
		///////////////////////////////////////////////////////////////////////////////////////////////////	
		case CONTROLMODE_BARTERSYSTEM:
			if( !bartersystem_IsActive() ) {
				game_LeaveBarterMode();
			} else {
				if( ( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_PAUSE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )) {
					FASSERT( MultiplayerMgr.IsSinglePlayer() );
					
					if( CPauseScreen::Start( ((CBot *)Player_aPlayer[nPlayer].m_pEntityCurrent)->m_pInventory ) ) {
						_aeControlMode[nPlayer] = CONTROLMODE_PAUSESCREEN;
					}
				}
			}
			break;
		
		///////////////////////////////////////////////////////////////////////////////////////////////////	
		case CONTROLMODE_MINIGAME:
			// We get out of this mode through game_LeaveMiniGame().
			break;
		}
	}
}


ControlMode_e game_GetControlMode() {
	return _aeControlMode[CPlayer::m_nCurrent];
}


// Called by gameloop once per frame to perform the game's work.
BOOL game_Work( void ) {
	u32 i;

	FASSERT( _bSystemInitialized );
	
	if( _nFadeState == _FADE_STATE_OUT ) {
		// don't run the work code while fading out
		return TRUE;
	}

#if FANG_PLATFORM_WIN
	CBot::m_bAllowPlayerDeath = gameloop_GetPlayerDeath();
#endif

#if FANG_ENABLE_DEV_FEATURES
	// support our auto level load feature, used during development to generate master file optimizer text files
	_fLoadTestTimer += FLoop_fRealPreviousLoopSecs;	
	if( _fLoadTestTimer >= 1.0f &&
		launcher_IsInLoadTestMode() ) {
		launcher_EnterMenus( LAUNCHER_FROM_GAME );
		return TRUE;
	}
	#if 0
		// simulate the end of the game, used for level flow and end of level development
		if( _fLoadTestTimer >= 30.0f ) {
			game_GotoLevel( "next" );
		}
	#endif
#endif

	PROTRACK_BEGINBLOCK("MotionObjWork");
		CFMotionObj::ModuleWork();
	PROTRACK_ENDBLOCK();//"MotionObjWork");

	PROTRACK_BEGINBLOCK("GamePadSample");
		gamepad_Sample();
	PROTRACK_ENDBLOCK();// "GamePadSample"

	if( Level_nLoadedIndex == -1 ) {
		return TRUE;
	}

	// make sure all player controllers are connected
	game_HandleControllersBeingUnplugged();

	PROTRACK_BEGINBLOCK("ControlModeWork");
		game_ControlModeWork();
	PROTRACK_ENDBLOCK();// "ControlModeWork"

	PROTRACK_BEGINBLOCK("CollectableWork");
		CCollectable::Work();
	PROTRACK_ENDBLOCK();// "CollectableWork"

	PROTRACK_BEGINBLOCK("MsgBox");
		CMsgBox::WorkAll();
	PROTRACK_ENDBLOCK();// "MsgBox"

	CHud2::GlobalWork();

	if( !CPauseScreen::IsActive() && 
		_nControllerToPlugIn == _ALL_CONTROLLERS_PLUGGED_IN ) {

		PROTRACK_BEGINBLOCK("CallActiveListCallbacks");
			CEntity::CallActiveListCallbacks();
		PROTRACK_ENDBLOCK();

		PROTRACK_BEGINBLOCK("CFVerletWork");
			CFVerlet::WorkAll();
		PROTRACK_ENDBLOCK();//  "CFVerletWork"

		PROTRACK_BEGINBLOCK("ZipLineWork");
			CEZipLine::WorkAll();
		PROTRACK_ENDBLOCK();// "ZipLineWork"

    	PROTRACK_BEGINBLOCK("EntityAutoWork");
			CEntity::CallAllAutoWorks();
		PROTRACK_ENDBLOCK();// "EntityAutoWork"

		PROTRACK_BEGINBLOCK("LevelWork");
			level_Work();
		PROTRACK_ENDBLOCK();// "LevelWork"

		PROTRACK_BEGINBLOCK("FWorld");
			smoketrail_WorkAll();
//			fworld_Work();
		PROTRACK_ENDBLOCK();// "FWorld"

		PROTRACK_BEGINBLOCK("AIMain");
			aimain_Work();
			CBotSwarmer::WorkSystem();
			CSpawnSys::Work();
			CTripwire::TripwireList_Work();
	PROTRACK_ENDBLOCK();// "AIMain"

		PROTRACK_BEGINBLOCK("ScriptSys");
			CFScriptSystem::Work();	
		PROTRACK_ENDBLOCK();// "ScriptSys"

		PROTRACK_BEGINBLOCK("CWorkable");
			CWorkable::SystemWork();
		PROTRACK_ENDBLOCK();// "CWorkable"

		PROTRACK_BEGINBLOCK("PlayerWork");
			for( i=0; i<(u32)CPlayer::m_nPlayerCount; i++ ) {
				CPlayer::SetCurrent( i );
				gamecam_SetActiveCamera( PLAYER_CAM( i ) );
				gamecam_SetViewportAndInitCamStack();

				// For now, we set the same listener for each player. I suspect
				// that we should be able to set up a separate listener for
				// each at some point, but right now that doesn't work.
				if( !_pListenerCallback ) {
					CFXfm xfmListener;
					xfmListener.BuildFromMtx(*CPlayer::m_pCurrent->m_pEntityCurrent->MtxToWorld());
					faudio_SetListenerOrientation(i, &xfmListener);
				} else {
					_pListenerCallback( i );
				}

				Player_aPlayer[i].Work();
			}
		PROTRACK_ENDBLOCK();// "PlayerWork"

		//PROTRACK_BEGINBLOCK("PotmarkWork");
		//	potmark_Work();
		//PROTRACK_ENDBLOCK();// "PotmarkWork"

		PROTRACK_BEGINBLOCK("TracerWork");
			tracer_Work();
		PROTRACK_ENDBLOCK();// "TracerWork"

		PROTRACK_BEGINBLOCK("Flamer");
			CFlamer::WorkAll();
		PROTRACK_ENDBLOCK();// "Flamer"

		PROTRACK_BEGINBLOCK("Tether");
			CTether::WorkAll();
		PROTRACK_ENDBLOCK();// "Tether"

		CWeaponRecruiter::WorkAll();

		CReticle::WorkAll();

		PROTRACK_BEGINBLOCK("FXShockwave");
			CFXShockwave::Work();
		PROTRACK_ENDBLOCK();// "FXShockwave"

		//PROTRACK_BEGINBLOCK("CSplat");
		//	CSplat::Work();
		//PROTRACK_ENDBLOCK();//"CSplat"

		PROTRACK_BEGINBLOCK("CSplat");
			CFDebris::Work();
		PROTRACK_ENDBLOCK();//"CSplat"

		PROTRACK_BEGINBLOCK("FXEMPBlast");
			CFXEMPBlast::Work();
		PROTRACK_ENDBLOCK();// "FXEMPBlast"

		PROTRACK_BEGINBLOCK("CSloshTank");
			CSloshTank::Work();
		PROTRACK_ENDBLOCK();// "CSloshTank"
		
		PROTRACK_BEGINBLOCK("CFXSlower");
			CFXSlower::Work();
		PROTRACK_ENDBLOCK();// "CFXSlower"

		AlarmSys_Work();

		PROTRACK_BEGINBLOCK("fexplosion");
			fexplosion_Work();
		PROTRACK_ENDBLOCK();// "fexplosion"

		CDamage::Work();
		CEntity::CallAllMarkedRemoveFromWorld();
		fparticle_Work();
		CFLightGroupMgr::Work();
	}

	#if !FANG_PRODUCTION_BUILD
		if( FLoop_bGamePaused ) {// game is paused
			if( CFTextMonitor::IsPaused() ) {
				// allow unpausing after text monitor paused.  (for script errors)
				if( Gamepad_aapSample[Player_aPlayer[0].m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
					CFTextMonitor::Pause( FALSE );
					CFScriptSystem::ClearErrorMonitor();
				}
			}
		}
	#endif

	for( i=0; i<(u32)CPlayer::m_nPlayerCount; i++ ) {
		CPlayer::SetCurrent( i );
		Player_aPlayer[i].HandleTogglingOfOnscreenText();
	}

	PROTRACK_BEGINBLOCK("CMASTWork");
		CMAST_BotWrapper::Work();
		CMAST_Timer::Work();
		CMAST_CamAnimWrapper::Work();
	PROTRACK_ENDBLOCK();// "CMASTWork"

	PROTRACK_BEGINBLOCK("LetBoxWork");
		letterbox_Work();
	PROTRACK_ENDBLOCK();// "LetBoxWork"

	PROTRACK_BEGINBLOCK("TalkSysWork");
		CTalkSystem2::Work();
	PROTRACK_ENDBLOCK();// "TalkSysWork"

	// CHud2::Work should come after CTalkSystem2::Work().
	PROTRACK_BEGINBLOCK("HUD/Pause");
	// TODO: CPauseScreen needs to work for multiplayer...
		CPauseScreen::Work(((CBot *)Player_aPlayer[0].m_pEntityCurrent)->m_pInventory);
		for( i=0; i<(u32)CPlayer::m_nPlayerCount; i++ ) {
			CPlayer::SetCurrent( i );
			CHud2::GetHudForPlayer(i)->Work(((CBot *)Player_aPlayer[i].m_pEntityCurrent)->m_pInventory);
		}
	PROTRACK_ENDBLOCK();// "HUD/Pause"

	PROTRACK_BEGINBLOCK("LaserBeamWork");
		CLaserBeam::Work();
	PROTRACK_ENDBLOCK();// "LaserBeamWork"

//	PROTRACK_BEGINBLOCK("BlinkShellWork");
//		CBlinkShell::Work();
//	PROTRACK_ENDBLOCK();// "BlinkShellWork"

	PROTRACK_BEGINBLOCK("LightningBoltGroupWork");
		CLightningBoltGroup::Work();
	PROTRACK_ENDBLOCK();// "LightningBoltGroupWork"

	PROTRACK_BEGINBLOCK("BlinkSpeedWork");
		CBlinkSpeed::Work();
	PROTRACK_ENDBLOCK();// "BlinkSpeedWork"

	PROTRACK_BEGINBLOCK("BlinkGlowWork");
		CBlinkGlow::Work();
	PROTRACK_ENDBLOCK();// "BlinkGlowWork"

	PROTRACK_BEGINBLOCK("BarterSysWork");
		bartersystem_Work();
	PROTRACK_ENDBLOCK();//  "BarterSysWork"

	PROTRACK_BEGINBLOCK("Camera");
		gamecam_Work();
	PROTRACK_ENDBLOCK();// "Camera"

	PROTRACK_BEGINBLOCK("MagmaBomb FX");
		CFXMagmaBomb::Work();
	PROTRACK_ENDBLOCK();// MagmaBomb FX

	PROTRACK_BEGINBLOCK("StreamerWork");
		CFXStreamerEmitter::Work();
	PROTRACK_ENDBLOCK();// "StreamerWork"

	PROTRACK_BEGINBLOCK("DecalWork");
		CFDecal::Work();
	PROTRACK_ENDBLOCK();// "DecalWork"

	PROTRACK_BEGINBLOCK("CheckpointWork");
		checkpoint_Work();
	PROTRACK_ENDBLOCK();// "CheckpointWork"

	MultiplayerMgr.Work();

	testbots_Work();
	
	CEntity *pPlayerEntity = CPlayer::m_pCurrent->m_pEntityCurrent;
	CFMtx43A *pWorldMtx = pPlayerEntity->MtxToWorld();
	
	fsh_RegisterPlayerPos(pWorldMtx->m_vPos);

	return TRUE;
}

// Called by gameloop once per frame to perform the game's drawing.
BOOL game_Draw( void ) {
	if( Level_nLoadedIndex != -1 ) {
		// Get the current (default) viewport for later restoration
		FViewport_t *pDefaultVP = fviewport_GetActive();

		gamecam_SetActiveCamera( PLAYER_CAM(0) );

		fvis_FrameBegin();

		// For now, we handle the pause screen globally, since we haven't yet
		// implemented it for multiplayer. Soon this will change.
		if ( CPauseScreen::IsActive() ) {
			_SetupCameraAndClearViewport();
			CPauseScreen::Draw( ((CBot *)Player_aPlayer[0].m_pEntityCurrent)->m_pInventory );
			CMsgBox::DrawAll();

			for (s32 nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++) {
				CHud2::GetHudForPlayer(nPlayer)->NoDraw();
			}
		} else {
			BOOL bWrend = TRUE;

			// ftex_HandleRenderTargets should never return false in multiplayer...
			// Just in case it does, we ignore it below.
			// clear the callback functions, since we don't want fdrawn stuff in reflections
			bWrend = ftex_HandleRenderTargets();

			#if _USE_FDRAW_IN_SCENE_RENDER
				// Hook perspective draw into the main scene render
				FRenderSortCallback_t *pPrevCB = frs_RegisterRenderCallback( FRS_PREPOINTSPRITE_CALLBACK, _RenderSortCallbackPrePointSprites );
			#endif

			// First, draw the main view for each player
			s32 nPlayer;
			for( nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++ ) {
				CPlayer::SetCurrent( nPlayer );
				gamecam_SetActiveCamera( PLAYER_CAM(nPlayer) );

				Player_aPlayer[nPlayer].PreRender();
				if( bWrend || (CPlayer::m_nPlayerCount > 1) ) {
					_SetupCameraAndClearViewport();
					_DrawMainScene_Persp();
				}

				if( Game_pFcnFullscreenRenderTargetReadyCallback ) {
					Game_pFcnFullscreenRenderTargetReadyCallback();
				}

				gamecam_SetViewportAndInitCamStack();

				_DrawDebugStuffForPlayer_Persp( nPlayer );
				_DrawHUD_Persp( nPlayer );
				Player_aPlayer[nPlayer].PostRender();
			}

			#if _USE_FDRAW_IN_SCENE_RENDER
				// Hook perspective draw into the main scene render
				frs_RegisterRenderCallback( FRS_PREPOINTSPRITE_CALLBACK, pPrevCB );
			#endif


			// Set the draw renderer just once
			frenderer_Push( FRENDERER_DRAW, NULL );
			CFXfm::InitStack();

			// Draw the reticles (it would make more sense for each player to
			// draw his own reticle, but if it ain't broke...)
			_DrawReticles();

			// Draw the regular hud on top of the reticle
			for (nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++) {
				CPlayer::SetCurrent( nPlayer );
				gamecam_SetActiveCamera( PLAYER_CAM(nPlayer) );
				_DrawHUD_OrthoForPlayer(nPlayer);
				_DrawPlayerText_Ortho( nPlayer );
			}

			// Draw Hud elements that handle all players
			_DrawHUD_OrthoForAll();

			
			frenderer_Pop();
		}

		fvis_SetupShadows();

		fvis_FrameEnd();

		// Draw any full-screen debug text etc.
		gamecam_SetActiveCamera( PLAYER_CAM(0));
		gamecam_SetViewportAndInitCamStack();

		// Set to a full-screen viewport
		fviewport_SetActive(pDefaultVP);

		_DrawDebugStuff_Persp();		
		
		// fade the screen in or out...
		if( _nFadeState != _FADE_STATE_NONE ) {
			
			_fFade_Timer += FLoop_fRealPreviousLoopSecs;
			
			switch( _nFadeState ) {
			
			case _FADE_STATE_IN:
				if( ( Level_fStartingFadeSecs == 0.0f ) ||
					( _fFade_Timer >= Level_fStartingFadeSecs ) ||
					( CPauseScreen::IsActive() ) ||
					( CFTextMonitor::IsPaused() ) ) {
					// something is up, end the fade in
					_nFadeState = _FADE_STATE_NONE;
				} else {
					// fade the whole screen in
					game_DrawSolidFullScreenOverlay( _fFade_Timer * (1.0f/Level_fStartingFadeSecs), 0.0f );
					ftext_ClearAllPending();
				}
				break;
				
			case _FADE_STATE_OUT:
				if( ( Level_fEndingFadeSecs == 0.0f ) ||
					( _fFade_Timer >= Level_fEndingFadeSecs ) ||
					( CPauseScreen::IsActive() ) ||
					( CFTextMonitor::IsPaused() ) ) {
					// something is up, end the fade out
					_nFadeState = _FADE_STATE_NONE;
					game_DrawSolidFullScreenOverlay( 0.0f, 0.0f );
					
					faudio_SetMusicMasterVol( 0.0f );
					faudio_SetSfxMasterVol( 0.0f );	
					
					if( MultiplayerMgr.IsSinglePlayer() ) {
						wpr_levelcomplete_ScheduleSinglePlayerScreen();
					} else {
						launcher_EndOfGameDecisions( LAUNCHER_DECISION_LOAD_NEXT );
					}
				} else {
					// fade the whole screen out
					f32 fUnitBlackness = 1.0f - ( _fFade_Timer * (1.0f/Level_fEndingFadeSecs) );
					game_DrawSolidFullScreenOverlay( fUnitBlackness, 0.0f );
					faudio_SetMusicMasterVol( fUnitBlackness * CPlayer::m_fMusicVolumeCache );
					faudio_SetSfxMasterVol( fUnitBlackness * CPlayer::m_fSfxVolumeCache );					
				}
				ftext_ClearAllPending();			
				break;
			}
		}
			
		CMsgBox::DrawAll();
	}

	return TRUE;
}


BOOL game_BeginCutScene( cchar *pszCutSceneTitle, BOOL bImmediate )
{
	CPlayer::m_pCurrent->DisableEntityControl();
	CAIBrain* pBrain = Player_aPlayer[0].m_pEntityCurrent->AIBrain();
	if (Player_aPlayer[0].m_pEntityCurrent ->TypeBits() & ENTITY_BIT_BOT)
	{
		((CBot*)Player_aPlayer[0].m_pEntityCurrent)->HeadStopLook();
	}
	aibrainman_Activate(pBrain);
	ai_NotifyCutSceneBegin();
	CBot::DisableBotDamageGlobally();
	CBot::SetCutscenePlaying( TRUE );
	if (fclib_strlen(pszCutSceneTitle) > 1)
	{
		return game_EnterLetterbox(pszCutSceneTitle, bImmediate);
	}
	else
	{
		return game_EnterLetterbox(NULL, bImmediate);
	}
}

BOOL game_EndCutScene( BOOL bImmediate )
{
	aibrainman_Deactivate(Player_aPlayer[0].m_pEntityCurrent->AIBrain());
	CPlayer::m_pCurrent->EnableEntityControl();
	aibrainman_ConfigurePlayerBotBrain(Player_aPlayer[0].m_pEntityCurrent->AIBrain(), 0);
	ai_NotifyCutSceneEnd();
	if (Player_aPlayer[0].m_pEntityCurrent ->TypeBits() & ENTITY_BIT_BOT)
	{
		((CBot*)Player_aPlayer[0].m_pEntityCurrent)->HeadLook();
	}
	CBot::EnableBotDamageGlobally();
	CBot::SetCutscenePlaying( FALSE );
	return game_LeaveLetterbox(bImmediate);
}


BOOL game_EnterLetterbox( cchar *pszCutSceneName/*=NULL*/, BOOL bImmediate/*=FALSE*/, BOOL bAllowMovieSkip/*=TRUE*/ ) {

	if( _aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_NORMAL ) {
		return FALSE;
	}

	letterbox_SlideOn( bImmediate );
	_pszCurCutSceneName = gstring_Main.AddString( pszCutSceneName );
	_bAllowCutSceneSkip = bAllowMovieSkip;

	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_LETTERBOX;

	return TRUE;
}


BOOL game_LeaveLetterbox( BOOL bImmediate ) {

	if( (_aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_LETTERBOX) && (_aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_LETTERBOXFF) ) {
		return FALSE;
	}

	_pCutSceneTable->AddString( _pszCurCutSceneName, TRUE );
	letterbox_SlideOff( bImmediate );
	floop_SetTimeScale( 1.0f );
	gameloop_SetDrawEnabled( TRUE );
	gameloop_SetSwapEnabled( TRUE );
	
	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_NORMAL;

	return TRUE;
}


BOOL game_EnterNonAmbientTalk( void ) {
	if( _aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_NORMAL ) {
		return FALSE;
	}

	_DisablePlayerControls(CPlayer::m_nCurrent);
	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_NONAMBIENTTALK;

	return TRUE;
}


BOOL game_EnterBarterMode( void ) {
	if( _aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_NORMAL ) {
		return FALSE;
	}

	_DisablePlayerControls(CPlayer::m_nCurrent);
	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_BARTERSYSTEM;

	return TRUE;
}

BOOL game_LeaveBarterMode( void ) 
{
	_EnablePlayerControls(CPlayer::m_nCurrent);
	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_NORMAL;
	return TRUE;
}

BOOL game_EnterMiniGameMode( void ) {
	if( _aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_NORMAL ) {
		return FALSE;
	}

	_DisablePlayerControls(CPlayer::m_nCurrent);
	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_MINIGAME;

	return TRUE;
}


BOOL game_LeaveMiniGameMode( void ) {
	if( _aeControlMode[CPlayer::m_nCurrent] != CONTROLMODE_MINIGAME ) {
		return FALSE;
	}

	_EnablePlayerControls(CPlayer::m_nCurrent);
	_aeControlMode[CPlayer::m_nCurrent] = CONTROLMODE_NORMAL;

	return TRUE;
}


////////////////////////////////////////
// private prototypes:

static void _SetupCameraAndClearViewport( void ) {
	gamecam_SetViewportAndInitCamStack();

	#if !FANG_PLATFORM_WIN
		// Clear the viewport to black...
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
	#else
		// Under windows, clear the viewport to the user specified color...
		CFColorRGB RGB;
		gameloop_GetBackgroundColor( RGB );
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, RGB.fRed, RGB.fGreen, RGB.fBlue, 1.0f, 0 );
	#endif
}

static void _DrawMainScene_Persp( void ) {
	CFCamera *pCamera;

	// Local copy of player number to help the compiler...
	s32 nPlayerIndex = CPlayer::m_nCurrent;

	// Draw skybox...
	frenderer_Push( FRENDERER_MESH, NULL );
		pCamera = gamecam_GetActiveCamera();
		Player_aPlayer[nPlayerIndex].m_SkyBox.Draw( &pCamera->GetFinalXfm()->m_MtxR.m_vPos );
	frenderer_Pop();

	// Add muzzle flash lights to the world...
	CMuzzleFlash::AddLights();

	// Draw pre-bot-mesh effects...
	CSloshTank::DrawAll();

	frenderer_Push(FRENDERER_NONE, NULL);

	// Draw world...
	#if !FANG_PRODUCTION_BUILD
		GameCam_DebugModes_e eCamMode;
		pCamera = gamecam_GetCameraBeingDebugged( &eCamMode );

		if( !pCamera ) {
			fvis_Draw( NULL, FALSE, nPlayerIndex, Gameloop_bDrawDebugInfo );
		} else if( eCamMode == GAMECAM_DEBUG_MODES_FREE_CAM ) {
			fvis_Draw( NULL, TRUE, nPlayerIndex, Gameloop_bDrawDebugInfo );
		} else {
			if( !Game_bCameraPOVVisibility ) {
				fvis_Draw( &pCamera->GetFinalXfm()->m_MtxR, FALSE, nPlayerIndex, Gameloop_bDrawDebugInfo );
			} else {
				fvis_Draw( NULL, FALSE, nPlayerIndex, Gameloop_bDrawDebugInfo );
			}
		}
	#else
		fvis_Draw( NULL, FALSE, nPlayerIndex );
	#endif

	frenderer_Pop();	

	frenderer_Push( FRENDERER_DRAW, NULL );
		CFXShockwave::Draw();
		CTether::DrawAll();
	frenderer_Pop();

#if !_USE_FDRAW_IN_SCENE_RENDER
	// Draw effects...
	frenderer_Push( FRENDERER_DRAW, NULL );
//		potmark_Draw();
		tracer_Draw();
		CEZipLine::DrawAll();
		CFDecal::Draw();
		CFXStreamerEmitter::Draw();
		// we should really have bot::Draw function that could call this, but...
		if (Player_aPlayer[nPlayerIndex].m_pEntityCurrent &&
			Player_aPlayer[nPlayerIndex].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT &&
			((CBot *)Player_aPlayer[nPlayerIndex].m_pEntityCurrent)->GetPowerupFx())
		{
			((CBot *)Player_aPlayer[nPlayerIndex].m_pEntityCurrent)->GetPowerupFx()->Draw();
		}

		CFXShockwave::Draw();
//		CSplat::Draw();
//		explosion_Draw();
		CFlamer::DrawAll();
		CTether::DrawAll();
		CWeaponRecruiter::DrawAll();
		CBotScientist::DrawAllFX();
		CFXMagmaBomb::DrawAll();
		CBotScout::DrawBeams();
		BotEliteGuard_DrawBeams();

		fexplosion_Draw( gamecam_GetActiveCamera() );

		#if( SAS_ACTIVE_USER == SAS_USER_STEVE )
			user_steve_Draw();
		#endif
	frenderer_Pop();
#endif

	level_Draw();

#if !_USE_FDRAW_IN_SCENE_RENDER
	frenderer_Push(FRENDERER_DRAW, NULL);
		CLaserBeam::Draw();
		CMuzzleFlash::Draw();
		CBlinkShell::FDraw();
		CLightningBoltGroup::FDraw();
		CBlinkSpeed::FDraw();
		CBlinkGlow::FDraw();
		smoketrail_PostDrawAll();
	frenderer_Pop();
#endif

	CFXEMPBlast::Draw();
	CFXSlower::Draw();

	CMuzzleFlash::RemoveLights();
}

static void _DrawPlayerText_Ortho( s32 nPlayer ) {

	// If this is a multiplayer game, don't draw any text if the player has
	// quit or has a menu up
	if ( MultiplayerMgr.ExclusiveText(nPlayer) )
		return;

	CPlayer& rPlayer = Player_aPlayer[nPlayer];
	CEntity *pEntity = rPlayer.m_pEntityCurrent;

	fviewport_SetActive(rPlayer.m_pViewportSafeOrtho3D);

	if( pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOTGLITCH) ) {
		CBotGlitch *pGlitch = (CBotGlitch*) pEntity;
		pGlitch->DrawText();
	} else if( pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOTGRUNT) ) {
		CBotGrunt *pGrunt = (CBotGrunt*) pEntity;
		pGrunt->DrawText();
	}

	rPlayer.DrawText();
}

static void _DrawDebugStuff_Persp( void ) {
#if !FANG_PRODUCTION_BUILD
	if( Gameloop_bDrawDebugInfo ) {// allows the user to turn off some elements so that we can capture better screenshots	
		/////////////////////////////////////////
		// put any fullscreen debug stuff here...
		/////////////////////////////////////////

	}
#endif
}

static void _DrawDebugStuffForPlayer_Persp( s32 nPlayer ) {
#if !FANG_PRODUCTION_BUILD
	if( Gameloop_bDrawDebugInfo ) {// allows the user to turn off some elements so that we can capture better screenshots	
		//////////////////////////////////////////////
		// put any player viewport debug stuff here...
		//////////////////////////////////////////////
		testbots_Draw( nPlayer );
	
		gamecam_Draw();
		
        BOOL bDebugAI;
		BOOL bShowEntityInfo;

		#if FANG_PLATFORM_XB || FANG_PLATFORM_GC
			// Debug AI in consoles since everything uses the second controller...
			bDebugAI = TRUE;
			bShowEntityInfo = TRUE;
		#else
			bDebugAI = gameloop_GetDebugAI();
			bShowEntityInfo = gameloop_GetShowEntityInfo();
		#endif

		if( CPlayer::m_nPlayerCount == 1 ) {
			if( bDebugAI ) {
				aimain_Draw();
			}

			// Disable this function for more than one player, because we don't want
			// to be sampling the controller
			if( bShowEntityInfo ) {
				CEntity::DrawAllDebugText();
			}
		}
	}
#endif
}

static void _DrawHUD_Persp( int nPlayer ) {
	frenderer_Push( FRENDERER_DRAW, NULL );
	fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportPersp3D);
	CEProj_Cleaner::DrawTargets( nPlayer );
	CFPhysicsObject::DebugDrawWork();

	CFVerlet::DrawAll();

	frenderer_Pop();
}

static void _DrawReticles( void ) {
	if( Gameloop_bDrawHUD ) {// allows the user to turn off some elements so that we can capture better screenshots
		CReticle::DrawAll();
	}
}

static void _DrawHUD_OrthoForAll( void ) {
	bartersystem_Draw();

	MultiplayerMgr.Draw();

	if( Gameloop_bDrawDebugInfo ) {// allows the user to turn off some elements so that we can capture better screenshots
		checkpoint_Draw();
	}

	// Draw the space between viewports if appropriate
	splitscreen_DrawBorders();	
}

static void _DrawHUD_OrthoForPlayer( int nPlayer ) {
	fviewport_SetActive( Player_aPlayer[nPlayer].m_pViewportOrtho3D );
	CBot::DrawOrthoEffects( nPlayer );

	if( Gameloop_bDrawHUD ) {// allows the user to turn off some elements so that we can capture better screenshots
        CHud2::GetHudForPlayer(nPlayer)->Draw( ((CBot *)Player_aPlayer[nPlayer].m_pEntityCurrent)->m_pInventory );
	}

	fviewport_SetActive( Player_aPlayer[nPlayer].m_pViewportOrtho3D );
	letterbox_Draw();
}

static void _FullscreenRenderTargetCallback( void *pUserData ) {
	_SetupCameraAndClearViewport();
	
	#if _USE_FDRAW_IN_SCENE_RENDER
		// Hook perspective draw into the main scene render
		FRenderSortCallback_t *pPrevCB = frs_RegisterRenderCallback( FRS_PREPOINTSPRITE_CALLBACK, _RenderSortCallbackPrePointSprites );
	#endif
	
	_DrawMainScene_Persp();
	
	#if _USE_FDRAW_IN_SCENE_RENDER
		// Hook perspective draw into the main scene render
		frs_RegisterRenderCallback( FRS_PREPOINTSPRITE_CALLBACK, pPrevCB );
	#endif
}

static BOOL _CreatePlayerBot( int nPlayer ) {
	CAIBrain *pBlinkBrain = NULL;

	// Get the entity name
	char szName[16];
	sprintf(szName, "Player%d", nPlayer);

	// Get the next starting point
	const CFMtx43A* pStartMtx = MultiplayerMgr.NextStartPoint();
	if( !testbots_OverridePlayerBot( &Player_aPlayer[nPlayer], pStartMtx ) )
	{
		if( fclib_stricmp("WERRreactr2", Level_aInfo[Level_nLoadedIndex].pszWorldResName) == 0)
		{
			// Allocate a new player bot...
			Player_aPlayer[nPlayer].m_pEntityOrig = fnew CBotSlosh;
			if( Player_aPlayer[nPlayer].m_pEntityOrig == NULL ) 
			{
				DEVPRINTF( "game::_CreatePlayerBot(): Not enough memory to create Player %d's bot.\n", nPlayer );
				goto _ExitWithError;
			}

			if( !((CBotSlosh *)Player_aPlayer[nPlayer].m_pEntityOrig)->Create( nPlayer, FALSE, szName, pStartMtx, "Default" ))
			{
				goto _ExitWithError;
			}
		} else if( fclib_stricmp( "WEWRresrch3", Level_aInfo[Level_nLoadedIndex].pszWorldResName ) == 0 ) {
			// Allocate a new player bot...
			Player_aPlayer[nPlayer].m_pEntityOrig = fnew CBotKrunk;
			if( Player_aPlayer[nPlayer].m_pEntityOrig == NULL ) {
				DEVPRINTF( "game::_CreatePlayerBot(): Not enough memory to create Player %d's bot.\n", nPlayer );
				goto _ExitWithError;
			}

			if( !((CBotKrunk *)Player_aPlayer[nPlayer].m_pEntityOrig)->Create( nPlayer, FALSE, szName, pStartMtx , "Default") ) {
				goto _ExitWithError;
			}
		} else if( fclib_stricmp( "WEWJjourn03", Level_aInfo[Level_nLoadedIndex].pszWorldResName ) == 0 ) {
			// Allocate a new player bot...
			Player_aPlayer[nPlayer].m_pEntityOrig = fnew CBotMozer;
			if( Player_aPlayer[nPlayer].m_pEntityOrig == NULL ) {
				DEVPRINTF( "game::_CreatePlayerBot(): Not enough memory to create Player %d's bot.\n", nPlayer );
				goto _ExitWithError;
			}

			if( !((CBotMozer *)Player_aPlayer[nPlayer].m_pEntityOrig)->Create( nPlayer, FALSE, szName, pStartMtx, "Default" ) ) {
				goto _ExitWithError;
			}
		}
		else  
		{
			// Allocate a new player bot...
			Player_aPlayer[nPlayer].m_pEntityOrig = fnew CBotGlitch;
			if( Player_aPlayer[nPlayer].m_pEntityOrig == NULL ) 
			{
				DEVPRINTF( "game::_CreatePlayerBot(): Not enough memory to create Player %d's bot.\n", nPlayer );
				goto _ExitWithError;
			}
			if( !((CBotGlitch *)Player_aPlayer[nPlayer].m_pEntityOrig)->Create( nPlayer, FALSE, szName, pStartMtx, "Default"))
			{
				goto _ExitWithError;
			}
		}
	
		// Remember player entity pointers...
		Player_aPlayer[nPlayer].m_pEntityCurrent = Player_aPlayer[nPlayer].m_pEntityOrig;
	}

	//
	//  Manually stick a dumbed down brain onto the player object!
	//
	pBlinkBrain = Player_aPlayer[nPlayer].m_pEntityOrig->AIBrain();
	if (!pBlinkBrain)
	{
		pBlinkBrain = aibrainman_Create( Player_aPlayer[nPlayer].m_pEntityCurrent );
	}
	FASSERT(pBlinkBrain);
/*	CAIBuilder* pBuilder = CAIBuilder::Request();
	if (pBuilder)
	{
		pBuilder->SetDefaults(

	}
*/	Player_aPlayer[nPlayer].m_pEntityCurrent->SetAIBrain( pBlinkBrain );	//give blink a brain
	aibrainman_AddToWorld(Player_aPlayer[nPlayer].m_pEntityCurrent, FALSE);//FALSE means don't activate (special for player brains)
	// Make sure this brain doesn't do anything on its own...
	aibrainman_ConfigurePlayerBotBrain( pBlinkBrain, nPlayer );
	Player_aPlayer[nPlayer].m_pEntityOrig->EnableAutoWork( FALSE );
	Player_aPlayer[nPlayer].m_pEntityOrig->SetControls( &Player_aPlayer[nPlayer].m_HumanControl );

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	return FALSE;
}

void _DisablePlayerControls(s32 nPlayer) {
	// Get the player currently in control...
	CPlayer* pPlayer = &Player_aPlayer[nPlayer];

	pPlayer->DisableEntityControl();
	CAIBrain* pBrain = pPlayer->m_pEntityCurrent->AIBrain();
	aibrainman_Activate( pBrain );
}

void _EnablePlayerControls(s32 nPlayer) {
	aibrainman_Deactivate( Player_aPlayer[nPlayer].m_pEntityCurrent->AIBrain() );
	aibrainman_ConfigurePlayerBotBrain( Player_aPlayer[nPlayer].m_pEntityCurrent->AIBrain(), 0 );
	Player_aPlayer[nPlayer].EnableEntityControl();
}

static BOOL _PostWorldLoadGameInit( const GameInitInfo_t *pGameInit ) {
	CFCamera *pCam;
	FResFrame_t ResFrame;
	int nPlayerNum;
	
////////////////////////////////////////////////////////////////////
// put modules that don't require the player array to be setup here:
////////////////////////////////////////////////////////////////////
	_UPDATE_LOADSCREEN
		eparticlepool_InitLevel();
	_UPDATE_LOADSCREEN
		// Note: CPlayer::m_nPlayerCount is not set until here
		CPlayer::InitLevel( pGameInit, Level_aInfo[Level_nLoadedIndex].nLevel );
	_UPDATE_LOADSCREEN
		faudio_SetActiveListenerCount( CPlayer::m_nPlayerCount );
	_UPDATE_LOADSCREEN
		if( !CCollectable::InitLevel() ) {
			DEVPRINTF( "_PostWorldLoadGameInit(): CCollectable::InitLevel() failed.\n" );
			goto _ExitStartGameWithError;
		}
	_UPDATE_LOADSCREEN
		CWorkable::LevelInit();
	_UPDATE_LOADSCREEN
		CHud2::InitLevel();
	_UPDATE_LOADSCREEN
		CPauseScreen::LevelInit();
	_UPDATE_LOADSCREEN
		CFMotionObj::LevelInit();
	_UPDATE_LOADSCREEN
		CMAScriptTypes::InitLevel();
	_UPDATE_LOADSCREEN
		CDoorEntity::InitLevel();
	_UPDATE_LOADSCREEN
		CESwitch::InitLevel();
	_UPDATE_LOADSCREEN
		CEBoomer::InitLevel();
	_UPDATE_LOADSCREEN
		CLaserBeam::InitLevel();
	_UPDATE_LOADSCREEN
//		CBlinkShell::InitLevel();
	_UPDATE_LOADSCREEN
		CLightningBoltGroup::InitLevel();
	_UPDATE_LOADSCREEN
		CBlinkSpeed::InitLevel();
	_UPDATE_LOADSCREEN
		CBlinkGlow::InitLevel();
	_UPDATE_LOADSCREEN
		CBotPowerupFx::InitLevel();
	_UPDATE_LOADSCREEN
		CFPhysicsObject::InitLevel();
		CDamage::InitLevel();
		CBotSwarmer::InitLevel();
		BotEliteGuard_InitLevel();
	_UPDATE_LOADSCREEN
		floop_Reset();

	_UPDATE_LOADSCREEN
		if( !wpr_system_InitInGame() ) {
			goto _ExitStartGameWithError;
		}

	_UPDATE_LOADSCREEN
		// Must be called before player bots can be set up
		MultiplayerMgr.PostLoadInitLevel( pGameInit );

	//allow levels to do stuff after inventory is initialized.
	if( Level_aInfo[Level_nLoadedIndex].pFcnLoad ) {
		Level_aInfo[Level_nLoadedIndex].pFcnLoad( LEVEL_EVENT_PRE_PLAYER_BOT_INIT );
	}

///////////////////////////////////////////////////
// setup the player bots and the player array here:
///////////////////////////////////////////////////
	_UPDATE_LOADSCREEN
		ResFrame = fres_GetFrame();

		for( nPlayerNum = 0; nPlayerNum < CPlayer::m_nPlayerCount; nPlayerNum++ ) {
			_UPDATE_LOADSCREEN
				Player_aPlayer[nPlayerNum].m_pEntityOrig = NULL;
				Player_aPlayer[nPlayerNum].m_pEntityCurrent = NULL;

				CPlayer::SetCurrent( nPlayerNum );
			_UPDATE_LOADSCREEN
				// Set up the viewports for this player
				if ( !splitscreen_SetupViewports(nPlayerNum, CPlayer::m_nPlayerCount))
					goto _ExitStartGameWithError;

			_UPDATE_LOADSCREEN
				if( !Player_aPlayer[nPlayerNum].m_Reticle.Create( CReticle::TYPE_NONE, nPlayerNum ) ) {
					DEVPRINTF( "game::_PostWorldLoadGameInit(): Unable to create player reticle.\n" );
					goto _ExitStartGameWithError;
				}
			_UPDATE_LOADSCREEN
				if( !_CreatePlayerBot(nPlayerNum) ) {
					goto _ExitStartGameWithError;
				}
			_UPDATE_LOADSCREEN

				gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(nPlayerNum), (CBot *)Player_aPlayer[nPlayerNum].m_pEntityCurrent );
				gamecam_SetActiveCamera( PLAYER_CAM(nPlayerNum) );

				pCam = gamecam_GetActiveCamera();
				Player_aPlayer[nPlayerNum].m_pViewportPersp3D = (FViewport_t *)pCam->GetViewport();

				// Initialize the HUD for this player
				if( !Player_aPlayer[nPlayerNum].m_Hud.Create(nPlayerNum) ) {
					goto _ExitStartGameWithError;
				}

				// Setup player-specific elements in multiplayer manager
				MultiplayerMgr.SetupPlayer( nPlayerNum );
		}

//////////////////////////////////////////////////////////////
// put modules that require the player array to be setup here:
//////////////////////////////////////////////////////////////
	_UPDATE_LOADSCREEN
		gamecam_InitLevelCameras( 0, Player_aPlayer[0].m_nControllerIndex );// set player 0 to be the debuggable player
		
		bartersystem_InitLevel( Level_aInfo[Level_nLoadedIndex].pszCSVFile );
		
	_UPDATE_LOADSCREEN
		testbots_Init();

	_UPDATE_LOADSCREEN

		if( !CFDebris::InitDebrisSystem( _DEBRIS_COUNT ) ) {
			DEVPRINTF( "game::_PostWorldLoadGameInit(): Unable to init debris system.\n" );
			goto _ExitStartGameWithError;
		}
	_UPDATE_LOADSCREEN
		if( !fexplosion_InitExplosionSystem( 30, 20, sizeof( CDamageForm::Damager_t ) ) ) {
			DEVPRINTF( "game::_PostWorldLoadGameInit(): Unable to init explosion system.\n" );
			goto _ExitStartGameWithError;
		}

	_UPDATE_LOADSCREEN
//		if( !CCollectable::PostPlayerCreationSetup() ) {
//			DEVPRINTF( "game::_PostWorldLoadGameInit(): CCollectable::PostPlayerCreationSetup() failed.\n" );
//			goto _ExitStartGameWithError;
//		}

		letterbox_Reset();

		// set the gameloop handlers to be game_Work() and game_Draw()
		gameloop_SetLoopHandlers( game_Work, game_Draw, NULL );

		if( Level_aInfo[Level_nLoadedIndex].pFcnLoad ) {
			if( !Level_aInfo[Level_nLoadedIndex].pFcnLoad( LEVEL_EVENT_PRE_ENTITY_FIXUP ) ) {
				// Level loader function returned an error...
				goto _ExitStartGameWithError;
			}
		}
///////////////////////////////////////////////////////////////////////
// put modules that require that all CEntities have been created here:
///////////////////////////////////////////////////////////////////////
		
		floop_Reset();
		_UPDATE_LOADSCREEN
		CEntity::ResolveEntityPointerFixups();

		if( Level_aInfo[Level_nLoadedIndex].pFcnLoad ) {
			if( !Level_aInfo[Level_nLoadedIndex].pFcnLoad( LEVEL_EVENT_POST_ENTITY_FIXUP ) ) {
				// Level loader function returned an error...
				goto _ExitStartGameWithError;
			}
		}
	_UPDATE_LOADSCREEN
		CFXMeshBuildPartMgr::InitLevel();
	_UPDATE_LOADSCREEN
		checkpoint_LevelInit();
	_UPDATE_LOADSCREEN
		// This line needs to be one of the, if not the, last thing called in this function.
		CFScriptSystem::DoInitScripts();
	_UPDATE_LOADSCREEN
		
		game_SetupRenderTargets();

		#if SAS_ACTIVE_USER == SAS_USER_STEVE
			if( !user_steve_PostlevelInit() ) {
				goto _ExitStartGameWithError;
			}
		#endif

		CFVerlet::PrimeAll();

	_UPDATE_LOADSCREEN
		// NKM - This was moved here since I need to do this AFTER the script inits have been called
		if( !CCollectable::PostPlayerCreationSetup() ) {
			DEVPRINTF( "game::_PostWorldLoadGameInit(): CCollectable::PostPlayerCreationSetup() failed.\n" );
			goto _ExitStartGameWithError;
		}

	_UPDATE_LOADSCREEN
		// save initial checkpoint (must be after script init)
		checkpoint_Save( 0, FALSE );
	_UPDATE_LOADSCREEN
		#if FANG_ENABLE_DEV_FEATURES
			_fLoadTestTimer = 0.0f;	
		#endif	

		// determine what controller should be the debug port
		if( CPlayer::m_nPlayerCount == MAX_PLAYERS ) {
			// we are completely full with player, use the last controller port
			gamepad_SetDebugPortIndex( GAMEPAD_MAX_PORT_COUNT-1 );
			_nControllerMaskRequired = (1 << MAX_PLAYERS) - 1;
		} else {
			// find the unused port and use it
			_nControllerMaskRequired = 0;
			for( nPlayerNum=0; nPlayerNum < CPlayer::m_nPlayerCount; nPlayerNum++ ) {
				_nControllerMaskRequired |= (1 << Player_aPlayer[nPlayerNum].m_nControllerIndex);
			}
			// walk the bits on _nControllerMaskRequired and the first unused bit becomes the debug controller index
			for( nPlayerNum=0; nPlayerNum < MAX_PLAYERS; nPlayerNum++ ) {
				if( !( _nControllerMaskRequired & (1<<nPlayerNum) ) ) {
					// this is the one
					gamepad_SetDebugPortIndex( nPlayerNum );
					break;
				}
			}
		}
	_UPDATE_LOADSCREEN
	
	// Reset the cheat codes for each controller
	Gamepad_ResetCheatCodes();

	// record the game init pointer
	_pCurrentGameInit = pGameInit;

	// assume all controllers are plugged in right now
	_nControllerToPlugIn = _ALL_CONTROLLERS_PLUGGED_IN;	
	_nControllerPauseState = _CONTROLLER_PAUSE_STATE_OK;
	
	// setup the fade in
	_nFadeState = _FADE_STATE_IN;
	_fFade_Timer = 0.0f;
		
	// Tell the shadow system that we are in multiplayer, shadow LOD is handled slightly differently.
	fshadow_SetMultiplayer( (CPlayer::m_nPlayerCount > 1) );
	
	return TRUE;

// Error:
_ExitStartGameWithError:
	for (nPlayerNum = 0; nPlayerNum < CPlayer::m_nPlayerCount; nPlayerNum++) {
		if( Player_aPlayer[nPlayerNum].m_pEntityOrig ) {
			fdelete( Player_aPlayer[nPlayerNum].m_pEntityOrig );
			Player_aPlayer[nPlayerNum].m_pEntityOrig = NULL;
			Player_aPlayer[nPlayerNum].m_pEntityCurrent = NULL;
		}
		Player_aPlayer[nPlayerNum].m_Reticle.Destroy();
	}

	// These pools live in the frame released below; game_UnloadLevel() must not tear them down again.
	CFDebris::UninitDebrisSystem();
	fexplosion_UninitExplosionSystem();

	fres_ReleaseFrame( ResFrame );

	Game_pFullscreenRenderTarget = NULL;

	return FALSE;
}

void game_SetupRenderTargets() {
	// Create fullscreen render target...
#if FANG_PLATFORM_GC
	// Create a 16-bit render target...

	Game_pFullscreenRenderTarget = ftex_CreateRenderTarget_FullScreen( FTEX_RENDERTARGET_FMT_C16_D24_S8, "FSRT", TRUE, FRES_NULLHANDLE, NULL, 256, 512 );
	if( Game_pFullscreenRenderTarget ) {
		ftex_AddRenderTarget( Game_pFullscreenRenderTarget, _FullscreenRenderTargetCallback, FALSE, 0, FALSE, FALSE, NULL, TRUE, FALSE );
	}
#else
	// Create a 32-bit render target...

	Game_pFullscreenRenderTarget = ftex_CreateRenderTarget_FullScreen( FTEX_RENDERTARGET_FMT_C24_A8_D24_S8, "FSRT", TRUE, FRES_NULLHANDLE, NULL, 512, 256 );
	if( Game_pFullscreenRenderTarget ) {
		ftex_AddRenderTarget( Game_pFullscreenRenderTarget, _FullscreenRenderTargetCallback, FALSE, 0, FALSE, FALSE, NULL, TRUE, FALSE );
	}
#endif

	if (Game_pFullscreenRenderTarget) { 
		ftex_FlushRenderTarget(Game_pFullscreenRenderTarget); 
		ftex_ActivateRenderTarget(Game_pFullscreenRenderTarget, FALSE);
	}
	if( Game_pFullscreenRenderTarget == NULL ) {
		DEVPRINTF( "game::_PostWorldLoadGameInit(): Could not create a full-screen render target. Some effects and weapons will not operate.\n" );
	}
}

// will end a level, using the command keyword to 
// figure out what to do
// Keywords:
// 'NEXT' - default if nothing is passed in
// 'RESTART' - restarts the current level
// 'LOSE' - the player has lost the level - mainly used as a way for mini-games to be able to get notification of this event
void game_GotoLevel( cchar *pszCommandKeyword ) {

	if( pszCommandKeyword && fclib_stricmp( pszCommandKeyword, "lose" ) == 0 ) {
		if( Level_nLoadedIndex >= 0 ) {
			if( Level_aInfo[Level_nLoadedIndex].nLevel == LEVEL_HOLD_YOUR_GROUND ) {
				mg_holdyourground_PlayerLost();
				fforce_Reset();
				return;
			}
		}

		// all cases that don't handle the "lose" case will be defaulted to the "restart" case...pass through
	}

	//ME:  Taking this out.  Doesn't seem that the player ever gets control again
	//if( Level_aInfo[Level_nLoadedIndex].nLevel != LEVEL_RAT_RACE && Level_aInfo[Level_nLoadedIndex].nLevel != LEVEL_RACE_TO_THE_ROCKET ) {
	//	CPlayer::m_pCurrent->DisableEntityControl();
	//}


	if (CBot::m_bCutscenePlaying)
	{
		game_EndCutScene();
	}
	// not sure if these are really needed.
	CBot::DisableBotDamageGlobally();

	// Just in case, stomp on any vibrating controllers here
	fforce_Reset();

	if( !pszCommandKeyword || 
		fclib_stricmp( pszCommandKeyword, "next" ) == 0 ) {
		// move to the next level
		_bCompletedLevel = TRUE;
		_nFadeState = _FADE_STATE_OUT;
		_fFade_Timer = 0.0f;
		CPlayer::CacheAudioLevels();
				
	} else if( fclib_stricmp( pszCommandKeyword, "restart" ) == 0 ||
			   fclib_stricmp( pszCommandKeyword, "lose" ) == 0 ) {
		// restart the current level
		_bCompletedLevel = FALSE;
		checkpoint_Restore( 0, FALSE );
		//launcher_EndOfGameDecisions( LAUNCHER_DECISION_RESTART );
	} else {
		// default to moving to the next level
		DEVPRINTF( "game_GotoLevel() : unknown command string '%s', defaulting to 'NEXT'.\n", pszCommandKeyword );
		_bCompletedLevel = TRUE;
		_nFadeState = _FADE_STATE_OUT;
		_fFade_Timer = 0.0f;
		CPlayer::CacheAudioLevels();
	}	
}

// This function loads the ma csv file
// It contains global settings that need
// to be read...
BOOL _LoadGlobalSettings( void ) {

	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

	int i;			//CPS 4.7.03
	
	f32 *pfVal;

	// grab an fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the gamephrase csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _pszGlobalSettingsCSVFilename );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "game::_LoadGlobalSettings() : Could not load the global settings csv file '%s'.\n", _pszGlobalSettingsCSVFilename );
		goto _EXIT_WITH_ERROR;
	}

	/////////////////////////
	// find the global_settings table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszGlobalSettingsTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "game::_LoadGlobalSettingsTable() : Could not find the phrases table named '%s'.\n", _pszGlobalSettingsTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
//CPS 4.7.03	int i = fgamedata_GetNumFields( hTable );
	i = fgamedata_GetNumFields( hTable );			//CPS 4.7.03
	if( i != _GLOBALSETTINGS_COUNT ) {
		DEVPRINTF( "game::_LoadGlobalSettingsTable() : The global settings table in '%s' didn't contain exactly %d entries.\n", _pszGlobalSettingsTableName, _GLOBALSETTINGS_COUNT );
		goto _EXIT_WITH_ERROR;
	}

	//now, run through the global settings fields and apply them
	for( i=0; i < _GLOBALSETTINGS_COUNT; i++ ) {
		switch( i ) {
			case _GLOBALSETTINGS_POSTPOSSESSION_BOTPOWERDOWNTIME:
				pfVal = (f32 *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
				if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
				{
					DEVPRINTF("game::_LoadGlobalSettings() : Field %d in global settings table %s is not a float format!\n", i, _pszGlobalSettingsTableName);
					goto _EXIT_WITH_ERROR;
				}
				CBot::m_fPossessionTerminatedPowerDownTime = *pfVal;

			break;

			default:
				FASSERT_NOW;
		}
	}	

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_EXIT_WITH_ERROR:

	fmem_ReleaseFrame( hMemFrame );

	return FALSE;
}

// This function loads a phrases CSV table that
// will contain the games displayable text.  This text
// is wide-char localized.
BOOL _LoadPhraseTable( void ) {
	FMemFrame_t hMemFrame;
	FResFrame_t hResFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	int i, nStringsAdded = 0;
	cwchar *pwszText;
	
	// grab an fres and fmem frame
	hResFrame = fres_GetFrame();
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the gamephrase csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _pszGamePhrasesCSVFilename );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : Could not load the game phrases csv file '%s'.\n", _pszGamePhrasesCSVFilename );
		goto _EXIT_WITH_ERROR;
	}

	//////////////////////////////////
	// find the 'common_phrases' table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszGamePhrasesTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : Could not find the phrases table named '%s'.\n", _pszGamePhrasesTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTable );
	if( i < GAMEPHRASE_NUM_COMMON_STRINGS ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : The phrases table in '%s' has %d strings; %d are required.\n", _pszGamePhrasesTableName, i, GAMEPHRASE_NUM_COMMON_STRINGS );
		goto _EXIT_WITH_ERROR;
	}
	if( i > GAMEPHRASE_NUM_COMMON_STRINGS ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : Ignoring %d trailing phrases in newer table '%s'.\n", i - GAMEPHRASE_NUM_COMMON_STRINGS, _pszGamePhrasesTableName );
	}
	// walk the string table adding each string to the string table and recording a ptr to them
	for( i=0; i < GAMEPHRASE_NUM_COMMON_STRINGS; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if( nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING ) {
			DEVPRINTF("game::_LoadPhrasesTable() : Field %d in phrases table %s is not a WIDESTRING format!\n", i, _pszGamePhrasesTableName );
			goto _EXIT_WITH_ERROR;
		}
		Game_apwszPhrases[nStringsAdded++] = gstring_Main.AddString( pwszText );
	}

	///////////////////////////////////////////
	// find the platform specific phrase table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszPlatformPhrasesTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : Could not find the phrases table named '%s'.\n", _pszPlatformPhrasesTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTable );
	if( i < GAMEPHRASE_NUM_PLATFORM_STRINGS ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : The phrases table in '%s' has %d strings; %d are required.\n", _pszPlatformPhrasesTableName, i, GAMEPHRASE_NUM_PLATFORM_STRINGS );
		goto _EXIT_WITH_ERROR;
	}
	if( i > GAMEPHRASE_NUM_PLATFORM_STRINGS ) {
		DEVPRINTF( "game::_LoadPhrasesTable() : Ignoring %d trailing phrases in newer table '%s'.\n", i - GAMEPHRASE_NUM_PLATFORM_STRINGS, _pszPlatformPhrasesTableName );
	}
	// walk the string table adding each string to the string table and recording a ptr to them
	for( i=0; i < GAMEPHRASE_NUM_PLATFORM_STRINGS; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if( nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING ) {
			DEVPRINTF("game::_LoadPhrasesTable() : Field %d in phrases table %s is not a WIDESTRING format!\n", i, _pszPlatformPhrasesTableName );
			goto _EXIT_WITH_ERROR;
		}
		Game_apwszPhrases[nStringsAdded++] = gstring_Main.AddString( pwszText );
	}

	FASSERT( nStringsAdded == GAMEPHRASE_COUNT );

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_EXIT_WITH_ERROR:

	fres_ReleaseFrame( hResFrame );
	fmem_ReleaseFrame( hMemFrame );

	return FALSE;
}


static wchar _wszString[64];
BOOL _ControllerWaitForReconnect( void );

BOOL _ControllerWaitForStart( void ) {
	if( !(Gamepad_nPortOnlineMask & (1<<_nControllerToPlugIn)) ) {	
		CMsgBox::Clear();
		_snwprintf( _wszString, 64, Game_apwszPhrases[GAMEPHRASE_LOST_CONTROLLER_FORMATSTRING], '\n', '\n', _nControllerToPlugIn+1 );
		CMsgBox::Display( "CtlReconnect", NULL, _wszString, NULL, NULL, NULL, 0, TRUE, _ControllerWaitForReconnect );

	} else if( Gamepad_aapSample[_nControllerToPlugIn][GAMEPAD_MENU_START]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		CMsgBox::Clear();
		_nControllerToPlugIn = _ALL_CONTROLLERS_PLUGGED_IN;
		_nControllerPauseState = _CONTROLLER_PAUSE_STATE_OK;
	}


	return TRUE;
}

BOOL _ControllerWaitForReconnect( void ) {

	if( Gamepad_nPortOnlineMask & (1<<_nControllerToPlugIn) ) {
		_nControllerPauseState = _CONTROLLER_PAUSE_STATE_WAIT_FOR_PRESS;
		
		CMsgBox::Clear();

		_snwprintf( _wszString, 64, Game_apwszPhrases[GAMEPHRASE_PRESS_START_TO_CONTINUE], '\n' );
		CMsgBox::Display( "CtlPressStart", NULL, _wszString, NULL, NULL, NULL, 0, TRUE, _ControllerWaitForStart );
	}

	return TRUE;
}


void game_HandleControllersBeingUnplugged( void ) {
	s32 i;

	// make sure all player controllers are connected
	if( _nControllerToPlugIn == _ALL_CONTROLLERS_PLUGGED_IN ) {
		// see if all controllers are still plugged in
		if( (Gamepad_nPortOnlineMask & _nControllerMaskRequired) != _nControllerMaskRequired ) {
			// a needed controller has been unplugged, which one?
			for( i=0; i < CPlayer::m_nPlayerCount; i++ ) {
				if( ((1<<Player_aPlayer[i].m_nControllerIndex) & Gamepad_nPortOnlineMask) == 0 ) {
					// not plugged in
					_nControllerToPlugIn = Player_aPlayer[i].m_nControllerIndex;
					_nControllerPauseState = _CONTROLLER_PAUSE_STATE_WAIT_FOR_INSERT;
					
					_snwprintf( _wszString, 64, Game_apwszPhrases[GAMEPHRASE_LOST_CONTROLLER_FORMATSTRING], '\n', '\n', _nControllerToPlugIn+1 );
					CMsgBox::Display( "CtlReconnect", NULL, _wszString, NULL, NULL, NULL, 0, TRUE, _ControllerWaitForReconnect );
					break;
				}
			}
		}
	}
}


void game_DrawSolidFullScreenOverlay( f32 fAlpha, f32 fIntensity ) {
	CFColorRGBA ColorRGBA;
	CFVec3 Vtx1, Vtx2, Vtx3, Vtx4;

	ColorRGBA.Set( fIntensity, fAlpha );

	FViewport_t *pPreviousVP = fviewport_GetActive();
	fviewport_SetActive( FViewport_pDefaultOrtho );
	FXfm_Identity.InitStackWithView();
	frenderer_Push( FRENDERER_DRAW, NULL );

	Vtx1.Set( 0.0f, FViewport_pDefaultOrtho->Res.y, 1.0f );
	Vtx2.Set( 0.0f, 0.0f, 1.0f );
	Vtx3.Set( FViewport_pDefaultOrtho->Res.x, 0.0f, 1.0f );
	Vtx4.Set( FViewport_pDefaultOrtho->Res.x, FViewport_pDefaultOrtho->Res.y, 1.0f );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_DST );
	fdraw_SetTexture( NULL );

	fdraw_SolidQuad( &Vtx1, &Vtx2, &Vtx3, &Vtx4, &ColorRGBA );

	frenderer_Pop();
	fviewport_SetActive( pPreviousVP );
}

void game_SetListenerOrientationCallback( ListenerOrientationCallback_t *pCallback ) {
	_pListenerCallback = pCallback;
}

ListenerOrientationCallback_t *game_GetListenerOrientationCallback( void ) {
	return _pListenerCallback;
}


#if _USE_FDRAW_IN_SCENE_RENDER
void _RenderSortCallbackPrePointSprites( CFMtx43A *pRenderCamMtx, FViewport_t *pRenderViewport ) {
	// Local copy of player number to help the compiler...
	s32 nPlayerIndex = CPlayer::m_nCurrent;
	
	frenderer_Push(FRENDERER_DRAW, NULL);

	fdraw_ClearDepth_Cache();

	tracer_Draw();
	CEZipLine::DrawAll();
	CFDecal::Draw();
	CFXStreamerEmitter::Draw();

	// we should really have bot::Draw function that could call this, but...
	if (Player_aPlayer[nPlayerIndex].m_pEntityCurrent &&
		Player_aPlayer[nPlayerIndex].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT &&
		((CBot *)Player_aPlayer[nPlayerIndex].m_pEntityCurrent)->GetPowerupFx())
	{
		((CBot *)Player_aPlayer[nPlayerIndex].m_pEntityCurrent)->GetPowerupFx()->Draw();
	}

	//CFXShockwave::Draw();
	CFlamer::DrawAll();
	//CTether::DrawAll();
	CWeaponRecruiter::DrawAll();
	CBotScientist::DrawAllFX();
	CFXMagmaBomb::DrawAll();
	CBotScout::DrawBeams();
	BotEliteGuard_DrawBeams();
	fexplosion_Draw( gamecam_GetActiveCamera() );
	CLaserBeam::Draw();
	CMuzzleFlash::Draw();
//	CBlinkShell::FDraw();
	CLightningBoltGroup::FDraw();
	CBlinkSpeed::FDraw();
	CBlinkGlow::FDraw();
	smoketrail_PostDrawAll();
	
	frenderer_Pop();
}
#endif

// Call this as the first set of localized resources to load.
// This is just to get bare-bones localized display capabilities
// to the user asap.
BOOL game_LoadLocalizedPhraseTableAndFonts( void ) {

	// Grab memory frames...
	FResFrame_t hResFrame = fres_GetFrame();

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( "Loading Localized GamePhrases and Fonts" );
#endif
	DEVPRINTF( "******** LOAD MARKER - START OF LOCALIZED GAMEPHRASES AND FONTS***********.\n" );

	// The very first thing to do is load the language specific phrase table
	if( !_LoadPhraseTable() ) {
		goto _ExitWithError;
	}

	if( FTEXT_ERROR == ftext_Load( '1', "tfharialw$" ) ) {
		DEVPRINTF( "Could not load the \"tfharialw$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
	//////////////////
	// 2 is available
	/////////////////
	if( FTEXT_ERROR == ftext_Load( '3', "tfhtych26$" ) ) {
		DEVPRINTF( "Could not load the \"tfhtych26$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
	if( FTEXT_ERROR == ftext_Load( '4', "tfhammo10$" ) ) {
		DEVPRINTF( "Could not load the \"tfhammo10$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
	// 5 is available

	if( FTEXT_ERROR == ftext_Load( '6', "tfhammo26$" ) ) {
		DEVPRINTF( "Could not load the \"tfhammo26$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
	//////////////////
	// 7 is available
	//////////////////
#if FANG_PLATFORM_GC
	if( FTEXT_ERROR == ftext_Load( '8', "tfhari10$" ) ) {
		DEVPRINTF( "Could not load the \"tfhari10$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
#else
	if( FTEXT_ERROR == ftext_Load( '8', "tfhver10$" ) ) {
		DEVPRINTF( "Could not load the \"tfhver10$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
#endif

#if FANG_PLATFORM_GC
	if( FTEXT_ERROR == ftext_Load( '9', "TFHmsgGC$" ) ) {
		DEVPRINTF( "Could not load the \"TFHmsgGC$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
#else

	if( FTEXT_ERROR == ftext_Load( '9', "TFHmsgXB$" ) ) {
		DEVPRINTF( "Could not load the \"TFHmsgXB$\" font.", "game_LoadFonts: Text System Problem" );
		goto _ExitWithError;
	}
#endif

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END OF LOCALIZED GAMEPHRASES AND FONTS***********.\n" );

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( hResFrame );

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END OF LOCALIZED GAMEPHRASES AND FONTS***********.\n" );

	return FALSE;
}


// This routine will Init/load the rest of the localized resources
// and systems required for gameplay
BOOL game_InitLocalizedResources( void ) {

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStart( "Loading Rest of Localized Resources" );
#endif
	DEVPRINTF( "******** LOAD MARKER - START OF REST OF LOCALIZED RESOURCES***********.\n" );

	// Grab memory frames...
	FResFrame_t hResFrame = fres_GetFrame();

	/////////////////////////
	// Init the audio system:
	FAudio_Init_t oAudioInit;
	fang_MemZero( &oAudioInit, sizeof( oAudioInit ) );

	oAudioInit.uMaxListeners      = MAX_PLAYERS;
	oAudioInit.uMaxEmitters       = 80;
	oAudioInit.uMaxStreams        = 2;
	oAudioInit.uMaxBanks          = 24;
	oAudioInit.uMaxPriorityLevels = 1;
	oAudioInit.uPriorityLimits    = 0;
	oAudioInit.paoPriorityLimits  = NULL;
	#if FANG_PLATFORM_WIN
	oAudioInit.DX8ONLY_ohWnd      = (u32)fdx8vid_GetWindowHandle();
	oAudioInit.DX8ONLY_ohInstance = (u32)FVid_Win.hInstance;
	#endif

	if( Gameloop_bInstallAudio ) {
		if( FAUDIO_ERROR == faudio_Install( &oAudioInit ) ) {
			DEVPRINTF( "Could not install audio driver --- proceeding without audio support.\n" );
		//	goto _ExitGameInitWithError;
		}
		#if FANG_PLATFORM_XB
		faudio_UsingHeadphones_XB( FALSE );
		#endif
	}

	//init the movie system...
	fmovie2_Install();

	if( Gameloop_bInstallAudio ) {
		fresload_Load( FSNDFX_RESTYPE, "Damage" );
		fresload_Load( FSNDFX_RESTYPE, "weapons" );
		fresload_Load( FSNDFX_RESTYPE, "Glitch" );
	}

	CFDebrisGroup::LoadAllFromGameData( _DEBRIS_GROUP_FILENAME, _DEBRIS_MESH_SET_FILENAME );

	if( !CFDecal::LoadData() ) {
		DEVPRINTF( "game_InitSystem(): Unable to load decal data.\n" );
		return FALSE;
	}
	
	if( !fexplosion_LoadExplosionData( _EXPLOSION_SET_FILENAME, _EXPLOSION_GROUPS_FILENAME ) ) {
		DEVPRINTF( "fexplosion_LoadExplosionData(): Failed\n" );
		return FALSE;
	}


	// Now initialize any systems which may require localized sounds or the phrase table
	if( !gameloop_InitLocalizedGameSystems() ) {
		DEVPRINTF( "Could not initialize one of the localized game systems\n" );
		goto _ExitWithError;
	}

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END OF REST OF LOCALIZED RESOURCES***********.\n" );

	return TRUE;

_ExitWithError:

#if !GAMELOOP_EXTERNAL_DEMO
	ffile_LogStop();
#endif
	DEVPRINTF( "******** LOAD MARKER - END OF REST OF LOCALIZED RESOURCES***********.\n" );

	fres_ReleaseFrame( hResFrame );
	return FALSE;
}

// This routine mainly exists for the Gamecube, to handle the fact that they were
// Beevises (That would be plurar Beevis) and allow the dvd drive door to be opened during gameplay.
// Because of this, we need to gracefully handle this situation and display an error message
static void _FFileAsyncReadErrorHaltCallback( FFileAsyncReadErrorCode_e eErrorCode, BOOL bHandleSwap ) {

	if(	_eAsyncHaltError == FFILE_ASYNC_READ_ERROR_NONE ) {
		// We were running, now we are halted, so lets shut down some stuff
		// First, pause the sounds
		_PreAsyncErrorHaltStreamPauseLevel = CFAudioStream::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_5 ); 
		_PreAsyncErrorHaltEmitterPauseLevel = CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_5 ); 

		if( fmovie2_GetStatus() == FMOVIE2_STATUS_PLAYING ) {
			fmovie2_Pause( TRUE );
		}

		_bIssueBeginOnAsyncErrorResume = FALSE;
	}

	// This is probably the wrong place for this, but I'll put it here anyway...
	faudio_Work();
	// Display an appropriate message here....

	// Check to see if our current Halt error is different than our previous error...
	if( _eAsyncHaltError != eErrorCode ) {
		BOOL bUseNoFontBox = FALSE;
		// We have a new error message to display
		if( ( _eAsyncHaltError != FFILE_ASYNC_READ_ERROR_NONE ) && CMsgBox::IsInitialized() ) {
			// We were displaying a previous error message, so clear
			// that one in anticipation of displaying a new one...
			CMsgBox::Clear();
		}

		// Now, display the new error message
		switch ( eErrorCode ) {
			case FFILE_ASYNC_READ_ERROR_DVD_COVER_OPEN:
				if( Game_apwszPhrases[ GAMEPHRASE_CLOSE_DVD_COVER ] ) {
					_pwszAsyncErrorHaltDisplayString = Game_apwszPhrases[ GAMEPHRASE_CLOSE_DVD_COVER ];
				} else {
					_pwszAsyncErrorHaltDisplayString = L"The Disk Cover is open.\nIf you want to continue the game,\nplease close the Disc Cover.";
					bUseNoFontBox = TRUE;
				} 
				break;
			case FFILE_ASYNC_READ_ERROR_DVD_NO_DISK:
			case FFILE_ASYNC_READ_ERROR_DVD_WRONG_DISK:
				if( Game_apwszPhrases[ GAMEPHRASE_INSERT_GAME_DISK ] ) {
					_pwszAsyncErrorHaltDisplayString = Game_apwszPhrases[ GAMEPHRASE_INSERT_GAME_DISK ];
				} else {
					_pwszAsyncErrorHaltDisplayString = L"Please Insert the\nMetal Arms Game Disk";
					bUseNoFontBox = TRUE;
				} 
				break;
			case FFILE_ASYNC_READ_ERROR_DVD_RETRY:
				if( Game_apwszPhrases[ GAMEPHRASE_CLEAN_DISK ] ) {
					_pwszAsyncErrorHaltDisplayString = Game_apwszPhrases[ GAMEPHRASE_CLEAN_DISK ];
				} else {
					_pwszAsyncErrorHaltDisplayString = L"The game disk could not be read.\nPlease read the Nintendo\nGameCube Instruction booklet\nfor more information.";
					bUseNoFontBox = TRUE;
				} 
				break;
			case FFILE_ASYNC_READ_ERROR_DVD_FATAL_ERROR:
				if( Game_apwszPhrases[ GAMEPHRASE_FATAL_ERROR ] ) {
					_pwszAsyncErrorHaltDisplayString = Game_apwszPhrases[ GAMEPHRASE_FATAL_ERROR ];
				} else {
					_pwszAsyncErrorHaltDisplayString = L"An Error has occured.\nTurn the power off and refer to\nthe nNintendo Gamecube Instruction\nBooklet for further instructions.";
					bUseNoFontBox = TRUE;
				} 
				gameloop_DisablePlatformReset();
				break;
			case FFILE_ASYNC_READ_ERROR_NONE:
			default:
				FASSERT_NOW;
		}

		// now, display the error message if 
		if( CMsgBox::IsInitialized() ) {

			// Figure out how many newlines there are in our string.  Then use the appropriate Messagebox
			cwchar *pwszString = _pwszAsyncErrorHaltDisplayString;
			u32 nNumNewLines = 0;
			while( pwszString ) {
				pwszString = fclib_wcschr( pwszString, L'\n' );
				if( pwszString ) {
					nNumNewLines++;
					pwszString++; // Increment PAST the newline
				}
			}

			FMATH_CLAMPMAX( nNumNewLines, 4 );
			char szMessageBoxName[16];
			if( bUseNoFontBox ) {
				sprintf( szMessageBoxName, "DVDErrorNoFont%d", nNumNewLines );			 
			} else {
				sprintf( szMessageBoxName, "DVDError%d", nNumNewLines );			 
			}
			CMsgBox::Display( szMessageBoxName, NULL, _pwszAsyncErrorHaltDisplayString, NULL, NULL, NULL, NULL, TRUE, NULL, !bHandleSwap );
		}
	}

	// Handle the reset button work here!
	if( eErrorCode != FFILE_ASYNC_READ_ERROR_DVD_FATAL_ERROR ) {
		// Allow a reset only if this is NOT a fatal Error.
		gameloop_CheckForPlatformReset( TRUE ); // Reset immediately
	}

	if( bHandleSwap && FVid_bBegin ) {
		// Finish drawing whatever we were drawing
		fvid_End();
		_bIssueBeginOnAsyncErrorResume = TRUE;
	}


	if( bHandleSwap ) {
		fvid_Swap();
		fvid_Begin(); // Begin the frame...
		fviewport_SetActive( NULL ); // Clear the viewport for sure
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0, 0, 0, 1.0f, 0 );
	
		// Draw the message box here
		if( CMsgBox::IsInitialized() ) {
			CMsgBox::DrawAll();		
		} else {
			// Draw the text using the ftext interface
			ftext_Printf( 0.5f, 0.3f, L"~aC~C99999999~s1.00~b0%ls", _pwszAsyncErrorHaltDisplayString );
			ftext_Draw();
		}
	
		fvid_End();
	}
	_eAsyncHaltError = eErrorCode;
}


// This routine will resume game systems that need to be restarted since the async read error is clear
static void _FFileAsyncReadErrorResumeCallback( FFileAsyncReadErrorCode_e eErrorCode, BOOL bHandleSwap ) {

	FASSERT( _eAsyncHaltError != FFILE_ASYNC_READ_ERROR_NONE );
	FASSERT( eErrorCode == FFILE_ASYNC_READ_ERROR_NONE );

	CMsgBox::Clear();

	// First, restore the sound pause levels to what they were prior to
	// the Async error callback
	CFAudioStream::SetGlobalPauseLevel( _PreAsyncErrorHaltStreamPauseLevel ); 
	CFAudioEmitter::SetGlobalPauseLevel( _PreAsyncErrorHaltEmitterPauseLevel ); 

	// Resume any movies, if they were playing
	if( fmovie2_GetStatus() == FMOVIE2_STATUS_PAUSED ) {
		fmovie2_Pause( FALSE );
	}

	faudio_Work();

	// Check to see if we need to issue a begin before we exit the loop
	// This may be necessary if we got paused in the middle of the draw loop 
	if( _bIssueBeginOnAsyncErrorResume ) {
		fvid_Begin();
	}	
	_eAsyncHaltError = FFILE_ASYNC_READ_ERROR_NONE; // Clear the halt error
}
