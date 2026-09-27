//////////////////////////////////////////////////////////////////////////////////////
// game.h - Main gameplay mode.
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
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GAME_H_
#define _GAME_H_ 1

#include "fang.h"
#include "player.h"
#include "gamesave.h"

////////////
// defines:
#define NUM_WCHARS_TAKE_OFF_THE_STACK (10*16)	//	10 * 16 = coexists well with aligned data, 
												//	160 chars is more than twice the size of my longest sentence 
												//	costs 320 bytes,  (stress to the stack?, could use smaller...)
												//  put in game.h because I'm using it w/ gamephrases, in more than one place
												//  fang is configured with max string buffer size of 160 at 
												//  Fang_ConfigDefs.nText_MaxCharsPerPrintf = 160;
												//	I'd use this value, if it were compile-time constant.
												//  MRS


enum ControlMode_e {
	CONTROLMODE_NORMAL = 0,
	CONTROLMODE_PAUSESCREEN,
	CONTROLMODE_MULTIPLAYER_OPTIONS,
	CONTROLMODE_WEAPONSELECT,
	CONTROLMODE_LETTERBOX,
	CONTROLMODE_LETTERBOXFF,
	CONTROLMODE_DEBUGCAM,
	CONTROLMODE_NONAMBIENTTALK,
	CONTROLMODE_BARTERSYSTEM,
	CONTROLMODE_MINIGAME,

	CONTROLMODE_COUNT
};

// There must be a one-to-one phrase correspondence between this enumeration table
// and the phrases defined in the gamephrase.csv file!
enum GamePhrase_e {
	GAMEPHRASE_LOSING_SIGNAL = 0,
	GAMEPHRASE_SIGNAL_LOST,
	GAMEPHRASE_PRESS_Y_TO_FLIP_VEHICLE_OVER,
	GAMEPHRASE_PRESS_Y_TO_DRIVE_VEHICLE,
	GAMEPHRASE_PRESS_Y_TO_OPERATE_GUN,
	GAMEPHRASE_VEHICLE_IS_LOCKED,
	GAMEPHRASE_LAUNCH_CLEANER,
	GAMEPHRASE_PRESS_Y_TO_RECRUIT_BUDDY,
	GAMEPHRASE_PRESS_Y_TO_DISCHARGE_BUDDY,
	GAMEPHRASE_CHIP_REQUIRED,
	GAMEPHRASE_MIL_ONLY,
	GAMEPHRASE_PRESS_Y_TO_USE_SWITCH,
	GAMEPHRASE_PRESS_Y_TO_USE_DET_PACK,
	GAMEPHRASE_DETPACK_REQUIRED,
	GAMEPHRASE_SELECT_TARGETS,
	GAMEPHRASE_INCOMING_TRANSMISSION,
	GAMEPHRASE_COLONEL_ALLOY,
	GAMEPHRASE_AGENT_SHHH,
	GAMEPHRASE_KRUNK,
	GAMEPHRASE_AGENT_GOFF,
	GAMEPHRASE_DR_EXAVOLT,
	GAMEPHRASE_HOLD_TO_EXIT_BOT,
	GAMEPHRASE_EUK_LEVEL,
	GAMEPHRASE_PRIMARY,
	GAMEPHRASE_SECONDARY,
	GAMEPHRASE_WEAPON,
	GAMEPHRASE_DUAL,
	GAMEPHRASE_UPGRADABLE,
	GAMEPHRASE_BACK,
	GAMEPHRASE_RESUME_GAME,
	GAMEPHRASE_ACCEPT,
	GAMEPHRASE_CANCEL,
	GAMEPHRASE_RESTART_LEVEL,
	GAMEPHRASE_RESPAWN,
	GAMEPHRASE_SOUND_OPTIONS,
	GAMEPHRASE_EDIT_CONTROLLER,
	GAMEPHRASE_ADVANCED_CONTROLLER_SETUP,
	GAMEPHRASE_QUIT_GAME,
	GAMEPHRASE_MISSION_OBJECTIVE,
	GAMEPHRASE_PRIMARY_EQUIPMENT,
	GAMEPHRASE_SECONDARY_EQUIPMENT,
	GAMEPHRASE_INFORMATION,
	GAMEPHRASE_OPTIONS,
	GAMEPHRASE_YES,
	GAMEPHRASE_NO,
	GAMEPHRASE_UNINITIALIZED,
	GAMEPHRASE_CHECKPOINT_SAVED,
	GAMEPHRASE_RESTORED_TO_CHECKPOINT,
	GAMEPHRASE_TARGET_UNKNOWN,
	GAMEPHRASE_TARGET,
	GAMEPHRASE_GRUNT,
	GAMEPHRASE_GUARD,
	GAMEPHRASE_TROOPER,
	GAMEPHRASE_TITAN,
	GAMEPHRASE_SNIPER,
	GAMEPHRASE_SWARMER,
	GAMEPHRASE_PREDATOR,
	GAMEPHRASE_SENTRY,
	GAMEPHRASE_MORTAR,
	GAMEPHRASE_CORROSIVE,
	GAMEPHRASE_HEIGHT,
	GAMEPHRASE_HEIGHT_UNKNOWN,
	GAMEPHRASE_SPEED,
	GAMEPHRASE_SPEED_UNKNOWN,
	GAMEPHRASE_HEALTH,
	GAMEPHRASE_HEALTH_UNKNOWN,
	GAMEPHRASE_D_PORT_OPEN,
	GAMEPHRASE_D_PORT_CLOSED,
	GAMEPHRASE_D_PORT_NONE,
	GAMEPHRASE_RANGE,
	GAMEPHRASE_RANGE_UNKNOWN,
	GAMEPHRASE_SAVING,
	GAMEPHRASE_LOADING,
	GAMEPHRASE_BAD_GAME_DISK,
	GAMEPHRASE_WARNING,
	GAMEPHRASE_GAME_WONT_BE_SAVED,
	GAMEPHRASE_RETICLE_NO_AMMO,
	GAMEPHRASE_RETICLE_LOW_AMMO,
	GAMEPHRASE_RETICLE_RELOAD,
	GAMEPHRASE_SCOPE_INCOMPATIBLE,
	GAMEPHRASE_YOU_ARE_FREE,

	GAMEPHRASE_MULTIPLAYER_NAME_1,
	GAMEPHRASE_MULTIPLAYER_NAME_2,
	GAMEPHRASE_MULTIPLAYER_NAME_3,
	GAMEPHRASE_MULTIPLAYER_NAME_4,
	GAMEPHRASE_MULTIPLAYER_NAME_5,
	GAMEPHRASE_MULTIPLAYER_NAME_6,
	GAMEPHRASE_MULTIPLAYER_NAME_7,
	GAMEPHRASE_MULTIPLAYER_NAME_8,
	GAMEPHRASE_MULTIPLAYER_NAME_9,
	GAMEPHRASE_MULTIPLAYER_NAME_10,
	GAMEPHRASE_TIME_REMAINING,
	GAMEPHRASE_PLAYER_WINS,
	GAMEPHRASE_PLAYER_QUIT,
	GAMEPHRASE_RESUME_BUTTON,
	GAMEPHRASE_QUIT_BUTTON,
	GAMEPHRASE_RANKING,
	GAMEPHRASE_PRESS_A_TO_CONTINUE,
	GAMEPHRASE_ENEMIES_DESTROYED,
	GAMEPHRASE_WASHERS_COLLECTED,
	GAMEPHRASE_TIME_TO_COMPETE,
	GAMEPHRASE_LEVEL_COMPLETE,
	GAMEPHRASE_PRESS_START_TO_CONTINUE,
	GAMEPHRASE_PLAYER_TIME,
	GAMEPHRASE_YOU_ARE_IT,
	GAMEPHRASE_TIME_FORMAT,
	GAMEPHRASE_HDR_PLAYER,
	GAMEPHRASE_HDR_KILLS,
	GAMEPHRASE_HDR_DEATHS,
	GAMEPHRASE_HDR_TIME,
	GAMEPHRASE_TEAMS_TIED,
	GAMEPHRASE_YOU_WON,
	GAMEPHRASE_SECOND_PLACE,
	GAMEPHRASE_THIRD_PLACE,
	GAMEPHRASE_YOU_LOST,
	GAMEPHRASE_TIED_FOR_FIRST,
	GAMEPHRASE_TIED_FOR_SECOND,
	GAMEPHRASE_TIED_FOR_LAST,
	GAMEPHRASE_YOUR_TEAM_WON,
	GAMEPHRASE_YOUR_TEAM_LOST,
	GAMEPHRASE_SUICIDE,
	GAMEPHRASE_TIME_BONUS,
	GAMEPHRASE_NO_IT_PLAYER,
	GAMEPHRASE_KILLING_SPREE_1,
	GAMEPHRASE_KILLING_SPREE_2,

	// Team phrases. Red phrase must always immediately follow blue phrase
	GAMEPHRASE_DM_BLU_AHEAD,
	GAMEPHRASE_DM_RED_AHEAD,
	GAMEPHRASE_TIME_BLU_AHEAD,
	GAMEPHRASE_TIME_RED_AHEAD,
	GAMEPHRASE_BLU_TEAM_WINS,
	GAMEPHRASE_RED_TEAM_WINS,

	// Kill messages. Must be contiguous
	GAMEPHRASE_KILL_1,
	GAMEPHRASE_KILL_2,
	GAMEPHRASE_KILL_3,

	// Killed messages. Must be contiguous
	GAMEPHRASE_KILLED_1,
	GAMEPHRASE_KILLED_2,

	GAMEPHRASE_MISSION_FAILED,

	// Collectable HUD messages.  Repurposed for barters, 5/31 MRS
	GAMEPHRASE_PRESS_Y_TO_SHOP,
	GAMEPHRASE_YOU_GOT_A,
	GAMEPHRASE_UPGRADE,
	GAMEPHRASE_N_ROUNDS_OF,
	GAMEPHRASE_AMMO,
	GAMEPHRASE_N_ROCKETS,
	GAMEPHRASE_N_CORING_CHARGES,
	GAMEPHRASE_N_CLEANERS,
	GAMEPHRASE_PURCHASE,
	GAMEPHRASE_NO_THANKS,
	GAMEPHRASE_LEVEL_1,
	GAMEPHRASE_LEVEL_2,
	GAMEPHRASE_LEVEL_3,
	GAMEPHRASE_BATTERY,
	GAMEPHRASE_ARM_SERVO,
	GAMEPHRASE_ENERGY,
	GAMEPHRASE_MEGAENERGY,
	// possession console phrases
	GAMEPHRASE_ONE_CHIP_REQUIRED,
	GAMEPHRASE_CHIPS_REQUIRED,
	GAMEPHRASE_NO_CHIPS_TO_INSERT,
	GAMEPHRASE_PRESS_Y_TO_INSERT_CHIP,
	GAMEPHRASE_PRESS_Y_TO_USE_CONSOLE,
	GAMEPHRASE_CONSOLE_OUT_OF_ORDER,

	GAMEPHRASE_SELECTNEWLOC,
	GAMEPHRASE_ERRORSAVING,
	GAMEPHRASE_SELECTSAVELOCATION,
	GAMEPHRASE_PROFILEFOUND,
	GAMEPHRASE_OVERWRITE,
	GAMEPHRASE_SAVINGPROFILE,
	GAMEPHRASE_NOPROGRESS,

	GAMEPHRASE_INSERT_CONTROLLER,

// ADD NEW COMMON STRINGS HERE...


	////////////////////////////////////////
	// MUST BE THE LAST COMMON STRING ENUM
	GAMEPHRASE_NUM_COMMON_STRINGS,
	////////////////////////////////////////

		/////////////////////////////////////////////////
		// STRINGS FROM THE PLATFORM SPECIFIC CSV TABLE
		GAMEPHRASE_LOST_CONTROLLER_FORMATSTRING = GAMEPHRASE_NUM_COMMON_STRINGS, //a wide character formatted string requiring 1 additional integer parameter: the port number of the disconnected controller.
		
		GAMEPHRASE_CLOSE_DVD_COVER,
		GAMEPHRASE_INSERT_GAME_DISK,
		GAMEPHRASE_CLEAN_DISK,
		GAMEPHRASE_FATAL_ERROR,
		GAMEPHRASE_MU_REMOVED,
		GAMEPHRASE_CARDNEEDSFORMATTING,
		GAMEPHRASE_CONFIRMFORMAT,
		GAMEPHRASE_MUSELECT_CARDNEEDSFORMATTING,
		GAMEPHRASE_SELECTMU,
		GAMEPHRASE_DONTTOUCHMU,
	
// ADD NEW PLATFORM STRINGS HERE...

		/////////////////////////////////////////////
		// MUST BE THE LAST PLATFORM SPECIFIC STRING
		GAMEPHRASE_NEXT_PLATFORM_STRING,
		GAMEPHRASE_NUM_PLATFORM_STRINGS = (GAMEPHRASE_NEXT_PLATFORM_STRING - GAMEPHRASE_NUM_COMMON_STRINGS),
		/////////////////////////////////////////////////

	// Count must be last real phrase number
	GAMEPHRASE_COUNT = (GAMEPHRASE_NUM_COMMON_STRINGS + GAMEPHRASE_NUM_PLATFORM_STRINGS),

	// Derived phrase numbers. Make sure these stay accurate
	GAMEPHRASE_KILL_LAST = GAMEPHRASE_KILL_3,
	GAMEPHRASE_KILLED_LAST = GAMEPHRASE_KILLED_2,
	GAMEPHRASE_MULTIPLAYER_NAME_LAST = 	GAMEPHRASE_MULTIPLAYER_NAME_10
};

typedef enum {
	GAME_TYPES_DEVELOPMENT = 0,
	GAME_TYPES_ADVENTURE,
	GAME_TYPES_MULTIPLAYER,
	GAME_TYPES_REPLAY_LEVEL,

	GAME_TYPES_COUNT
} GameTypes_e;

// These types must maintain their order, because the wrappers count on it.
typedef enum {
	GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH = 0,
	GAME_MULTIPLAYER_BASE_TYPES_KING_OF_THE_HILL,
	GAME_MULTIPLAYER_BASE_TYPES_TAG,
	GAME_MULTIPLAYER_BASE_TYPES_REVERSE_TAG,

	GAME_MULTIPLAYER_BASE_TYPES_COUNT,

} Game_Multiplayer_Game_Types_e;

struct GameInitInfo_t {
	// vars used for both single and multiplayer modes
	CPlayerProfile *apProfile[MAX_PLAYERS];			// what profile is each player using (packed, only the first nNumPlayers elements are used)
	u8 nNumPlayers;									// how many human players are playing (up to MAX_PLAYERS), 1 in single player mode
	BOOL8 bSinglePlayer;							// if FALSE, then multiplayer, otherwise single player mode
	u8 nLevelToPlay;								// what level number should we play
	cwchar *pwszLevelDisplayHeading;				// what do you want displayed on the loading screen (can be NULL)

	// vars used only for single player mode
	BOOL8 bNewGame;									// is this game either being played for the 1st time or being starting over
	BOOL8 bReplayingALevel;							// TRUE if we are replaying a level and NOT continuing the adventure
	u8 nDifficultyLevel;							// what difficulty should the game be played at, see GameSave_Difficulty_e for the various levels

	// vars used only for multiplayer mode
	u8 nTeamAHumanMask;								// each bit repersents a player index, if bit 0 is set, then player 0 is on team A otherwise he is on team B
	BOOL8 bTeamPlay;								// TRUE if this is a team game
	const GameSave_MPRules_t *pMultiplayerRules;	// in multiplayer mode, what rules should we play by	
};

/////////////////////////
// forward declarations:
class CFTexInst;
typedef void GameFullscreenRenderTargetReadyCallback_t( void );
typedef void ListenerOrientationCallback_t( u32 uPlayerIndex );



///////////////////////////////////////////////////////
// Global Vars:
extern CFTexInst *Game_pFullscreenRenderTarget;
extern GameFullscreenRenderTargetReadyCallback_t *Game_pFcnFullscreenRenderTargetReadyCallback;
extern BOOL Game_bCameraPOVVisibility;
extern cwchar *Game_apwszPhrases[ GAMEPHRASE_COUNT ]; //Pointer of GAMEPHRASE_COUNT elements to an array of pointers to wide char phrases.
extern BOOL Game_bEnterPauseModeImmediatelyForCutsceneCreation; // Override variable used ONLY BY THE SCRIPT SYSTEM for the making of cutscene movies.  DON'T USE FOR NORMAL GAMEPLAY!

///////////////////////////////////////////////////////
// system functions:
extern BOOL game_InitSystem( void );
extern void game_UninitSystem( void );
extern BOOL game_IsInitialized( void );

// loading functions:
// Call this as the first set of localized resources to load
// This is a quick loading to get localized display capabilities ASAP
extern BOOL game_LoadLocalizedPhraseTableAndFonts( void );
#if defined(MA_PC_INPUT)
// Switches the on-screen prompts between key names and pad buttons, following the device the
// keyboard/mouse port used last. Called every input frame.
extern void game_PcPromptWork( void );
#endif
// This loads the rest of the localized assets and systems... This call takes longer but
// a localized loading screen is presented to the user at this time so it's not a big deal...
extern BOOL game_InitLocalizedResources( void );

extern BOOL game_LoadLevel( cchar *pszLevelTitle, BOOL bShowLoadingScreen=TRUE, const GameInitInfo_t *pGameInit=NULL );
extern BOOL game_LoadGenericDebugLevel( cchar *pszWorldResName );
extern void game_UnloadLevel( void );
extern GameTypes_e game_GetCurrentType( void );
extern BOOL game_IsCurrentGameAReplay( void );
extern void game_SetupRenderTargets();


// controller functions:
extern void game_ControlModeWork( void );
extern ControlMode_e game_GetControlMode();

// work/draw functions:
extern BOOL game_Work( void );
extern BOOL game_Draw( void );

// misc functions:
extern BOOL game_BeginCutScene( cchar *pszCutSceneTitle=NULL, BOOL bImmediate=FALSE );
extern BOOL game_EndCutScene( BOOL bImmediate=FALSE ); 

extern BOOL game_EnterLetterbox( cchar *pszCutSceneName=NULL, BOOL bImmediate=FALSE, BOOL bAllowMovieSkip=TRUE ); // Do we want to immediately enter letterbox (ie, no scrolling on of bars)
extern BOOL game_LeaveLetterbox( BOOL bImmediate = FALSE); // Do we want to immediately exit letterbox? (No scrolling?)

extern BOOL game_EnterNonAmbientTalk();
extern BOOL game_EnterBarterMode();
extern BOOL game_LeaveBarterMode();

extern BOOL game_EnterMiniGameMode();
extern BOOL game_LeaveMiniGameMode();

void game_GotoLevel( cchar *pszCommandKeyword );

// Setting the listener orientation callback will cause the callback to get called for each player index in the game.
// The callback is expected to set the listener orientation for each player index when it is called.
extern void game_SetListenerOrientationCallback( ListenerOrientationCallback_t *pCallback );
extern ListenerOrientationCallback_t *game_GetListenerOrientationCallback( void );

extern void game_DrawSolidFullScreenOverlay( f32 fAlpha, f32 fIntensity );
extern void game_HandleControllersBeingUnplugged( void );


#endif

