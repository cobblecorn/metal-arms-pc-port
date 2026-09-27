//////////////////////////////////////////////////////////////////////////////////////
// wpr_system.cpp - the main wrapper system interface
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
// 11/05/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "wpr_system.h"
#include "wpr_datatypes.h"
#include "wpr_drawutils.h"
#include "CamManual.h"
#include "fmath.h"
#include "fvis.h"
#include "gamecam.h"
#include "floop.h"
#include "gamepad.h"
#include "ftext.h"
#include "fstringtable.h"
#include "fstorage.h"
#include "level.h"
#include "ItemInst.h"
#include "faudio.h"
#include "fsndfx.h"
#include "fforce.h"
#include "fmovie2.h"
#include "player.h"
#include "fresload.h"
#include "fclib.h"
#include "frenderer.h"
#include "ftimer.h"
#include "gamesave.h"
#include "gameloop.h"
#include "launcher.h"
#include "game.h"
#include "fcamanim.h"
#include "entity.h"
#include "fsh.h"
#include "eparticlepool.h"
#include "skybox.h"
#include "gcoll.h"
#include "flightgroup.h"
#include "ffile.h"
#include "fdatastreaming.h"
#include "msgbox.h"
#if defined(MA_PC_INPUT)
#include "fvid.h"
#include "pc_input.h"
#endif

#define _DEBUG_NO_SPACE							0

//====================
// private definitions

#define _MAIN_MENU_INACTIVE_TIME				25.0f
#define _MAX_ROWS_OF_KEYS						8
#define _MU_SELECT_DISPLAY_NAME_X				0.50f
#define _MU_SELECT_DISPLAY_NAME_Y				0.26f
#define _MU_SELECT_CUSTOM_NAME_Y				0.30f
#define _MU_SELECT_DISPLAY_NAME_SCALE			1.0f
#define _MU_SELECT_STAT_X						0.55f
#define _CONTROLLER_HEIGHT						0.65f
#define _CONTROLLER_X							0.0f
#define _CONTROLLER_Y							-0.1f
#define _LINE_THICKNESS							3.0f
#define _MENU_MUSIC_VOLUME						0.25f
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	#define _MAX_MU_NAME_DISPLAY_LEN			14
#else
	#define _MAX_MU_NAME_DISPLAY_LEN			20
#endif
#define _MAX_MINUTES_PER_ROUND					120	// Keep this less than 127 or change variable type
#define _MAX_POINTS_PER_ROUND					100
#define _MAX_NUMBER_AI							5
#define _PICK_TEAMS_TITLE_Y_OFFSET				0.035f
#define _PICK_TEAMS_ENTRY_Y_OFFSET				0.03f
#define _PICK_TEAMS_FONT_SCALE					0.85f
#define _MULTIPLAYER_RULES_OPTIONS_X			0.73f
#define _TICKS_TO_PERCENT( nTicks, nTotalTicks)	( (f32)nTicks * (1.0f/(nTotalTicks-1)) )
#define _TOTAL_DEVICES							( FSTORAGE_MAX_DEVICES + 1 )
#define _VIRTUAL_DEVICE_INDEX					( _TOTAL_DEVICES - 1 )// THIS IS THE INDEX WHERE THE VIRTUAL PROFILES WILL BE STORED
#define _NUM_VIRTUAL_PROFILES					( 2 )// THIS IS THE COUNT OF VIRTUAL PROFILES
#define _CREATE_JOIN_KEY( nDeviceIndex, nProfileIndex )			( (nDeviceIndex << 24) | (nProfileIndex & 0x00FFFFFF) )
#define _GET_JOIN_DEVICE_INDEX( nKey )							( nKey >> 24 )
#define _GET_JOIN_PROFILE_INDEX( nKey )							( nKey & 0x00FFFFFF )
#define _TOTAL_FADE_OUT_TIME					0.75f
#define _TOTAL_FADE_IN_TIME						0.45f
#define _MULTIPLAYER_TYPES_X					0.68f
#define _ADV_SETTINGS_TICK_X					0.20066564292907715f
#define _ADV_SETTINGS_FFB_TICK_Y				0.03f
#define _ADV_SETTINGS_LOOK_TICK_Y				-0.09f
#define _ADV_SETTINGS_TICK_HEIGHT				0.070f
#define _ADV_SETTINGS_TICK_SPACE				0.022040f

#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	#if FANG_PLATFORM_PS2
		// ps2 version
		static cchar *_pszControllerScreenPosTableName = "Xbox_Screen_Pos";	//CPS 5.5.03
		static cchar * _pszSierraLogoMovie = "SOL_N.PSS";					//CPS 5.5.03
		static cchar *_pszVUGLogoMovie = "SA_N.PSS";
		static cchar * _pszSASLogoMovie = "SA_N.PSS";						//CPS 5.5.03
		static cchar * _pszMMILogoMovie = "MMI_N.PSS";						//CPS 5.5.03
		static cchar * _pszDemoMovie = "XB_Demo_Mov.bik";					//CPS 5.5.03
		static cchar * _pszTextureTableName = "XB_Textures";				//CPS 5.5.03
		static cchar *_pszPlatformPhrasesTableName = "xb_phrases";
	#else
		// xbox and dx versions
		static cchar *_pszControllerScreenPosTableName = "Xbox_Screen_Pos";
		static cchar * _pszSierraLogoMovie = "XB_SOL_logo.bik";
		static cchar *_pszVUGLogoMovie = "XB_VUG_Logo.bik";
		static cchar * _pszSASLogoMovie = "XB_SAS_logo.bik";
		static cchar * _pszDemoMovie = "XB_Demo_Mov.bik";
		static cchar * _pszTextureTableName = "XB_Textures";
		static cchar *_pszPlatformPhrasesTableName = "xb_phrases";
	#endif
#else
	// gamecube version
	static cchar *_pszControllerScreenPosTableName = "GC_Screen_Pos";
	static cchar * _pszSierraLogoMovie = "GC_SOL_logo.bik";
	static cchar *_pszVUGLogoMovie = "GC_VUG_Logo.bik";
	static cchar * _pszSASLogoMovie = "GC_SAS_logo.bik";
	static cchar * _pszDemoMovie = "GC_Demo_Mov.bik";
	static cchar * _pszTextureTableName = "GC_Textures";
	static cchar *_pszPlatformPhrasesTableName = "gc_phrases";
#endif
static cchar *_pszLevelName = "wfhu_droid";
static cchar *_pszCamFile = "cfhu_droid";
static cchar *_pszCSVFilename = "wrappers$";
static cchar *_pszIGCSVFilename = "IGwrappers$";
static cchar *_pszMeshTableName = "meshes";
static cchar *_pszPhrasesTableName = "common_phrases";
#if FANG_WINGC
// The retail wrappers$ phrase tables were reordered after this source snapshot: the "Press A to ..."
// phrases moved into the platform table and retail-only phrases were added. When the tables have the
// retail sizes, each phrase is read from the field listed here (the common table, the platform table, or
// a literal for the two phrases retail dropped).
#define _RETAIL_COMMON_PHRASE_COUNT		111
#define _RETAIL_PLATFORM_PHRASE_COUNT	19
#define _RETAIL_PHRASE_PLATFORM( n )	( 0x100 | (n) )
#define _RETAIL_PHRASE_LITERAL			0xFFFF
static const u16 _anRetailPhraseField[WPR_DATATYPES_PHRASES_COUNT] = {
	0,	// WPR_DATATYPES_PHRASES_ACCEPT
	1,	// WPR_DATATYPES_PHRASES_BACK
	2,	// WPR_DATATYPES_PHRASES_ON
	3,	// WPR_DATATYPES_PHRASES_OFF
	4,	// WPR_DATATYPES_PHRASES_2WAY
	5,	// WPR_DATATYPES_PHRASES_4WAY
	6,	// WPR_DATATYPES_PHRASES_SPACE
	7,	// WPR_DATATYPES_PHRASES_CAPS
	8,	// WPR_DATATYPES_PHRASES_DONE
	9,	// WPR_DATATYPES_PHRASES_LEVEL
	10,	// WPR_DATATYPES_PHRASES_WASHERS
	11,	// WPR_DATATYPES_PHRASES_SECRET_CHIPS
	12,	// WPR_DATATYPES_PHRASES_ENEMIES
	13,	// WPR_DATATYPES_PHRASES_STATUS
	14,	// WPR_DATATYPES_PHRASES_COMPLETE
	15,	// WPR_DATATYPES_PHRASES_CONFIGURATION
	16,	// WPR_DATATYPES_PHRASES_CONTROLLER
	17,	// WPR_DATATYPES_PHRASES_UPPER
	18,	// WPR_DATATYPES_PHRASES_LOWER
	19,	// WPR_DATATYPES_PHRASES_PORT
	20,	// WPR_DATATYPES_PHRASES_LEFT
	21,	// WPR_DATATYPES_PHRASES_RIGHT
	22,	// WPR_DATATYPES_PHRASES_NONE
	23,	// WPR_DATATYPES_PHRASES_HARD_DISK
	24,	// WPR_DATATYPES_PHRASES_CREATE_NEW_PROFILE
	25,	// WPR_DATATYPES_PHRASES_PRESS_START
	_RETAIL_PHRASE_PLATFORM( 9 ),	// WPR_DATATYPES_PHRASES_PRESS_A_TO_JOIN
	26,	// WPR_DATATYPES_PHRASES_DEFAULT
	27,	// WPR_DATATYPES_PHRASES_INVERTED
	_RETAIL_PHRASE_PLATFORM( 10 ),	// WPR_DATATYPES_PHRASES_PRESS_A_TO_PROCEED
	28,	// WPR_DATATYPES_PHRASES_LOCATION
	_RETAIL_PHRASE_PLATFORM( 11 ),	// WPR_DATATYPES_PHRASES_PRESS_A_TO_ACCEPT
	29,	// WPR_DATATYPES_PHRASES_CHOOSE_PROFILE
	30,	// WPR_DATATYPES_PHRASES_WAITING_FOR_OTHER_PLAYERS
	31,	// WPR_DATATYPES_PHRASES_PLAYER
	32,	// WPR_DATATYPES_PHRASES_SELECTING
	33,	// WPR_DATATYPES_PHRASES_STUPID
	34,	// WPR_DATATYPES_PHRASES_EASY
	35,	// WPR_DATATYPES_PHRASES_REGULAR
	36,	// WPR_DATATYPES_PHRASES_HARD
	37,	// WPR_DATATYPES_PHRASES_INSANE
	38,	// WPR_DATATYPES_PHRASES_RANDOM
	39,	// WPR_DATATYPES_PHRASES_MINUTE
	40,	// WPR_DATATYPES_PHRASES_MINUTES
	41,	// WPR_DATATYPES_PHRASES_NO_LIMIT
	42,	// WPR_DATATYPES_PHRASES_AI_BOT
	43,	// WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_0
	44,	// WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_1
	45,	// WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_2
	46,	// WPR_DATATYPES_PHRASES_STATS
	47,	// WPR_DATATYPES_PHRASES_COLLECTED
	48,	// WPR_DATATYPES_PHRASES_FINISHED_IN
	49,	// WPR_DATATYPES_PHRASES_ENEMIES_DESTROYED
	50,	// WPR_DATATYPES_PHRASES_SECRET_AREAS_FOUND
	51,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT1
	52,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT2
	53,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT3
	54,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT4
	55,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT5
	56,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT6
	57,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT7
	58,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT8
	59,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT9
	60,	// WPR_DATATYPES_PHRASES_PRIMARY_LIMIT10
	61,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT1
	62,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT2
	63,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT3
	64,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT4
	65,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT5
	66,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT6
	67,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT7
	68,	// WPR_DATATYPES_PHRASES_SECONDARY_LIMIT8
	69,	// WPR_DATATYPES_PHRASES_DEATHMATCH
	70,	// WPR_DATATYPES_PHRASES_TIMED_DEATHMATCH
	71,	// WPR_DATATYPES_PHRASES_POSSESSION_MELEE
	72,	// WPR_DATATYPES_PHRASES_KING_OF_THE_HILL
	73,	// WPR_DATATYPES_PHRASES_MOVING_HILL
	74,	// WPR_DATATYPES_PHRASES_REVERSE_TAG
	75,	// WPR_DATATYPES_PHRASES_TAG
	76,	// WPR_DATATYPES_PHRASES_BASE_DEATHMATCH
	77,	// WPR_DATATYPES_PHRASES_BASE_KING_OF_HILL
	78,	// WPR_DATATYPES_PHRASES_BASE_TAG
	79,	// WPR_DATATYPES_PHRASES_BASE_REVERSE_TAG
	80,	// WPR_DATATYPES_PHRASES_INDIVIDUAL
	81,	// WPR_DATATYPES_PHRASES_TEAM
	82,	// WPR_DATATYPES_NO_HILL_SWAPS
	83,	// WPR_DATATYPES_PHRASES_UNABLE_LOAD_PROFILE
	_RETAIL_PHRASE_PLATFORM( 12 ),	// WPR_DATATYPES_PHRASES_PRESS_A_TO_CONTINUE
	84,	// WPR_DATATYPES_PHRASES_VEHICLE_LIMIT1
	85,	// WPR_DATATYPES_PHRASES_VEHICLE_LIMIT2
	86,	// WPR_DATATYPES_PHRASES_VEHICLE_LIMIT3
	87,	// WPR_DATATYPES_PHRASES_VEHICLE_LIMIT4
	88,	// WPR_DATATYPES_PHRASES_VEHICLE_LIMIT5
	89,	// WPR_DATATYPES_PHRASES_CREATE_NEW_GAMETYPE
	90,	// WPR_DATATYPES_PHRASES_DEFAULT_GAMETYPE_NAME
	91,	// WPR_DATATYPES_PHRASES_BUILT_IN_TYPE
	_RETAIL_PHRASE_LITERAL,	// WPR_DATATYPES_PHRASES_TIME_TO_BEAT
	92,	// WPR_DATATYPES_PHRASES_BONUS_SECRET_CHIP_EARNED
	_RETAIL_PHRASE_LITERAL,	// WPR_DATATYPES_PHRASES_LOCKED
	93,	// WPR_DATATYPES_PHRASES_DELETE
	94,	// WPR_DATATYPES_PHRASES_FORMAT
	95,	// WPR_DATATYPES_PHRASES_MA_NEEDS_0
	96,	// WPR_DATATYPES_PHRASES_MA_NEEDS_1
	97,	// WPR_DATATYPES_PHRASES_MA_NEEDS_2
	98,	// WPR_DATATYPES_PHRASES_WRONG_DEVICE
	_RETAIL_PHRASE_PLATFORM( 0 ),	// WPR_DATATYPES_PHRASES_MANAGEMEMORY
	_RETAIL_PHRASE_PLATFORM( 1 ),	// WPR_DATATYPES_PHRASES_MEMORY_CARD_A
	_RETAIL_PHRASE_PLATFORM( 2 ),	// WPR_DATATYPES_PHRASES_MEMORY_CARD_B
	_RETAIL_PHRASE_PLATFORM( 3 ),	// WPR_DATATYPES_PHRASES_MU_UNUSABLE
	_RETAIL_PHRASE_PLATFORM( 4 ),	// WPR_DATATYPES_PHRASES_NO_ROOM_FOR_NEW_PROFILE
	_RETAIL_PHRASE_PLATFORM( 5 ),	// WPR_DATATYPES_PHRASES_INSERT_MEMORY_CARD
	_RETAIL_PHRASE_PLATFORM( 6 ),	// WPR_DATATYPES_PHRASES_DAMAGED_MEMORY_CARD_WARNING
};
#endif
static cchar *_pszControllerConfigs = "Controller_Configs";
static cchar *_pszMusicTableName = "music";
static cchar *_pszSoundTableName = "sound_fx_bank";
static cchar *_pszSoundsEffectTable = "SOUNDS";
static cchar *_pszScreenTableName = "Screen_Table_Names";
static cchar *_pszNoTableUsed = "NULL";
static cchar *_pszFFBMeshIndexTable = "adv_controller_ffb_mesh_index";
static cchar *_pszMPLevelUnLockInfoTableName = "level_unlocking";
static cchar *_pszSPLevelCSV = "pick_level$";
static cchar *_pszLevelsTableName = "Level_Names";
static cchar *_pszMPLevelCSV = "multi_lvl$";
static cchar *_pszMultiplayerLevelTableName = "DeathMatchLevels";	// Actually all MP levels


// the axis up/down/unchanged return values
typedef enum {
	_UP = -1,
	_NOT_UP_OR_DOWN,
	_DOWN,
} _UpDown_e;

// the axis left/right/unchanged return values
typedef enum {
	_LEFT = -1,
	_NOT_LEFT_OR_RIGHT,
	_RIGHT,
} _LeftRight_e;

// this enum hold each different screen's counts
typedef enum {
	// MM = MAIN MENU
	_MENU_ITEMS_MM_SINGLE = 0,
	_MENU_ITEMS_MM_MULTI,
//	_MENU_ITEMS_MM_GAMEDEMOS,
	_MENU_ITEMS_MM_COUNT,

	_MENU_ITEMS_MM_START_OFFSET = 1,

	// SS = SOUND SETTINGS MENU
	_MENU_ITEMS_SS_SOUND = 0,
	_MENU_ITEMS_SS_MUSIC,
	_MENU_ITEMS_SS_COUNT,

	_MENU_ITEMS_SS_START_OFFSET = 2,
	_MENU_ITEMS_SS_NUM_TICKS = 19,

	// PS = PROFILE SETTINGS MENU
	_MENU_ITEMS_PS_EDIT_NAME = 0,
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
	_MENU_ITEMS_PS_EDIT_CONTROLLER,
#endif
	_MENU_ITEMS_PS_ADV_CONTROLLER,
	_MENU_ITEMS_PS_AUDIO,
	_MENU_ITEMS_PS_COLOR,
	_MENU_ITEMS_PS_COUNT,

	_MENU_ITEMS_PS_START_OFFSET = 1,

	// AS = ADVANCED PROFILE SETTINGS MENU
	_MENU_ITEMS_AS_INVERT = 0,
	_MENU_ITEMS_AS_VIBRATION,
#if WPR_SYSTEM_ALLOW_AUTO_CENTER_CHANGES
	_MENU_ITEMS_AS_AUTO_CENTER,
#endif
	_MENU_ITEMS_AS_LOOK_SENSITIVITY,
	_MENU_ITEMS_AS_DPAD_COMBO,
	_MENU_ITEMS_AS_ASSISTED_TARGETING,
	_MENU_ITEMS_AS_COUNT,

	_MENU_ITEMS_AS_START_OFFSET = 2,
	_MENU_ITEMS_AS_NUM_TICKS = 21,

	// PN = PROFILE NAME SCREEN
	_MENU_ITEMS_PN_CAPS_COLUMN = 0,
	_MENU_ITEMS_PN_SPACE_COLUMN,
	_MENU_ITEMS_PN_BACK_COLUMN,

	_MENU_ITEMS_PN_START_OFFSET = 2,

	// LM = LAUNCH MENU SCREEN
	_MENU_ITEMS_LM_START_NEW = 0,
	_MENU_ITEMS_LM_CONTINUE,
	_MENU_ITEMS_LM_REPLAY,
	_MENU_ITEMS_LM_EDIT_SETTINGS,
	_MENU_ITEMS_LM_COUNT,

	_MENU_ITEMS_LM_START_OFFSET = 1,

	// MU = MEMORY UNIT SELECT SCREEN
	_MENU_ITEMS_MU_LOCATION = 0,
	_MENU_ITEMS_MU_BLOCKS_FREE,	
	_MENU_ITEMS_MU_PROFILES_SAVED,
	_MENU_ITEMS_MU_COUNT,

	_MENU_ITEMS_MU_START_OFFSET = 1,

	// PP = PICK PROFILE SCREEN
	_MENU_ITEMS_PP_MEM_UNIT = 0,

	_MENU_ITEMS_PP_ON_SCREEN_LIMIT = 5,
	_MENU_ITEMS_PP_START_OFFSET = 1,

	// PC = PICK COLOR SCREEN
	_MENU_ITEMS_PC_START_OFFSET = 2,

	// MJ = MULTIPLAYER JOIN SCREEN
	_MENU_ITEMS_MJ_PLAYER_HEADING_OFFSET = 2,

	// MT = MULTIPLAYER GAME TYPE
	_MENU_ITEMS_MT_TYPE = 0,
	_MENU_ITEMS_MT_MODE,
	
	_MENU_ITEMS_MT_COUNT,

	_MENU_ITEMS_MT_TYPE_INDIVIDUAL = 0,
	_MENU_ITEMS_MT_TYPE_TEAM,
	_MENU_ITEMS_MT_TYPE_COUNT,

	_MENU_ITEMS_MT_START_OFFSET = 2,
	_MENU_ITEMS_MT_DESCRIPTION_START_OFFSET = (_MENU_ITEMS_MT_START_OFFSET + _MENU_ITEMS_MT_COUNT + 1),
	_MENU_ITEMS_MT_DESCRIPTIONS_PER_TYPE = 2,

	// ME = MULTIPLAYER EDIT GAME TYPES
	_MENU_ITEMS_ME_PROFILE = 0,
	_MENU_ITEMS_ME_GAME_TYPE,

	_MENU_ITEMS_ME_COUNT,

	_MENU_ITEMS_ME_PRECOOKED_DEATHMATCH = 0,
	_MENU_ITEMS_ME_PRECOOKED_DEATHMATCH_TIMED,
	_MENU_ITEMS_ME_PRECOOKED_POSSESSION_MELEE,
	_MENU_ITEMS_ME_PRECOOKED_KOTH_FIXED,
	_MENU_ITEMS_ME_PRECOOKED_KOTH_MOVING,
	_MENU_ITEMS_ME_PRECOOKED_REVERSE_TAG,
	_MENU_ITEMS_ME_PRECOOKED_TAG,

	_MENU_ITEMS_ME_PRECOOKED_COUNT,

	_MENU_ITEMS_ME_START_OFFSET = 2,

	// MR = MULTIPLAYER RULES SCREEN
	_MENU_ITEMS_MR_BASE_TYPE = 0,
	_MENU_ITEMS_MR_TIME_LIMIT,
	_MENU_ITEMS_MR_DM_POINTS_TO_WIN,
	_MENU_ITEMS_MR_KOTH_SWAP_TIME,
	_MENU_ITEMS_MR_ONLY_PRIMARY,
	_MENU_ITEMS_MR_ONLY_SECONDARY,
	_MENU_ITEMS_MR_ONLY_VEHICLE,
	_MENU_ITEMS_MR_POSSESSABLE_BOTS,
	_MENU_ITEMS_MR_POSSESSION_BOT_STATE,
	_MENU_ITEMS_MR_MARKERS,
	_MENU_ITEMS_MR_RADAR,

	_MENU_ITEMS_MR_COUNT,

	_MENU_ITEMS_MR_START_OFFSET = 2,

	// MP = MULTIPLAYER PICK TEAMS SCREEN
	_MENU_ITEMS_MP_TEAM_A_OFFSET = 2,
	_MENU_ITEMS_MP_TEAM_B_OFFSET = 3,

	// DL = DIFFICULTY LEVEL
	_MENU_ITEMS_DL_EASY = 0,
	_MENU_ITEMS_DL_NORMAL,
	_MENU_ITEMS_DL_HARD,
	_MENU_ITEMS_DL_INSANE,

	_MENU_ITEMS_DL_COUNT,

	_MENU_ITEMS_DL_START_OFFSET = 1,

	// FC = FORMAT CORRUPT CARD
	_MENU_ITEMS_FC_CONTINUEWITHOUTFORMAT = 0,
	_MENU_ITEMS_FC_FORMAT,
	_MENU_ITEMS_FC_RETRY,
	_MENU_ITEMS_FC_COUNT,

	_MENU_ITEMS_FC_START_OFFSET = 4,
	
	// EL = ERROR LOADING
	_MENU_ITEMS_EL_SELECT_DIFF_PROFILE = 0,
	_MENU_ITMES_EL_RESET_PROFILE,
	_MENU_ITMES_EL_NO_SAVE,
	_MENU_ITEMS_EL_DELETE,
	_MENU_ITEMS_EL_RETRY,
	_MENU_ITEMS_EL_COUNT,
	
	_MENU_ITEMS_EL_START_OFFSET = 1,
	
	// DR = DON'T REMOVE
	_MENU_ITEMS_DR_HD_SAVE_FIRST_OFFSET = 1,
	_MENU_ITEMS_DR_HD_SAVE_LAST_OFFSET = 2,
	
	
} _MenuItems_e;

typedef struct {
	u32 anNumProfiles[_TOTAL_DEVICES];// the last device is not real, it is just to hold the 2 default profiles
	u32 nTotalProfiles;
	u32 nDefaultKey;// use this key when someone first joins the game
	BOOL bWaitingForOtherPlayers;
} _JoinInfo_t;

typedef enum {
	_MULTI_JOIN_STATE_ENTER = 0,
	_MULTI_JOIN_STATE_SELECT,
	_MULTI_JOIN_STATE_LOAD_ERROR,
	_MULTI_JOIN_STATE_WAIT,

	_MULTI_JOIN_STATE_COUNT
} _MultiJoin_States_e;

typedef struct {
	u16 nState;// one of _MultiJoin_States_e values
	BOOL8 bWorkDoneThisFrame;
	u8 nTeamID;
	u32 nProfileKey;
	CPlayerProfile *pProfile;	
} _MultiJoin_SectionData_t;// a section is one of the boxes displayed on the screen ( 1 player per section )

typedef struct {
	u32 nNumLevels;
	u16 **papwzLocations;	// nNumLevels of these
	u16 **papwzNames;		// nNumLevels of these
	CFTexInst *paTexInsts;	// nNumLevels of these
	u32 *panSecretChips;	// nNumLevels of these
	f32 *pafTimeToBeat;		// nNumLevels of these
	u32 *panLevelNumbers;	// nNumLevels of these
} _LevelSelectInfo_t;

typedef enum {
	_LAUNCH_CODE_NOT_STARTED = 0,
	_LAUNCH_CODE_TRIGGER_DOWN,
	_LAUNCH_CODE_A_DOWN,
	_LAUNCH_CODE_B_DOWN,
	_LAUNCH_CODE_X_DOWN,
	_LAUNCH_CODE_Y_DOWN,

	_LAUNCH_CODE_ENTERED
} _LaunchCode_e;

typedef enum {
	_START_METHOD_NEW = 0,
	_START_METHOD_CONTINUE,
	_START_METHOD_REPLAY,
	_START_METHOD_MULTIPLAYER,
	
	_START_METHOD_COUNT
} _StartMethod_e;

typedef struct {
	u8 nNumFreebies;
	u8 *panChipsToUnlock;// LEVEL_MULTIPLAYER_COUNT - nNumFreebies of these
} _MPLevelUnlockInfo_t;

// this structure is a collection of all of the data that is needed by the wrappers screens
typedef struct {
	// info that tells us what screen we are on, where we came from, and what item we are currently on
	f32 fModeTimer;
	u16 nCurrentScreen;// Wpr_DataTypes_Screens_e...
	u8 nLastScreen;// Wpr_DataTypes_Screens_e...
	BOOL8 bFadeScreen;
	s8 nCurItemIndex;
	u8 nStartGameMethod;// how did we arrive at the start game (one of _StartMethod_e values) 
	u8 nLevelToPlay;	
	u8 nMode;// Wpr_DataTypes_Modes_e...
	s8 nControllerIndex;// tells us what controller to sample
	s8 nUnpluggedControllerState;
	u8 nButtonDrawMask;// tells the button renderer which buttons to draw

	// general multiplayer specific vars
	u8 nMPNumPlayers;	// tells us how many players are in the round
	GameSave_MPRules_t MPRules;
	u8 nMPPlayerMask;	// tells us what players are in the round	
	u8 nMPTeamAHumanMask;// TeamB Human Mask = nMPPlayerMask & ~nMPTeamAHumanMask

	// MU - 'mem unit select' vars
	u16 nMUNumEntries;
	Wpr_DataTypes_MUSelectEntry_t *paMUEntries;
	cwchar* pwszMUManageMemory;
	cwchar* pwszMUFormat;

	// PP - 'pick profile' vars
	BOOL8 bPPRoomForNewProfiles;
	BOOL8 bPPNeedToCacheProfiles;
	s16 nPPBottomSelectionIndex;
	s16 nPPTopSelectionIndex;	
	s16 nPPNumSelections;
	const FStorage_DeviceInfo_t *pPPDevInfo;
	FStorage_ProfileInfo_t aPPProfileInfos[_MENU_ITEMS_PP_ON_SCREEN_LIMIT];	
				
	// SS - 'sound settings' screen vars
	f32 fSSOrigSoundPercent;
	s8 nSSNumMusicTicks;
	s8 nSSNumSoundTicks;	

	// AS - 'advanced settings' vars
	BOOL8 bASInvertAnalog;
	BOOL8 bASAssistedTargeting;
	BOOL8 bASForceFeedbackON;
	BOOL8 bASAutoCenter;
	s16 nASVibrationTicks;
	s16 nASLookSensitivity;
	BOOL8 bASFourWayQuickSelect;
	u8 nASIndexOfFFBMesh;
	f32 fASForceFeedbackOrigIntensity;
	FForceHandle_t hASForceFeedback;
	
	// PN - 'profile name' vars
	s8 nPNCurRow;
	u8 nPNNumRows;
	BOOL8 bPNLower;
	s8 nPNCurCol;
	u32 anPNColumnsPerRow[_MAX_ROWS_OF_KEYS];
	wchar wszPNCurProfileName[FSTORAGE_MAX_NAME_LEN];// Unicode.
	
	// LM - 'launch' vars
	_LaunchCode_e nLMCodeState;// where are we in the code entry (to allow all levels to be opened up)
	BOOL8 bLMNewProfile;// this is a profile that is brand new (the player hasn't made it past level 1)

	// DR - 'do not remove' vars
	BOOL8 bWriteSuccessful;// was the write ok?
	s16 nDRScreenToReturn;// what screen should we return to after saving the profile
	f32 fDRTimer;
	wchar *pwszDRNewName;
	CPlayerProfile *pDRProfile;
	
	// PL - 'pick level' vars
	_LevelSelectInfo_t PLSelectInfo;
		
	// CC - 'controller config' vars
	WprSystem_ControllerConfigData_t CCData;

	// MJ - 'multiplayer join' vars
	_JoinInfo_t MJJoinInfo;
	_MultiJoin_SectionData_t aMJSections[MAX_PLAYERS];

	// MR - 'multiplayer rules' vars
	BOOL bMRMadeChanges;
	s32 nMRTmpRuleType;			// If we are editing a read-only type, this gives us the type number
	s32 nMRProfileIndex;		// Index into MEUniqueProfiles of current profile, or -1 if no profile
	s32 nMRRuleIndex;			// Index into array of rules in profile of rules being edited. N/A if nMRProfileIndex < 0
	GameSave_MPRules_t MR_WorkingRules;

	// ML - 'multiplayer level select' vars
	_LevelSelectInfo_t MLLevelSelectInfo;
	_MPLevelUnlockInfo_t MLUnLockInfo;
	s16 nMLNumUnlockedLevels;

	// MP - 'multiplayer pick teams' vars
	s16 nMPBalanceDirection;

	// MT - 'multiplayer game type' vars
	s16 nMTType;			// Index from 0 to nMTTypeCount of current game meta type
	s16 nMTMode;			// Individual or team play mode

	// ME - 'multiplayer edit game types' vars
	CPlayerProfile*	apMEUniqueProfiles[MAX_PLAYERS];	// Unique profiles found for editing game types
	BOOL bMEAppend;										// TRUE if edited rules should be appended to the current profile
	s16 nMEUniqueProfileCount;							// How many unique profiles are in our array
	s16 nMEPlayerIndexToGetRulesFrom;					// Player in the unique profile array
	s16 nMEType;										// Currently selected game rule set in current profile

	// HD - 'hard drive out of space warning
	u16 uHDNeededBlocks;
		
	// FC - 'format memory card'
	u32 nFCEntryArrayID;
	cwchar* pwszFCDeviceName;
	FStorage_DeviceID_e eFCMemCardID;	
} _MenuData_t;

//=================
// public variables

//==================
// private variables

static BOOL8 _bSystemOK = FALSE;
static BOOL8 _bPrevGovernorState;
static BOOL8 _bInGame;
static BOOL8 _bBootup;
static _MenuData_t _MenuState;
static FResFrame_t _ResFrame;
static CFStringTable *_pStringTable;
static FSndFx_FxHandle_t _ahSounds[WPR_DATATYPES_SOUNDS_COUNT];
static u32 _nNumMeshes;
static CFMeshInst *_paMeshInsts;// points to _nNumMeshes *
static CFTexInst *_paTexInsts;// points to WPR_DATATYPES_TEXTURES_COUNT
static cwchar *_apwszPhrases[WPR_DATATYPES_PHRASES_COUNT];

static CFAudioStream *_pAudioStream;
static CSkyBox *_pSkyBox;
static FViewport_t *_pViewportOrtho3D = NULL;
static CCamManualInfo _CamInfo;
static CFMtx43A _CamMtx;
static CFCamAnimInst *_pCamAnimInst;

// loaded profiles
static CPlayerProfile *_paProfiles = NULL;// when non-NULL, will point to MAX_PLAYERS elements
static GameInitInfo_t _GameInitInfo;// filled in when we go to actually start a game

#if defined(MA_PC_INPUT)
//===================================
// mouse pointer (PC port)
//
// Draw functions record where the selectable list items were printed (ids in _MenuState.nCurItemIndex's
// space) and wpr_drawutils records the button prompts; the next frame's Work hovers and clicks them. Only
// the front end's keyboard/mouse port responds:
//	- moving the pointer over an item selects it; hover never changes the selection while the pointer is
//	  still, so it doesn't fight the keyboard or a pad
//	- a click on the selected item is A (on screens where A picks the item), a right click is B, and a click
//	  on a button prompt is that button
//	- the wheel steps up/down through lists (left/right on screens that page sideways); over a selected
//	  setting it changes the value, as left/right do
//	- a click on the zones beside a sideways chooser is left/right
// Keyboard or pad menu input hides the pointer until the mouse moves again.

#define _MOUSE_MAX_ITEMS			128			// the name keyboard alone has 53 items
#define _MOUSE_POINTER_TEXTURE		"tfh_cross01"	// the HUD's triangular reticle, used apex up as an arrow
#define _MOUSE_POINTER_HEIGHT		0.10f			// bipolar units (2 = the screen height)
#define _MOUSE_POINTER_APEX_V		0.07f			// the apex's row in the texture, as a fraction of its height
#define _MOUSE_ITEM_SLACK_Y			0.006f			// screen fraction added above and below each item
#define _MOUSE_ITEM_MIN_WIDTH		0.045f			// narrower items (single keys, "l", "-") are widened to this

enum {
	_MOUSE_ITEM_ACCEPT = 0x01,		// a click on the item while it is selected is A
	_MOUSE_ITEM_ADJUST = 0x02,		// a setting: the wheel over it while it is selected is left/right
};
#define _MOUSE_ZONE_LEFT			(-2)	// item ids of click zones that act as left/right
#define _MOUSE_ZONE_RIGHT			(-3)

typedef struct {
	f32 fLeft, fTop, fRight, fBottom;
	s32 nItem;
	u32 nFlags;
} _MouseItem_t;

typedef enum {
	_MOUSE_BUTTON_A = 0,	// the prompt order of WPR_DATATYPES_DRAW_*_BUTTON_ONLY
	_MOUSE_BUTTON_B,
	_MOUSE_BUTTON_Y,
	_MOUSE_BUTTON_X,
} _MouseButton_e;

static _MouseItem_t _aMouseItems[_MOUSE_MAX_ITEMS];
static u32 _nMouseItems;
static s32 _nMouseItemsScreen = -1;		// the screen the items and prompts were drawn for
static s32 _nMouseHoverItem = -1;		// the item under the pointer this frame
static u32 _nMouseHoverFlags;
static s32 _nMouseHoverZone;			// _MOUSE_ZONE_* under the pointer, or 0
static s32 _nMouseHoverButton = -1;		// the prompt under the pointer this frame
static BOOL _bMouseMoved;				// the pointer moved or clicked this frame
static u32 _nMouseClickFrames;			// frames a left click stays live: it may select an item, then accept it
static BOOL _bMouseClickUsed;
static s32 _nMouseClickItem = -1;		// what the live left click landed on (hit-tested where it happened)
static u32 _nMouseClickFlags;
static s32 _nMouseClickZone;
static s32 _nMouseClickButton = -1;
static s32 _nMouseClickScreen = -1;		// the screen the live click was hit-tested against
static BOOL _bMouseRightClick;
static s32 _nMouseWheelPending;			// notches not yet delivered, + = away from the user (up)
static s32 _nMouseWheelStep;			// this frame's step, as _UpDown_e
static CFTexInst _MousePointerTex;
static FVidDrawOverlayFcn_t *_pMousePrevOverlayFcn;
static BOOL _bMouseOverlayInstalled;
static BOOL _bMouseDrawPointer;			// the menus drew this frame; draw the pointer over their text
static BOOL _bMouseDebug;				// MA_PORT_POINTER_DEBUG: outline the hit boxes and log clicks

static BOOL _MousePort( u32 nControllerID ) {
	return !_bInGame && nControllerID == pcinput_KeyboardPort();
}

static void _MouseAddBox( s32 nItem, u32 nFlags, f32 fLeft, f32 fTop, f32 fRight, f32 fBottom ) {
	if( _nMouseItems >= _MOUSE_MAX_ITEMS || fRight <= fLeft || fBottom <= fTop ) {
		return;
	}
	if( nItem >= 0 && fRight - fLeft < _MOUSE_ITEM_MIN_WIDTH ) {
		f32 fCenter = 0.5f * (fLeft + fRight);
		fLeft = fCenter - 0.5f * _MOUSE_ITEM_MIN_WIDTH;
		fRight = fCenter + 0.5f * _MOUSE_ITEM_MIN_WIDTH;
	}
	_MouseItem_t *pItem = &_aMouseItems[_nMouseItems++];
	pItem->fLeft = fLeft;
	pItem->fTop = fTop;
	pItem->fRight = fRight;
	pItem->fBottom = fBottom;
	pItem->nItem = nItem;
	pItem->nFlags = nFlags;
}

// Draw side: records the list item nItem, just printed by _DrawText() or ftext_Printf().
static void _MouseAddItem( s32 nItem, u32 nFlags=_MOUSE_ITEM_ACCEPT ) {
	f32 fLeft, fTop, fRight, fBottom;
	if( nItem >= 0 && ftext_GetLastPrintBounds( &fLeft, &fTop, &fRight, &fBottom ) ) {
		_MouseAddBox( nItem, nFlags, fLeft, fTop, fRight, fBottom );
	}
}

// Draw side: records the list item nItem, drawn as a mesh by wpr_drawutils_DrawMesh_XlatOnly() (the
// retail main menu's items are 3D text). Its model-space box is placed as that function places it.
static void _MouseAddMesh( s32 nItem, const Wpr_DataTypes_MeshLayout_t *pMesh,
						   f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes, u32 nFlags=_MOUSE_ITEM_ACCEPT ) {
	if( !pMesh->pMeshInst || !pMesh->pMeshInst->m_pMesh ) {
		return;
	}
	const FMesh_t *pFMesh = pMesh->pMeshInst->m_pMesh;
	f32 fScale = pMesh->fScale * fScaleMultiplier;
	// ortho pixels, y up, origin at the screen center -> fractions of the screen, y down
	f32 fMinX = pMesh->fBiPolarUnitX * fHalfXRes + pFMesh->vBoundBoxMin_MS.x * fScale;
	f32 fMaxX = pMesh->fBiPolarUnitX * fHalfXRes + pFMesh->vBoundBoxMax_MS.x * fScale;
	f32 fMinY = pMesh->fBiPolarUnitY * fHalfYRes + pFMesh->vBoundBoxMin_MS.y * fScale;
	f32 fMaxY = pMesh->fBiPolarUnitY * fHalfYRes + pFMesh->vBoundBoxMax_MS.y * fScale;
	_MouseAddBox( nItem, nFlags,
				  (fMinX / fHalfXRes + 1.0f) * 0.5f, (1.0f - fMaxY / fHalfYRes) * 0.5f,
				  (fMaxX / fHalfXRes + 1.0f) * 0.5f, (1.0f - fMinY / fHalfYRes) * 0.5f );
}

// Draw side: left/right click zones fWidth wide on either side of what was just printed.
static void _MouseAddArrowZones( f32 fWidth ) {
	f32 fLeft, fTop, fRight, fBottom;
	if( ftext_GetLastPrintBounds( &fLeft, &fTop, &fRight, &fBottom ) ) {
		_MouseAddBox( _MOUSE_ZONE_LEFT, 0, fLeft - fWidth, fTop, fLeft, fBottom );
		_MouseAddBox( _MOUSE_ZONE_RIGHT, 0, fRight, fTop, fRight + fWidth, fBottom );
	}
}

// How the current screen's generic list rows (wpr_system_DrawBasicScreen) respond.
static u32 _MouseBasicScreenItemFlags( void ) {
	switch( _MenuState.nCurrentScreen ) {
	case WPR_DATATYPES_SCREENS_SOUND_OPTIONS:
		// A leaves the screen; the rows are volume sliders
		return _MOUSE_ITEM_ADJUST;
	default:
		return _MOUSE_ITEM_ACCEPT;
	}
}

// What is at (fX, fY): a prompt (*pnButton), an item (*pnItem, *pnFlags) or a left/right zone (*pnZone).
static void _MouseHitTest( f32 fX, f32 fY, s32 *pnItem, u32 *pnFlags, s32 *pnZone, s32 *pnButton ) {
	u32 i;
	*pnItem = *pnButton = -1;
	*pnFlags = 0;
	*pnZone = 0;
	if( _nMouseItemsScreen != _MenuState.nCurrentScreen ) {
		return;
	}
	for( i=0; i < WPR_DRAWUTILS_BUTTON_HITS; i++ ) {
		const Wpr_DrawUtils_ButtonHit_t *pHit = &Wpr_DrawUtils_aButtonHits[i];
		if( pHit->bDrawn && fX >= pHit->fLeft && fX <= pHit->fRight && fY >= pHit->fTop && fY <= pHit->fBottom ) {
			*pnButton = (s32)i;
			return;
		}
	}
	// where padded boxes overlap, the one whose center is nearest wins
	const _MouseItem_t *pBest = NULL;
	f32 fBestDistSq = 0.0f;
	for( i=0; i < _nMouseItems; i++ ) {
		const _MouseItem_t *pItem = &_aMouseItems[i];
		if( fX >= pItem->fLeft && fX <= pItem->fRight &&
			fY >= pItem->fTop - _MOUSE_ITEM_SLACK_Y && fY <= pItem->fBottom + _MOUSE_ITEM_SLACK_Y ) {
			f32 fDX = fX - 0.5f * (pItem->fLeft + pItem->fRight), fDY = fY - 0.5f * (pItem->fTop + pItem->fBottom);
			f32 fDistSq = fDX * fDX + fDY * fDY;
			if( !pBest || fDistSq < fBestDistSq ) {
				pBest = pItem;
				fBestDistSq = fDistSq;
			}
		}
	}
	if( pBest ) {
		if( pBest->nItem >= 0 ) {
			*pnItem = pBest->nItem;
			*pnFlags = pBest->nFlags;
		} else {
			*pnZone = pBest->nItem;
		}
	}
}

static void _MouseBeginDraw( void ) {
	_nMouseItems = 0;
	_nMouseItemsScreen = _MenuState.nCurrentScreen;
	wpr_drawutils_ClearButtonHits();
}

// Work side: once per front-end frame, after gamepad_Sample().
static void _MouseFrame( void ) {
	f32 fX, fY;

	// one left click at a time: it stays live for two frames (it may select an item, then accept it) and
	// the next queued click waits until it is used or expires
	if( _bMouseClickUsed || _nMouseClickScreen != _MenuState.nCurrentScreen ) {
		// used, or the screen it was aimed at has gone
		_nMouseClickFrames = 0;
		_bMouseClickUsed = FALSE;
	} else if( _nMouseClickFrames ) {
		_nMouseClickFrames--;
	}
	if( !_nMouseClickFrames ) {
		_nMouseClickItem = _nMouseClickButton = -1;
		_nMouseClickFlags = 0;
		_nMouseClickZone = 0;
		if( pcinput_TakeMenuClick( FALSE, &fX, &fY ) ) {
			_nMouseClickFrames = 2;
			_nMouseClickScreen = _MenuState.nCurrentScreen;
			_MouseHitTest( fX, fY, &_nMouseClickItem, &_nMouseClickFlags, &_nMouseClickZone, &_nMouseClickButton );
			if( _bMouseDebug ) {
				DEVPRINTF( "wpr mouse: click %.3f,%.3f screen %d -> item %d button %d zone %d (selected %d)\n", fX, fY,
					_MenuState.nCurrentScreen, _nMouseClickItem, _nMouseClickButton, _nMouseClickZone, _MenuState.nCurItemIndex );
			}
		}
	}
	_bMouseRightClick = pcinput_TakeMenuClick( TRUE, NULL, NULL );

	_nMouseWheelPending += pcinput_TakeMenuWheel();
	FMATH_CLAMP( _nMouseWheelPending, -3, 3 );
	_nMouseWheelStep = (_nMouseWheelPending > 0) ? _UP : ( (_nMouseWheelPending < 0) ? _DOWN : _NOT_UP_OR_DOWN );
	_nMouseWheelPending += _nMouseWheelStep;

	_bMouseMoved = pcinput_MenuPointerMoved();
	if( pcinput_MenuPointer( &fX, &fY ) ) {
		_MouseHitTest( fX, fY, &_nMouseHoverItem, &_nMouseHoverFlags, &_nMouseHoverZone, &_nMouseHoverButton );
	} else {
		_nMouseHoverItem = _nMouseHoverButton = -1;
		_nMouseHoverFlags = 0;
		_nMouseHoverZone = 0;
	}
}

// A left click on the selected item (where that means A) or on the A prompt.
static BOOL _MouseAccept( u32 nControllerID ) {
	if( !_MousePort( nControllerID ) || !_nMouseClickFrames ) {
		return FALSE;
	}
	if( _nMouseClickButton == _MOUSE_BUTTON_A ||
		( (_nMouseClickFlags & _MOUSE_ITEM_ACCEPT) && _nMouseClickItem >= 0 && _nMouseClickItem == _MenuState.nCurItemIndex ) ) {
		_bMouseClickUsed = TRUE;
		return TRUE;
	}
	return FALSE;
}

// A left click on the prompt for nButton.
static BOOL _MousePrompt( u32 nControllerID, _MouseButton_e nButton ) {
	if( !_MousePort( nControllerID ) || !_nMouseClickFrames || _nMouseClickButton != (s32)nButton ) {
		return FALSE;
	}
	_bMouseClickUsed = TRUE;
	return TRUE;
}

static BOOL _MouseBack( u32 nControllerID ) {
	return ( _MousePort( nControllerID ) && _bMouseRightClick ) || _MousePrompt( nControllerID, _MOUSE_BUTTON_B );
}

// The wheel as up/down, unless it is adjusting the setting under the pointer.
static _UpDown_e _MouseUpDown( u32 nControllerID ) {
	if( !_MousePort( nControllerID ) || (_nMouseHoverFlags & _MOUSE_ITEM_ADJUST) ) {
		return _NOT_UP_OR_DOWN;
	}
	return (_UpDown_e)_nMouseWheelStep;
}

// A click on a left/right zone, or the wheel over the selected setting (up = right).
static _LeftRight_e _MouseLeftRight( u32 nControllerID ) {
	if( !_MousePort( nControllerID ) ) {
		return _NOT_LEFT_OR_RIGHT;
	}
	if( _nMouseClickFrames && _nMouseClickZone ) {
		_bMouseClickUsed = TRUE;
		return (_nMouseClickZone == _MOUSE_ZONE_LEFT) ? _LEFT : _RIGHT;
	}
	if( _nMouseWheelStep && (_nMouseHoverFlags & _MOUSE_ITEM_ADJUST) && _nMouseHoverItem == _MenuState.nCurItemIndex ) {
		return (_nMouseWheelStep == _UP) ? _RIGHT : _LEFT;
	}
	return _NOT_LEFT_OR_RIGHT;
}

// The item that should become the selection (0..nMaxValidIndex and not already selected): the one a live
// click landed on, else the one under a moving pointer. -1 for none.
static s32 _MouseHoverSelect( u32 nControllerID, s32 nMaxValidIndex ) {
	if( !_MousePort( nControllerID ) ) {
		return -1;
	}
	s32 nItem = -1;
	if( _nMouseClickFrames && _nMouseClickItem >= 0 ) {
		nItem = _nMouseClickItem;
	} else if( _bMouseMoved ) {
		nItem = _nMouseHoverItem;
	}
	if( nItem < 0 || nItem > nMaxValidIndex || nItem == _MenuState.nCurItemIndex ) {
		return -1;
	}
	return nItem;
}

// Keyboard or pad input on the mouse's port hides the pointer until the mouse moves again.
static void _MouseOtherInput( u32 nControllerID ) {
	if( _MousePort( nControllerID ) ) {
		pcinput_HideMenuPointer();
	}
}

static void _MouseDrawPointerQuad( f32 fApexX, f32 fApexY, f32 fHalfXRes, f32 fHalfYRes, const CFColorRGBA &rColor ) {
	// the apex is the hotspot; the texture's center sits below it
	f32 fHalfY = 0.5f * _MOUSE_POINTER_HEIGHT;
	f32 fHalfX = fHalfY * fHalfYRes / fHalfXRes;
	f32 fCenterY = fApexY - (0.5f - _MOUSE_POINTER_APEX_V) * _MOUSE_POINTER_HEIGHT;
	f32 fLowerX = (fApexX - fHalfX) * fHalfXRes, fUpperX = (fApexX + fHalfX) * fHalfXRes;
	f32 fLowerY = (fCenterY - fHalfY) * fHalfYRes, fUpperY = (fCenterY + fHalfY) * fHalfYRes;

	Wpr_DrawUtils_aVtx[0].ST.Set( 0.0f, 1.0f );
	Wpr_DrawUtils_aVtx[1].ST.Set( 0.0f, 0.0f );
	Wpr_DrawUtils_aVtx[2].ST.Set( 1.0f, 1.0f );
	Wpr_DrawUtils_aVtx[3].ST.Set( 1.0f, 0.0f );
	Wpr_DrawUtils_aVtx[0].Pos_MS.Set( fLowerX, fLowerY, 1.0f );
	Wpr_DrawUtils_aVtx[1].Pos_MS.Set( fLowerX, fUpperY, 1.0f );
	Wpr_DrawUtils_aVtx[2].Pos_MS.Set( fUpperX, fLowerY, 1.0f );
	Wpr_DrawUtils_aVtx[3].Pos_MS.Set( fUpperX, fUpperY, 1.0f );
	for( u32 i=0; i < 4; i++ ) {
		Wpr_DrawUtils_aVtx[i].ColorRGBA = rColor;
	}
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, Wpr_DrawUtils_aVtx, 4 );
}

// Runs after ftext_Draw() so the pointer sits above the menu text.
static void _MouseDrawOverlay( void ) {
	if( _pMousePrevOverlayFcn ) {
		_pMousePrevOverlayFcn();
	}
	if( !_bMouseDrawPointer ) {
		return;
	}
	_bMouseDrawPointer = FALSE;

	f32 fX, fY;
	if( !_pViewportOrtho3D || !_MousePointerTex.GetTexDef() || !pcinput_MenuPointer( &fX, &fY ) ) {
		return;
	}
	FViewport_t *pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );
	CFXfm::InitStack();
	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( &_MousePointerTex );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	f32 fHalfXRes = _pViewportOrtho3D->HalfRes.x, fHalfYRes = _pViewportOrtho3D->HalfRes.y;
	f32 fApexX = fX * 2.0f - 1.0f, fApexY = 1.0f - fY * 2.0f;
	// a dark outline keeps the white reticle readable over bright scenery
	CFColorRGBA Color;
	Color.Set( 0.0f, 0.0f, 0.05f, 0.85f );
	f32 fPixelX = 1.5f / fHalfXRes, fPixelY = 1.5f / fHalfYRes;
	_MouseDrawPointerQuad( fApexX - fPixelX, fApexY, fHalfXRes, fHalfYRes, Color );
	_MouseDrawPointerQuad( fApexX + fPixelX, fApexY, fHalfXRes, fHalfYRes, Color );
	_MouseDrawPointerQuad( fApexX, fApexY - fPixelY, fHalfXRes, fHalfYRes, Color );
	_MouseDrawPointerQuad( fApexX, fApexY + fPixelY, fHalfXRes, fHalfYRes, Color );
	Color.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	_MouseDrawPointerQuad( fApexX, fApexY, fHalfXRes, fHalfYRes, Color );

	if( _bMouseDebug ) {
		fdraw_SetTexture( NULL );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
		CFColorRGBA BoxColor( 0.0f, 1.0f, 0.2f, 1.0f );
		for( u32 i=0; i <= _nMouseItems + WPR_DRAWUTILS_BUTTON_HITS; i++ ) {
			f32 fL, fT, fR, fB;
			if( i < _nMouseItems ) {
				fL = _aMouseItems[i].fLeft; fT = _aMouseItems[i].fTop - _MOUSE_ITEM_SLACK_Y;
				fR = _aMouseItems[i].fRight; fB = _aMouseItems[i].fBottom + _MOUSE_ITEM_SLACK_Y;
			} else if( i < _nMouseItems + WPR_DRAWUTILS_BUTTON_HITS && Wpr_DrawUtils_aButtonHits[i - _nMouseItems].bDrawn ) {
				const Wpr_DrawUtils_ButtonHit_t *pHit = &Wpr_DrawUtils_aButtonHits[i - _nMouseItems];
				fL = pHit->fLeft; fT = pHit->fTop; fR = pHit->fRight; fB = pHit->fBottom;
			} else {
				continue;
			}
			CFVec3 a( (fL*2.0f-1.0f)*fHalfXRes, (1.0f-fT*2.0f)*fHalfYRes, 1.0f ), b( (fR*2.0f-1.0f)*fHalfXRes, (1.0f-fT*2.0f)*fHalfYRes, 1.0f );
			CFVec3 c( (fR*2.0f-1.0f)*fHalfXRes, (1.0f-fB*2.0f)*fHalfYRes, 1.0f ), d( (fL*2.0f-1.0f)*fHalfXRes, (1.0f-fB*2.0f)*fHalfYRes, 1.0f );
			fdraw_SolidLine( &a, &b, &BoxColor ); fdraw_SolidLine( &b, &c, &BoxColor );
			fdraw_SolidLine( &c, &d, &BoxColor ); fdraw_SolidLine( &d, &a, &BoxColor );
		}
	}

	frenderer_Pop();
	fviewport_SetActive( pPrevViewport );
}

static void _MouseInstall( void ) {
	char szDebug[8];
	_bMouseDebug = GetEnvironmentVariableA( "MA_PORT_POINTER_DEBUG", szDebug, sizeof( szDebug ) ) > 0 && szDebug[0] == '1';
	_MousePointerTex.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _MOUSE_POINTER_TEXTURE ) );
	if( !_bMouseOverlayInstalled ) {
		_pMousePrevOverlayFcn = fvid_GetDrawOverlayFcn();
		fvid_SetDrawOverlayFcn( _MouseDrawOverlay );
		_bMouseOverlayInstalled = TRUE;
	}
	_nMouseItems = 0;
	_nMouseItemsScreen = -1;
	_nMouseClickFrames = 0;
	_nMouseClickItem = _nMouseClickButton = -1;
	_nMouseClickZone = 0;
	_bMouseClickUsed = _bMouseRightClick = _bMouseDrawPointer = FALSE;
	_nMouseWheelPending = _nMouseWheelStep = 0;
}

static void _MouseUninstall( void ) {
	if( _bMouseOverlayInstalled ) {
		fvid_SetDrawOverlayFcn( _pMousePrevOverlayFcn );
		_pMousePrevOverlayFcn = NULL;
		_bMouseOverlayInstalled = FALSE;
	}
	_MousePointerTex.SetTexDef( NULL );
	_nMouseItems = 0;
	_nMouseItemsScreen = -1;
	_bMouseDrawPointer = FALSE;
}
#endif



//===================
// private prototypes

static BOOL _Init( void );
static BOOL _Work( void );
static BOOL _Draw( void );
static BOOL _WrappersLoadingCallback( cchar *pszLoadingString = NULL );

///////////////////////
// CONTROLLER ROUTINES
///////////////////////
static _UpDown_e _CheckUpDownAxis( u32 nControllerID );
static _LeftRight_e _CheckLeftRightAxis( u32 nControllerID );
static BOOL _CheckAcceptButtons( u32 nControllerID );
static BOOL _CheckBackButtons( u32 nControllerID );
static BOOL _CheckYButton( u32 nControllerID );
static BOOL _CheckXButton( u32 nControllerID );
static BOOL _HandleUnpluggedController( u32 nControllerIndex );
static BOOL _ControllerWaitForReconnect( void );
static BOOL _ControllerWaitForStart( void );

/////////////////
// MISC ROUTINES
/////////////////
static BOOL _InitUnLockInfo( _MPLevelUnlockInfo_t *pUnLockInfo, FGameDataTableHandle_t hTable, cchar *pszTableName );
static BOOL _InitLevelSelectInfo( _LevelSelectInfo_t *pSelectInfo, FGameDataTableHandle_t hTable, cchar *pszTableName );
static void _DrawText( Wpr_DataTypes_TextLayout_t *pText,
					BOOL bSelected,
					f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
					BOOL bDisabled=FALSE,
					BOOL bDrawSelectedArrows=FALSE );
static void _DrawSelectingPlayerMsg( f32 fX=0.155f, f32 fY=0.215f );
static void _DrawOnOffSelection( f32 fX, f32 fY, BOOL bActiveItem, f32 fScale, BOOL bSelected );
static void _ResetSystem( void );
static BOOL _IsCardStillAttached( FStorage_DeviceID_e nStorageDeviceID,
								cwchar *pwszCardName,
								BOOL bCheckCardIsReadyForUse = TRUE );
static BOOL _DoesNameExistOnCard( FStorage_DeviceID_e nStorageDeviceID,
                                cwchar *pwszProfileName );
static BOOL _CreateUniqueName( FStorage_DeviceID_e nStorageDeviceID,
							  wchar *pwszNewName );
static s32 _FindMemCardProfileIndexFromName( const FStorage_DeviceInfo_t *pDevInfo, wchar *pwszProfileName );
static BOOL _HandleAxisSelections( BOOL bLeftRight, u32 nControllerIndex, s16 &rnCurrent, u32 nMaxValidIndex,
								BOOL bWrapAround, FSndFx_FxHandle_t hSfxToPlayOnChange );
static BOOL _HandleRuleAxisSelections( BOOL bLeftRight, u32 nControllerIndex, s8 &rnCurrent, u32 nMaxValidIndex,
								  BOOL bWrapAround, FSndFx_FxHandle_t hSfxToPlayOnChange );
static BOOL _IsSpaceAvailForProfile( const FStorage_DeviceInfo_t *pDevInfo );
static s32 _BumpNumber( s32 nNumberIn, s32 nSmallIncr, s32 nThreshold, s32 nBigIncr );
static BOOL _RuleDisabled( s16 nCurrent );
static void _ME_TransitionToNew( s32 nIndex );
static void _ME_ResetKeyboard( void );
static void _MP_FillUniqueProfiles( void );
static BOOL _MP_IsEqual( CPlayerProfile* p1, CPlayerProfile* p2 );

static void _IG_StartSave( void );
static BOOL _IG_SaveGame_Work( void );
static void _ShortenString( s32 uMaxLen, wchar *pwszDest, cwchar *pwszSource );
static void _SafeCopyString( wchar *pwszDst, cwchar *pwszSrc, s32 nDstSize, s32 nLimitAtThisLen=0 );
FINLINE void _SafeCopyFStorageName( wchar *pwszDst, cwchar *pwszSrc, s32 nLimitAtThisLen=0 ) {
	_SafeCopyString( pwszDst, pwszSrc, FSTORAGE_MAX_NAME_LEN, nLimitAtThisLen );
}
static void _TryToReadProfileAndSetupProperResponseState( BOOL bMakeRetryTheDefaultOptionIfError );
static void _SetupLaunchMenu( void );
static void _IGSaveDisplaySelectMUMsgBox( BOOL bForceStatusUpdate );

///////////////////////////////////////////////////////////////////////////////////////////////////
// GENERIC SCREEN FUNCTIONS THAT CAN BE USED BY ANY SCREEN INSTEAD OF IMPLEMENTING A CUSTOM ROUTINE
///////////////////////////////////////////////////////////////////////////////////////////////////
static void _GenericDrawOrtho_NoSelections( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _GenericDrawOrtho_Selections( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _GenericDrawOrtho_NoSelectionsWithString( Wpr_DataTypes_ScreenData_t *pScreen, 
													 f32 fStringUnitY, cwchar *pwszString, 
													 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static Wpr_DataTypes_NavCode_e _GenericButtonWork( BOOL bCheckA, BOOL bCheckB, u32 nButtonMask );
static void _FillInStorageDeviceName( u32 nDeviceID, wchar *pwszTarget );													 

/////////////////////////
// 'main menu' functions
/////////////////////////
static Wpr_DataTypes_NavCode_e _MainMenu_Work( void );
static void _MainMenu_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MainMenu_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
////////////////////////////
// 'sound setting' functions
////////////////////////////
static Wpr_DataTypes_NavCode_e _SoundSettings_Work( void );
static void _SoundSettings_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _SoundSettings_ProfileToWorkingVars();
static void _SoundSettings_WorkingVarsToProfile();
static void _SoundSettings_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
///////////////////////////////////////
// DR - 'don't remove mu' warning functions
///////////////////////////////////////
static Wpr_DataTypes_NavCode_e _RemoveWarning_Work( void );
static void _RemoveWarning_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _RemoveWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static void _SetupMemCardSave( Wpr_DataTypes_Screens_e nScreenToReturn, CPlayerProfile *pProfile );
//////////////////////////////////////
// 'already exists' warning functions
//////////////////////////////////////
static Wpr_DataTypes_NavCode_e _AlreadyExistsWarning_Work( void );
static void _AlreadyExistsWarning_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _AlreadyExistsWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
///////////////////////////////////////////
// 'delete profile' confirmation functions
///////////////////////////////////////////
static Wpr_DataTypes_NavCode_e _DeleteProfile_Work( void );
static void _DeleteProfile_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _DeleteProfile_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
/////////////////////////////////
// AS - 'advanced settings' functions
/////////////////////////////////
static Wpr_DataTypes_NavCode_e _AdvSettings_Work( void );
static void _AdvSettings_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _AdvSettings_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _AdvSettings_ProfileToWorkingVars();
static void _AdvSettings_WorkingVarsToProfile();
static void _AdvSettings_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
////////////////////////////////
// PS - 'profile settings' functions
////////////////////////////////
static Wpr_DataTypes_NavCode_e _ProfileSettings_Work( void );
static void _ProfileSettings_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _ProfileSettings_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
////////////////////////////
// PN - 'profile name' functions
////////////////////////////
static Wpr_DataTypes_NavCode_e _ProfileName_Work( void );
static void _ProfileName_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _ProfileName_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static BOOL _ProfileName_IsNameValid( cwchar *pwszProfileName );
//////////////////////////////////
// 'start over' warning functions
//////////////////////////////////
static Wpr_DataTypes_NavCode_e _StartOverWarning_Work( void );
static void _StartOverWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
///////////////////////////////
// 'no save' warning functions
///////////////////////////////
static Wpr_DataTypes_NavCode_e _NoSaveWarning_Work( void );
static void _NoSaveWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
//////////////////////////
// LM - 'launch menu' functions
//////////////////////////
static Wpr_DataTypes_NavCode_e _LaunchMenu_Work( void );
static void _LaunchMenu_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _LaunchMenu_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _LaunchMenu_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static _LaunchCode_e _HandleUnlockLevelsCode( _LaunchCode_e nCurState, u32 nControllerID );
//////////////////////////
// PL - 'pick level' functions
//////////////////////////
static Wpr_DataTypes_NavCode_e _PickLevel_Work( void );
static void _PickLevel_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _PickLevel_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _PickLevel_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
//////////////////////
// CC - 'controller config'
//////////////////////
static Wpr_DataTypes_NavCode_e _ControllerConfig_Work( void );
static void _ControllerConfig_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _ControllerConfig_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _ControllerConfig_ProfileToWorkingVars();
static void _ControllerConfig_WorkingVarsToProfile();
static void _ControllerConfig_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static Wpr_DataTypes_ControllerTextPos_t *_FindControllerTextPos( cchar *pszKey, u32 nNumInList, Wpr_DataTypes_ControllerTextPos_t *paList );
///////////////
// 'select mu'
///////////////
static Wpr_DataTypes_NavCode_e _SelectMU_Work( void );
static void _SelectMU_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _SelectMU_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _SelectMU_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static Wpr_DataTypes_MUSelectEntry_t *_InitMUSelectVars( u16 &rnNumElements );
static void _MUSelectUpdate( Wpr_DataTypes_MUSelectEntry_t *paSelectEntries, BOOL bForceUpdate );
static u32 _MUSelectPickDefault( Wpr_DataTypes_MUSelectEntry_t *paSelectEntries );
///////////////////
// 'select profile'
///////////////////
static Wpr_DataTypes_NavCode_e _SelectProfile_Work( void );
static void _SelectProfile_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _SelectProfile_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _SelectProfile_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static void _SelectProfile_SetupReturnVisit( const FStorage_DeviceInfo_t *pDevInfo, wchar *pwszProfileNameToMakeActive );
//////////////////////
// PC - 'pick color'
//////////////////////
static Wpr_DataTypes_NavCode_e _PickColor_Work( void );
static void _PickColor_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _PickColor_ProfileToWorkingVars();
static void _PickColor_WorkingVarsToProfile();
static void _PickColor_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
////////////////////
// MJ - 'multi join'
////////////////////
static Wpr_DataTypes_NavCode_e _MultiJoin_Work( void );
static void _MultiJoin_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiJoin_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static void _MJ_InitJoinInfoAndSections( _JoinInfo_t *pJoinInfo, 
										_MultiJoin_SectionData_t *paSections,
										u32 nNumPlayers );
static void _MJ_UpdateJoinInfo( _JoinInfo_t *pJoinInfo, BOOL bForceUpdate );
static Wpr_DataTypes_NavCode_e _MJ_SectionWork_EnterState( const _JoinInfo_t *pJoinInfo,
														  _MultiJoin_SectionData_t *pSection );
static Wpr_DataTypes_NavCode_e _MJ_SectionWork_SelectState( const _JoinInfo_t *pJoinInfo,
														   _MultiJoin_SectionData_t *pSection );
static Wpr_DataTypes_NavCode_e _MJ_SectionWork_WaitState( const _JoinInfo_t *pJoinInfo,
														 _MultiJoin_SectionData_t *pSection );
static void _MJ_UpdateSaveInfo( const _JoinInfo_t *pJoinInfo, 
							   _MultiJoin_SectionData_t *pSection,
							   BOOL bAllowChanges );
static BOOL _MJ_ConvertProfileIndexToProfileInfo( const _JoinInfo_t *pJoinInfo,
												 u32 nDeviceIndex, u32 nProfileIndex,
												 FStorage_ProfileInfo_t *pProfileInfo,
												 const FStorage_DeviceInfo_t **ppDeviceInfo );
/////////////////////////
// MT - 'multi game type'
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiType_Work( void );
static void _MultiType_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static s32 _MT_CalcTotalTypes( void );
static cwchar* _MT_GetTypeName( s32 nType );
static CPlayerProfile* _MT_GetProfForType( s32 nType, s32* pnRulesIndex );
static s32 _MT_GetProfIndexForType( s32 nType, s32* pnRulesIndex );
static void _MT_PrepareEditRulesFromType( void );
static void _MT_PrepareEditFromType( void );
static void _MT_InitRulesFromType( GameSave_MPRules_t* pRules, s32 nType, BOOL bPreserveName, BOOL bPreserveBaseType );

/////////////////////////
// ME - 'multi game edit types'
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiEditType_Work( void );
static void _MultiEditType_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiEditType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

/////////////////////////
// New multiplayer game type
/////////////////////////
static void _MultiNewType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

/////////////////////////
// Rules are full
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiNosaveType_Work( void );
static void _MultiNosaveType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

/////////////////////////
// Confirm delete of rules
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiDelType_Work( void );
static void _MultiDelType_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiDelType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

/////////////////////////
// Duplicate rule set name warning
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiDupWarn_Work( void );
static void _MultiDupWarn_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiDupWarn_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

//////////////////////////
// MR - 'multi game rules'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiRules_Work( void );
static void _MultiRules_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiRules_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
///////////////////////////
// MP - 'multi pick teams'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiTeam_Work( void );
static void _MultiTeam_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiTeam_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static void _MultiTeam_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
///////////////////////////
// ML - 'multi pick level'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiLevel_Work( void );
static void _MultiLevel_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _MultiLevel_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static void _MultiLevel_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static Wpr_DataTypes_NavCode_e _PrepareToLoad_Work( void );
static void _PrepareToLoad_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );

///////////////////////////////
// EL - 'error loading' warning functions
///////////////////////////////
static Wpr_DataTypes_NavCode_e _ErrorLoading_Work( void );
static void _ErrorLoading_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _ErrorLoading_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

///////////////////////////////
// 'warning hdspace' warning functions
///////////////////////////////

static Wpr_DataTypes_NavCode_e _WarningHDSpace_Work( void );
static void _WarningHDSpace_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _WarningHDSpace_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

/////////////////////////////////////
// 'warning format card' functions
/////////////////////////////////////
static Wpr_DataTypes_NavCode_e _WarningFormatCard_Work( void );
static void _WarningFormatCard_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );
static void _WarningFormatCard_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );

static Wpr_DataTypes_NavCode_e _Error_Create_Work( void );					
static void _Error_Create_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );		
////////////////////////////////////
// WPR_DATATYPES_SCREENS_ERROR_RENAM
static Wpr_DataTypes_NavCode_e _Error_Rename_Work( void );					
static void _Error_Rename_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );		
////////////////////////////////////
// WPR_DATATYPES_SCREENS_ERROR_RENAM
static Wpr_DataTypes_NavCode_e _Error_RenameSpace_Work( void );			
static void _Error_RenameSpace_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

//////////////////////////////////////////
// WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL
static Wpr_DataTypes_NavCode_e _DifficultyLevel_Work( void );			
static void _DifficultyLevel_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
static void _DifficultyLevel_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );

//////////////////////////////////////////
// WPR_DATATYPES_SCREENS_CONFIRM_FORMAT
static Wpr_DataTypes_NavCode_e _ConfirmFormat_Work( void );
static void _ConfirmFormat_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode );


// put each screen's functions into this table so that they get called
static const Wpr_DataTypes_ScreenFunctions_t _aScreenFunctions[WPR_DATATYPES_SCREEN_COUNT] = {
	//////////////////////////////////////
	// WPR_DATATYPES_SCREENS_NONE
	_PrepareToLoad_Work,				// Wpr_DataTypes_WorkFcn
	NULL,								// Wpr_DataTypes_DrawOrthoFcn
	_PrepareToLoad_DrawFDraw,			// Wpr_DataTypes_DrawFDrawFcn
    NULL,								// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////
	// WPR_DATATYPES_SCREENS_SOUND_OPTIONS
	_SoundSettings_Work,				// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,		// Wpr_DataTypes_DrawOrthoFcn
	_SoundSettings_DrawFDraw,			// Wpr_DataTypes_DrawFDrawFcn
    _SoundSettings_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_PROFILE_SETTINGS
	_ProfileSettings_Work,				// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,		// Wpr_DataTypes_DrawOrthoFcn
	_ProfileSettings_DrawFDraw,			// Wpr_DataTypes_DrawFDrawFcn
	_ProfileSettings_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS
	_ControllerConfig_Work,				// Wpr_DataTypes_WorkFcn
	_ControllerConfig_DrawOrtho,		// Wpr_DataTypes_DrawOrthoFcn
	_ControllerConfig_DrawFDraw,		// Wpr_DataTypes_DrawFDrawFcn
	_ControllerConfig_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS
	_AdvSettings_Work,					// Wpr_DataTypes_WorkFcn
	_AdvSettings_DrawOrtho,				// _DrawOrthoFcn
	_AdvSettings_DrawFDraw,				// _DrawFDrawFcn
	_AdvSettings_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////
	// WPR_DATATYPES_SCREENS_MAIN_MENU
	_MainMenu_Work,						// Wpr_DataTypes_WorkFcn
	_MainMenu_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	NULL,								// Wpr_DataTypes_DrawFDrawFcn
	_MainMenu_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT
	_SelectMU_Work,						// Wpr_DataTypes_WorkFcn
	_SelectMU_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	_SelectMU_DrawFDraw,				// Wpr_DataTypes_DrawFDrawFcn
	_SelectMU_SP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_PROFILE_SELECT
	_SelectProfile_Work,				// Wpr_DataTypes_WorkFcn
	_SelectProfile_DrawOrtho,			// Wpr_DataTypes_DrawOrthoFcn
	_SelectProfile_DrawFDraw,			// Wpr_DataTypes_DrawFDrawFcn
    _SelectProfile_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////
	// WPR_DATATYPES_SCREENS_DELETE_PROFILE
	_DeleteProfile_Work,				// Wpr_DataTypes_WorkFcn
	_DeleteProfile_DrawOrtho,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,								// Wpr_DataTypes_DrawFDrawFcn
	_DeleteProfile_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_NO_SAVE_WARNING
	_NoSaveWarning_Work,				// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,		// Wpr_DataTypes_DrawOrthoFcn
	NULL,								// Wpr_DataTypes_DrawFDrawFcn
	_NoSaveWarning_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_ALREADY_EXISTS_WARNING
	_AlreadyExistsWarning_Work,			// Wpr_DataTypes_WorkFcn
	_AlreadyExistsWarning_DrawOrtho,	// Wpr_DataTypes_DrawOrthoFcn
	NULL,								// Wpr_DataTypes_DrawFDrawFcn
	_AlreadyExistsWarning_SP_ExitDecisions,// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME
	_ProfileName_Work,					// Wpr_DataTypes_WorkFcn
	_ProfileName_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	NULL,								// Wpr_DataTypes_DrawFDrawFcn
	_ProfileName_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////////
	// WPR_DATATYPES_SCREENS_PICK_LEVEL
	_PickLevel_Work,					// Wpr_DataTypes_WorkFcn
	_PickLevel_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	_PickLevel_DrawFDraw,				// Wpr_DataTypes_DrawFDrawFcn
	_PickLevel_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////////////////////
    // WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING
	_RemoveWarning_Work,				// Wpr_DataTypes_WorkFcn
	_RemoveWarning_DrawOrtho,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,								// Wpr_DataTypes_DrawFDrawFcn
	_RemoveWarning_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////
	// WPR_DATATYPES_SCREENS_LAUNCH
	_LaunchMenu_Work,						// Wpr_DataTypes_WorkFcn
	_LaunchMenu_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	_LaunchMenu_DrawFDraw,					// Wpr_DataTypes_DrawFDrawFcn
	_LaunchMenu_SP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	////////////////////////////////////
	// WPR_DATATYPES_SCREENS_START_OVER
	_StartOverWarning_Work,					// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_NoSelections,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_StartOverWarning_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////
	// WPR_DATATYPES_SCREENS_PICK_COLOR
	_PickColor_Work,						// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	_PickColor_DrawFDraw,					// Wpr_DataTypes_DrawFDrawFcn
    _PickColor_SP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_JOIN
	_MultiJoin_Work,						// Wpr_DataTypes_WorkFcn
	_MultiJoin_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiJoin_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_TYPE
	_MultiType_Work,						// Wpr_DataTypes_WorkFcn
	_MultiType_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiType_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES
	_MultiEditType_Work,					// Wpr_DataTypes_WorkFcn
	_MultiEditType_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiEditType_MP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_NEW_TYPE
	_ProfileName_Work,						// Wpr_DataTypes_WorkFcn
	_ProfileName_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiNewType_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_EDIT_NO_SAVE
	_MultiNosaveType_Work,					// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_NoSelections,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiNosaveType_MP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_EDIT_CONFIRM_DEL
	_MultiDelType_Work,						// Wpr_DataTypes_WorkFcn
	_MultiDelType_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiDelType_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_DUPLICATE
	_MultiDupWarn_Work,						// Wpr_DataTypes_WorkFcn
	_MultiDupWarn_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiDupWarn_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_RULES
	_MultiRules_Work,						// Wpr_DataTypes_WorkFcn
	_MultiRules_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_MultiRules_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_TEAM
	_MultiTeam_Work,						// Wpr_DataTypes_WorkFcn
	_MultiTeam_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	_MultiTeam_DrawFDraw,					// Wpr_DataTypes_DrawFDrawFcn
	_MultiTeam_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	//////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_MULTI_LEVEL
	_MultiLevel_Work,						// Wpr_DataTypes_WorkFcn
	_MultiLevel_DrawOrtho,					// Wpr_DataTypes_DrawOrthoFcn
	_MultiLevel_DrawFDraw,					// Wpr_DataTypes_DrawFDrawFcn
	_MultiLevel_MP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_ERROR_LOADING
	_ErrorLoading_Work,						// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	_ErrorLoading_DrawFDraw,				// Wpr_DataTypes_DrawFDrawFcn
	_ErrorLoading_SP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_WARNING_HDSPACE
	_WarningHDSpace_Work,					// Wpr_DataTypes_WorkFcn
	_WarningHDSpace_DrawOrtho,				// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_WarningHDSpace_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_WARNING_FORMATCARD
	_WarningFormatCard_Work,				// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	_WarningFormatCard_DrawFDraw,			// Wpr_DataTypes_DrawFDrawFcn
	_WarningFormatCard_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_ERROR_CREATE,		
	_Error_Create_Work,						// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_Error_Create_SP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_ERROR_RENAME,		
	_Error_Rename_Work,						// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_Error_Rename_SP_ExitDecisions,			// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_ERROR_RENAME_SPACE,
	_Error_RenameSpace_Work,				// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_Error_RenameSpace_SP_ExitDecisions,	// Wpr_DataTypes_DecisionsFcn
	/////////////////////////////////////////
	// WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL
	_DifficultyLevel_Work,					// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_Selections,			// Wpr_DataTypes_DrawOrthoFcn
	_DifficultyLevel_DrawFDraw,				// Wpr_DataTypes_DrawFDrawFcn
	_DifficultyLevel_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
	///////////////////////////////////////
	// WPR_DATATYPES_SCREENS_CONFIRM_FORMAT
	_ConfirmFormat_Work,					// Wpr_DataTypes_WorkFcn
	_GenericDrawOrtho_NoSelections,			// Wpr_DataTypes_DrawOrthoFcn
	NULL,									// Wpr_DataTypes_DrawFDrawFcn
	_ConfirmFormat_SP_ExitDecisions,		// Wpr_DataTypes_DecisionsFcn
};


//////////////////////////////////////////////////////////////
// IN GAME SAVE LOGIC

#define _IGSAVE_DELAY_SECONDS		( 1 )
#define _IGSAVE_START_DELAY_SECONDS	( 0.1f )

typedef enum {
	_IGSAVE_NONE = 0,
	_IGSAVE_SAVE,
	_IGSAVE_DELAY,
	_IGSAVE_INSERT_CARD,
	_IGSAVE_SELECTMU,
	_IGSAVE_WARN_OVERWRITE,
	_IGSAVE_ERROR,
	_IGSAVE_WARNNOSAVE,
	_IGSAVE_CARDNEEDSFORMATTING,
	_IGSAVE_WARN_FORMAT,
	
} _IG_SaveState_e;

typedef enum {
	_IGSAVESTATUS_SAVE		= 0x01,
	_IGSAVESTATUS_SAVED		= 0x02,
	_IGSAVESTATUS_ROAMING	= 0x04,
};


static u8 _IGSaveState = _IGSAVE_NONE;// see _IG_SaveState_e...
static u8 _IGSaveErrorState	= _IGSAVE_NONE;// see _IG_SaveState_e...
static u8 _uIGErrorPlayer;
static u8 _auIGPlayerSaveStatus[MAX_PLAYERS];
static CPlayerProfile *_apIGSaveProfiles[MAX_PLAYERS];
static u64 _uSaveTimer;

///////////////////////////


//=================
// public functions

BOOL wpr_system_InitSystem( void ) {
	u32 i;

	FASSERT( !_paProfiles );

	// allocate the player profiles that will be used throughout the life of the application
	_paProfiles = (CPlayerProfile *)fres_AlignedAllocAndZero( sizeof( CPlayerProfile ) * MAX_PLAYERS, 4 );
	if( !_paProfiles ) {
		DEVPRINTF( "wpr_system_InitSystem : Could not allocate memory for profiles, no wrappers are possible.\n" );
	}
	// init the data in the profiles
	for( i=0; i < MAX_PLAYERS; i++ ) {
		_paProfiles[i].InitData();
	}
	
	_ResetSystem();

	return TRUE;
}

// clears a frame if inside the wrapper system, should be the 1st Uninit called, so last Init in the list
void wpr_system_UninitSystem( void ) {

	if( _paProfiles ) {
		if( _bSystemOK ) {
			wpr_system_End();
		}

        _paProfiles = NULL;
	}
}

BOOL wpr_system_AreWrappersReadyToUse( void ) {
	return _bSystemOK;
}

BOOL wpr_system_InitInGame( void ) {
	_bInGame = TRUE;

//	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS;
//	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS;
	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SOUND_OPTIONS;

	_IGSaveState = _IGSAVE_NONE;
	_IGSaveErrorState = _IGSAVE_NONE;


	return _Init();
}

void wpr_system_Start( BOOL bBootup ) {
	
	FASSERT( _paProfiles );

	_bInGame = FALSE;
	gameloop_SetLoopHandlers( _Work, _Draw, _Init );

	_bBootup = bBootup;
}

void wpr_system_End( void ) {
	BOOL bWrappers = !_bInGame;

	FASSERT( _paProfiles );

	if( _bSystemOK ) {
		if( bWrappers ) {
			fmovie2_Unload();
		}

		fdelete_array( _paMeshInsts );
		_paMeshInsts = NULL;
		fdelete_array( _paTexInsts );
		_paTexInsts = NULL;
		fdelete_array( _MenuState.PLSelectInfo.paTexInsts );
		_MenuState.PLSelectInfo.paTexInsts = NULL;
		fdelete_array( _MenuState.MLLevelSelectInfo.paTexInsts );
		_MenuState.MLLevelSelectInfo.paTexInsts = NULL;

		if( bWrappers ) {
			if( _pSkyBox ) {
				_pSkyBox->Destroy();
				fdelete( _pSkyBox );
				_pSkyBox = NULL;
			}
			if( _pAudioStream ) {
				_pAudioStream->Destroy();
				_pAudioStream = NULL;
			}
		}

		_pCamAnimInst = NULL;
		
		_ResetSystem();

		if( bWrappers ) {
			gamecam_PostLevelCleanup();

			FDS_StreamMgr.Uninit();

			eparticlepool_UninitLevel();
			CGColl::ClearMaterialTable();
	
			fres_ReleaseFrame( _ResFrame );

			fparticle_KillAllEmitters();

			floop_EnableGovernor( _bPrevGovernorState );
		}
	}
}

const GameInitInfo_t *wpr_system_GetInitInfo( void ) {

	FASSERT( _bSystemOK );
	FASSERT( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_NONE );
	
	return &_GameInitInfo;
}

// resets the vars to put us at the main menu
void wpr_system_ResetToStartupScreen( BOOL bBootup ) {
	CFQuatA Quat;
	CFVec3 Pos;

	if( !_bSystemOK ) {
		return;
	}

	//////////////////////
	// init the 3d camera
	_pCamAnimInst->UpdateUnitTime( 0.71f );
	_pCamAnimInst->ComputeFrame( _CamInfo.m_fHalfFOV, Quat, Pos );
	Quat.BuildMtx( _CamMtx );
	_CamMtx.m_vPos.Set( Pos );
	
	_CamInfo.m_pmtxMtx = &_CamMtx;
	
	// init the gamecam system
	gamecam_InitLevelCameras();
	gamecam_SwitchPlayerToManualCamera( GAME_CAM_PLAYER_1, &_CamInfo );
	gamecam_SetActiveCamera( GAME_CAM_PLAYER_1 );

	//////////////////////////
	// init the player profile
	_paProfiles[0].InitNewProfile( FALSE );

	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MAIN_MENU;
	_MenuState.nCurItemIndex = _MENU_ITEMS_MM_SINGLE;
	_paProfiles[0].m_SaveInfo.nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
	
	if( bBootup ) {
		// during bootup, go to the VUG screen
		_MenuState.nMode = WPR_DATATYPES_MODES_VUG_LOGO;
	} else {
		_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;
		_MenuState.bFadeScreen = TRUE;
	}
	_MenuState.fModeTimer = 0.0f;

	fmovie2_Unload();
	if( bBootup ) {
		fmovie2_Play( _pszVUGLogoMovie );	
	}
}

// returns the controller id of an active controller, one that the stick or Accept button is pressed
// returns WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN if none could be found
s32 wpr_system_FindActiveControllerPort( BOOL bTestAcceptButton, BOOL bTestUDStick, BOOL bTestLRStick ) {
	u32 i;
#if defined(MA_PC_INPUT)
	i = pcinput_KeyboardPort();
	if( i < GAMEPAD_MAX_PORT_COUNT && _MousePort( i ) &&
		( (bTestAcceptButton && _nMouseClickFrames) || ((bTestUDStick || bTestLRStick) && _bMouseMoved && _nMouseHoverItem >= 0) ) ) {
		return i;
	}
#endif
	
	if( bTestAcceptButton ) {
		for( i=0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) {
			if( _CheckAcceptButtons( i ) ) {
				return i;
			}
		}		
	}
	
	for( i=0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) {
		if( bTestUDStick ) {
			if( _CheckUpDownAxis( i ) != _NOT_UP_OR_DOWN ) {
				return i;
			}
		}
		if( bTestLRStick ) {
			if( _CheckLeftRightAxis( i ) != _NOT_LEFT_OR_RIGHT ) {
				return i;
			}
		}
	}
	return WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
}

// if nSelectedIndex is less than 0, then all text will be drawn NOT SELECTED
void wpr_system_DrawBasicScreen( Wpr_DataTypes_ScreenData_t *pScreen, s32 nSelectedIndex, BOOL bDrawUnselectedAsThis,
								f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
		
	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {	
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	
	// draw the text
	BOOL bSelected;
	for( i=0; i < pScreen->nNumTextElements; i++ ) {
		if( !fclib_wcsicmp( pScreen->pText[i].pwszText, L"None" ) ) {
			continue;
		}

		if( (s32)i != nSelectedIndex ) {
            bSelected = bDrawUnselectedAsThis;
		} else {
			bSelected = TRUE;
		}

		_DrawText( &pScreen->pText[i], bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes );
#if defined(MA_PC_INPUT)
		if( nSelectedIndex >= 0 && Wpr_DataTypes_paScreenData && pScreen == &Wpr_DataTypes_paScreenData[_MenuState.nCurrentScreen] &&
			(pScreen->pText[i].nType == WPR_DATATYPES_REGULAR || pScreen->pText[i].nType == WPR_DATATYPES_KEYBOARD) ) {
			_MouseAddItem( (s32)i - nSelectedIndex + _MenuState.nCurItemIndex, _MouseBasicScreenItemFlags() );
		}
#endif
	}
}

// configures a simple single player game that doesn't have a real profile 
// (used for the e3 menus without requiring a ton extra memory)
const GameInitInfo_t *wpr_system_CreateSimpleSinglePlayerGameInit( BOOL bInvertControls ) {
	FASSERT( _paProfiles );

	fang_MemZero( &_GameInitInfo, sizeof( GameInitInfo_t ) );

	_GameInitInfo.apProfile[0] = &_paProfiles[0];
	_paProfiles[0].InitData();
	if( bInvertControls ) {
        _paProfiles[0].m_Data.nFlags |= ( GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE | GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG );
	} else {
		_paProfiles[0].m_Data.nFlags |= ( GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE );
	}
	_GameInitInfo.nNumPlayers = 1;
	_GameInitInfo.bSinglePlayer = TRUE;
	_GameInitInfo.bNewGame = TRUE;
	_GameInitInfo.nLevelToPlay = 0;	

	return &_GameInitInfo;
}

const GameInitInfo_t *wpr_system_CreateSimpleMultiPlayerGameInit( u32 nNumPlayers, u32 nPlayerMask, u32 nLevelToPlay ) {
	u32 i, nMask;

	FASSERT( _paProfiles );

	fang_MemZero( &_GameInitInfo, sizeof( GameInitInfo_t ) );

	_GameInitInfo.bSinglePlayer = FALSE;
	_GameInitInfo.nLevelToPlay = nLevelToPlay;
	CPlayerProfile::SetMultiPlayerRulesToDefault( &_MenuState.MPRules, _apwszPhrases[WPR_DATATYPES_PHRASES_DEFAULT_GAMETYPE_NAME] );
	_GameInitInfo.pMultiplayerRules = &_MenuState.MPRules;
	_GameInitInfo.bTeamPlay = FALSE;
	
	_GameInitInfo.nNumPlayers = 0;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		nMask = 1 << i;
		if( nPlayerMask & nMask ) {
			// player i is in the game
			_GameInitInfo.apProfile[_GameInitInfo.nNumPlayers] = &_paProfiles[i];
			_paProfiles[i].InitData();
			_paProfiles[i].m_Data.nFlags |= ( GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE );
			
			// increase the number of players
			_GameInitInfo.nNumPlayers++;
		}
	}

	FASSERT( nNumPlayers == _GameInitInfo.nNumPlayers );

	return &_GameInitInfo;
}

BOOL wpr_system_InitControllerConfigData( FGameDataFileHandle_t hFile, WprSystem_ControllerConfigData_t *pData, CFStringTable &rStringTable ) {
	u32 i, j, nCount, nIndex, nOffset;
	FGameDataTableHandle_t hTable, hConfigTable;
	cchar *pszTableName, *pszString;
	cwchar *pwszString;
	FGameData_VarType_e nDataType;
	Wpr_DataTypes_ControllerConfig_t *pConfig;
	f32 fValue;

	//////////////////////////////////////////////
	// Find the platform specific screen pos table
	pszTableName = _pszControllerScreenPosTableName;	
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
    if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_system_InitControllerConfigData() : Could not find the table named '%s'.\n", pszTableName );
		return FALSE;
	}

	// get the number of elements
	nCount = fgamedata_GetNumFields( hTable );
	if( ((nCount % 5) != 0) || nCount == 0 ) {
		DEVPRINTF( "wpr_system_InitControllerConfigData() : The table named '%s' does not have the proper number of elements.\n", pszTableName );
		return FALSE;
	}

	// allocate room to hold the screen pos data
	pData->nCCNumScreenPosInfos = nCount / 5;
	pData->paCCScreenPosInfos = (Wpr_DataTypes_ControllerTextPos_t *)fres_Alloc( sizeof( Wpr_DataTypes_ControllerTextPos_t ) * pData->nCCNumScreenPosInfos );
	if( !pData->paCCScreenPosInfos ) {
		DEVPRINTF( "wpr_system_InitControllerConfigData() : Could not allocate %d elements to hold the controller config text positions, out of memory.\n", pData->nCCNumScreenPosInfos );
		return FALSE;
	}

	// read in the screen pos table
	nIndex = 0;
	for( i=0; i < pData->nCCNumScreenPosInfos; i++ ) {
		// grab the key string and put it into the rStringTable
		pszString = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );
		pData->paCCScreenPosInfos[i].pszKeyString = rStringTable.AddString( pszString );

		// grab the x, y, and scale values
		pData->paCCScreenPosInfos[i].fUnitX = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
		pData->paCCScreenPosInfos[i].fUnitY = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 2, nDataType );
		pData->paCCScreenPosInfos[i].fScale = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 3, nDataType );
		fValue = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 4, nDataType );
		pData->paCCScreenPosInfos[i].nLineDrawMask = (u32)fValue;

		nIndex += 5;
	}

	////////////////////////////////////
	// Find the controller configs table
	pszTableName = _pszControllerConfigs;
	hConfigTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
    if( hConfigTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_system_InitControllerConfigData() : Could not find the table named '%s'.\n", pszTableName );
		return FALSE;
	}

	// get the number of elements
	nCount = fgamedata_GetNumFields( hConfigTable );
	if( nCount != (GAMEPAD_MAP_COUNT-1) ) {
		DEVPRINTF( "wpr_system_InitControllerConfigData() : The code only knows about %d controller configs, but there are %d listed in the table named '%s'.\n",
			(GAMEPAD_MAP_COUNT-1),
			nCount,
			pszTableName );
		return FALSE;
	}
	
	// allocate room to hold the configs
	pData->nCCNumConfigs = nCount;
	pData->paCCConfigs = (Wpr_DataTypes_ControllerConfig_t *)fres_Alloc( sizeof( Wpr_DataTypes_ControllerConfig_t ) * pData->nCCNumConfigs );
	if( !pData->paCCConfigs ) {
		DEVPRINTF( "wpr_system_InitControllerConfigData() : Could not allocate %d elements to hold the controller configs, out of memory.\n", pData->nCCNumConfigs );
		return FALSE;
	}

	for( i=0; i < pData->nCCNumConfigs; i++ ) {
		pConfig = &pData->paCCConfigs[i];

		// grab the table name
		pszTableName = (cchar *)fgamedata_GetPtrToFieldData( hConfigTable, i, nDataType );

		// find the listed table
		hTable = fgamedata_GetFirstTableHandle( hFile, pszTableName );
		if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			DEVPRINTF( "wpr_system_InitControllerConfigData() : Could not find the table named '%s'.\n", pszTableName );
			return FALSE;
		}

		// get the number of elements in this config table
		nCount = fgamedata_GetNumFields( hTable );
		if( ((nCount % 3) != 0) || nCount == 0 ) {
			DEVPRINTF( "wpr_system_InitControllerConfigData() : The table named '%s' does not have the proper number of elements.\n", pszTableName );
			return FALSE;
		}
        
		// allocate room to hold the elements in this table
		pConfig->nNumTextFields = nCount / 3;
		pConfig->paTextFields = (Wpr_DataTypes_ControllerText_t *)fres_Alloc( sizeof( Wpr_DataTypes_ControllerText_t ) * pConfig->nNumTextFields );
		if( !pConfig->paTextFields ) {
			DEVPRINTF( "wpr_system_InitControllerConfigData() : Could not allocate %d elements to hold the text fields of a controller config, out of memory.\n", pConfig->nNumTextFields );
			return FALSE;
		}
		pConfig->nLineDrawMask = WPR_DATATYPES_DRAW_LINE_NONE;

		// set the offset into the table data, depending on what platform we are on
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
		nOffset = 1;
#else
		nOffset = 2;
#endif

		// walk the entries of the table
		nIndex = 0;
		for( j=0; j < pConfig->nNumTextFields; j++ ) {
			// grab the display string
			pwszString = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );
			pConfig->paTextFields[j].pwszStringToDisplay = rStringTable.AddString( pwszString );

			// grab the platform specific key string
			pszString = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + nOffset, nDataType );

			// find the Text Pos ptr
			pConfig->paTextFields[j].pPosInfo = _FindControllerTextPos( pszString, pData->nCCNumScreenPosInfos, pData->paCCScreenPosInfos );
			if( !pConfig->paTextFields[j].pPosInfo ) {
				DEVPRINTF( "wpr_system_InitControllerConfigData() : The table '%ls' contains a keyword '%s' that isn't in the keyword table.\n",
					pszTableName,
					pszString );
				return FALSE;
			}

			pConfig->nLineDrawMask |= pConfig->paTextFields[j].pPosInfo->nLineDrawMask;

			nIndex += 3;
		}		
	}

	return TRUE;	
}

void wpr_system_ControllerConfig_DrawFDraw( Wpr_DataTypes_ControllerConfig_t *pConfig,
										   CFTexInst *pTexInst,
										   BOOL bDrawArrows,
										   f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	CFVec2 Lower, Upper;

	// draw the controller
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( pTexInst );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	
	f32 fHalfX = 0.5f * (_CONTROLLER_HEIGHT * fHalfYRes/fHalfXRes * (225.0f/182.0f) );
	f32 fHalfY = 0.5f * _CONTROLLER_HEIGHT;

	Lower.Set( (_CONTROLLER_X - fHalfX) * fHalfXRes,
			   (_CONTROLLER_Y - fHalfY) * fHalfYRes );		
	Upper.Set( (_CONTROLLER_X + fHalfX) * fHalfXRes,
			   (_CONTROLLER_Y + fHalfY) * fHalfYRes );
	
	// xbox texture coordinates
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	Wpr_DrawUtils_aVtx[0].ST.Set( 0.0f, (200.0f/255.0f) );
	Wpr_DrawUtils_aVtx[1].ST.Set( 0.0f, 0.0f );
	Wpr_DrawUtils_aVtx[2].ST.Set( (255.0f/255.0f), (200.0f/255.0f) );
	Wpr_DrawUtils_aVtx[3].ST.Set( (255.0f/255.0f), 0.0f );
#else
	Wpr_DrawUtils_aVtx[0].ST.Set( 0.0f, (178.0f/255.0f) );
	Wpr_DrawUtils_aVtx[1].ST.Set( 0.0f, 0.0f );
	Wpr_DrawUtils_aVtx[2].ST.Set( (255.0f/255.0f), (178.0f/255.0f) );
	Wpr_DrawUtils_aVtx[3].ST.Set( (255.0f/255.0f), 0.0f );
#endif

	CFColorRGBA Color;
	Color.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	
	Wpr_DrawUtils_aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 3.0f ); 
	Wpr_DrawUtils_aVtx[0].ColorRGBA = Color;
		
	Wpr_DrawUtils_aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 3.0f );						
	Wpr_DrawUtils_aVtx[1].ColorRGBA = Color;

	Wpr_DrawUtils_aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 3.0f );
	Wpr_DrawUtils_aVtx[2].ColorRGBA = Color;

	Wpr_DrawUtils_aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 3.0f );
	Wpr_DrawUtils_aVtx[3].ColorRGBA = Color;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, Wpr_DrawUtils_aVtx, 4 );

	// draw the lines
 	fdraw_SetTexture( NULL );

	//////////////////
	// xbox line draws

#if 0
	// draw a grid to ease the line placement
	u32 i;
	f32 fPercent = -1.0f;
	for( i=0; i < 20; i++ ) {
		fPercent += 0.1f;
		if( fPercent >= -0.01f && fPercent <= 0.01f ) {	
			Color.Set( 0.50f, 0.60f, 0.90f, 0.70f );
		} else {
			Color.Set( 0.35f, 0.35f, 0.35f, 0.50f );
		}
		wpr_drawutils_DrawThickLine( fPercent, -1.0f, fPercent, 1.0f, 2.0f, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );

		wpr_drawutils_DrawThickLine( -1.0f, fPercent, 1.0f, fPercent, 2.0f, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
#endif

#if WPR_DATATYPES_XBOX_GRAPHICS_ON

	Color.Set( 0.30f, 0.30f, 0.70f, 0.70f );

	// DPad
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_DPADY | WPR_DATATYPES_DRAW_LINE_DPADX) ) {
        wpr_drawutils_DrawThickLine( -0.41f, -0.26f, -0.13f, -0.15f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// left analog
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_LEFTY | WPR_DATATYPES_DRAW_LINE_LEFTX) ) {
		wpr_drawutils_DrawThickLine( -0.41f,  0.15f, -0.20f, 0.06f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// left trigger
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_LTRIGGER ) {
		wpr_drawutils_DrawThickLine( -0.15f,  0.20f, -0.20f, 0.31f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// right trigger
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_RTRIGGER ) {
		wpr_drawutils_DrawThickLine(  0.14f,  0.20f,  0.19f, 0.30f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the top button cluster
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_Y | WPR_DATATYPES_DRAW_LINE_B) ) {
		// line to the B button
		wpr_drawutils_DrawThickLine(  0.245f, 0.050f, 0.43f,  0.05f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
		// line to the Y button
		wpr_drawutils_DrawThickLine(  0.20f,  0.120f, 0.40f,  0.18f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the bottom button cluster
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_A | WPR_DATATYPES_DRAW_LINE_X) ) {
		// line to the X button
		wpr_drawutils_DrawThickLine(  0.145f,  0.03f, 0.43f, -0.05f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
		// line to the A button
		wpr_drawutils_DrawThickLine(  0.20f,  -0.05f, 0.40f, -0.15f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// right analog
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_RIGHTY | WPR_DATATYPES_DRAW_LINE_RIGHTX) ) {
		wpr_drawutils_DrawThickLine(  0.11f, -0.145f, 0.38f, -0.37f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the start button

	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_START ) {
		wpr_drawutils_DrawThickLine(  -0.4f, -0.06f, -0.21f, -0.11f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
#else
	Color.Set( 0.70f, 0.70f, 0.70f, 0.70f );

	// DPad
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_DPADY | WPR_DATATYPES_DRAW_LINE_DPADX) ) {
		wpr_drawutils_DrawThickLine( -0.41f, -0.26f, -0.15f, -0.25f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// left analog
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_LEFTY | WPR_DATATYPES_DRAW_LINE_LEFTX) ) {
		wpr_drawutils_DrawThickLine( -0.41f,  0.15f, -0.24f, -0.06f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// left trigger
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_LTRIGGER ) {
		wpr_drawutils_DrawThickLine( -0.17f,  0.19f, -0.17f, 0.33f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// right trigger
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_RTRIGGER ) {
		wpr_drawutils_DrawThickLine(  0.18f,  0.19f,  0.18f, 0.33f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the Z button
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_Z ) {
        wpr_drawutils_DrawThickLine(  0.25f,  0.12f, 0.48f,  0.32f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the Y button
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_Y ) {
		wpr_drawutils_DrawThickLine(  0.20f,  0.05f, 0.40f,  0.17f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the X button
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_X ) {
		wpr_drawutils_DrawThickLine(  0.25f,  -0.02f, 0.40f,  0.025f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the A button
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_A ) {
		wpr_drawutils_DrawThickLine(  0.23f,  -0.07f, 0.38f, -0.08f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the B button
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_B ) {
		wpr_drawutils_DrawThickLine(  0.14f,  -0.11f, 0.43f, -0.17f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// right analog
	if( pConfig->nLineDrawMask & (WPR_DATATYPES_DRAW_LINE_RIGHTY | WPR_DATATYPES_DRAW_LINE_RIGHTX) ) {
		wpr_drawutils_DrawThickLine(  0.15f, -0.25f, 0.41f, -0.38f,  _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	// the start button
	if( pConfig->nLineDrawMask & WPR_DATATYPES_DRAW_LINE_START ) {
		wpr_drawutils_DrawThickLine(  0.00f, -0.05f, 0.00f, -0.37f, _LINE_THICKNESS, &Color, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
#endif

	if( bDrawArrows ) {
		// draw left right arrows
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_SetTexture( NULL );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		wpr_drawutils_DrawLeftRightArrow( -0.11f,
							-0.02f,
							-0.46f,
							15.0f,
							fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
}

void wpr_system_ControllerConfig_DrawOrtho( Wpr_DataTypes_ScreenData_t *pScreen, Wpr_DataTypes_ControllerConfig_t *pConfig,
										   cwchar *pwszConfigString, u32 nCurrentConfigIndex,
                                           f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	Wpr_DataTypes_ControllerText_t *pText;
		
	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	// print "Configuration:   3"
	if( pwszConfigString ) {
		ftext_Printf( 0.16f, 0.55f,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls :     %ls%d", 
					WprDataTypes_pwszWhiteTextColor,
					L'L',
					1.0f, 
					pwszConfigString,
					WprDataTypes_pwszHiLightedBlink,
					nCurrentConfigIndex+1 );
	}

	// draw the button/axis labels
	for( i=0; i < pConfig->nNumTextFields; i++ ) {
		pText = &pConfig->paTextFields[i];

		ftext_Printf( pText->pPosInfo->fUnitX,
			pText->pPosInfo->fUnitY,
            L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
			WprDataTypes_pwszGrayTextColor,
			L'C',
			pText->pPosInfo->fScale,
			pText->pwszStringToDisplay );
	}
}

//===================
// private functions

// load everything that we need for the wrapper screens
static BOOL _Init( void ) {
	u32 i, nCount, nNameTableIndex, nNumElements, nPlatformOffset;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable, hTableNames;
#if FANG_WINGC
	FGameDataTableHandle_t hCommonPhrases = FGAMEDATA_INVALID_TABLE_HANDLE;
	u32 nNumCommonPhrases = 0;
#endif
	cchar *pszText, *pszTableName;
	cwchar *pwszText;
	FGameData_VarType_e nDataType;
	FMemFrame_t hMemFrame;
	Wpr_DataTypes_ScreenData_t *pScreen;
	f32 fLastY, fValue;
	Wpr_DataTypes_TextLayout_t *pTextLayout;
	cchar *pszCSVFilename;

	BOOL bWrappers = !_bInGame;

	pszCSVFilename = bWrappers ? _pszCSVFilename : _pszIGCSVFilename;

	FASSERT( _paProfiles );
	FASSERT( (WPR_DATATYPES_PHRASES_PRIMARY_LIMIT10 - WPR_DATATYPES_PHRASES_PRIMARY_LIMIT1 + 1) == GAMESAVE_PRIMARY_WEAPON_LIMIT_COUNT );
	FASSERT( (WPR_DATATYPES_PHRASES_SECONDARY_LIMIT8 - WPR_DATATYPES_PHRASES_SECONDARY_LIMIT1 + 1) == GAMESAVE_SECONDARY_WEAPON_LIMIT_COUNT );

	if( bWrappers ) {
		// make sure that the game is cleaned up before starting
		_bInGame = TRUE;// since game_Unload() now calls wpr_system_End(), we have to fool it to now free the memory frames
		game_UnloadLevel();
		_bInGame = FALSE;
	}

	// Set our wrappers loading callback
	if( bWrappers ) {
		fresload_SetProgressCallback( _WrappersLoadingCallback );
	}

	// reset all of our vars
	_ResetSystem();

	// grab a fres frame
	_ResFrame = fres_GetFrame();
	hMemFrame = fmem_GetFrame();

	#if !FANG_PRODUCTION_BUILD
		u32 nFreeBytes = fres_GetFreeBytes();
	#endif

	if( bWrappers ) {
		floop_Reset();
		DEVPRINTF( "******** LOAD MARKER - START OF WRAPPERS***********.\n" );
	}

	#if !GAMELOOP_EXTERNAL_DEMO
		if( bWrappers ) {
			// only start logging if this is the real wrappers, not the in game (which are already being logged)
			ffile_LogStart( "real wrappers" );
		}
	#endif

	//////////////////////////////////////
	// allocate memory for our screen data
	Wpr_DataTypes_paScreenData = (Wpr_DataTypes_ScreenData_t *)fres_Alloc( sizeof( Wpr_DataTypes_ScreenData_t ) * WPR_DATATYPES_SCREEN_COUNT );
	if( !Wpr_DataTypes_paScreenData ) {
		DEVPRINTF( "wpr_system::_Init() : Could not allocate memory to hold each screen's data.\n" );
		goto _EXIT_WITH_ERROR;
	}

	/////////////////////////////////
	// create the string table to use
	_pStringTable = CFStringTable::CreateTable( pszCSVFilename, TRUE );
	if( !_pStringTable ) {
		DEVPRINTF( "wpr_system::_Init() : Could not create a string table for all of the strings.\n" );
		goto _EXIT_WITH_ERROR;
	}

	if( bWrappers ) {
		CGColl::LoadMaterialTable( "mset_std" );
	}

	//////////////////////////////////////////////////
	// load the wrapper csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( pszCSVFilename );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "wpr_system::_Init() : Could not load the wrapper csv file '%s'.\n", pszCSVFilename );
		goto _EXIT_WITH_ERROR;
	}

	if( bWrappers ) {
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
			DEVPRINTF( "wpr_system::_Init() : Could not load the wrapper world file '%s'.\n", _pszLevelName );
			goto _EXIT_WITH_ERROR;
		}

		eparticlepool_InitLevel();

		////////////////////
		// create a viewport
		_pViewportOrtho3D = fviewport_Create();
		if( !_pViewportOrtho3D ) {
			DEVPRINTF( "wpr_system::_Init() : Could not create an rendering viewport.\n" );
			goto _EXIT_WITH_ERROR;
		}
		fviewport_InitOrtho3D( _pViewportOrtho3D, 0.1f, 100.0f );

		/////////////////////////////
		// create and init a sky box
		_pSkyBox = fnew CSkyBox;
		if( !_pSkyBox ) {
			DEVPRINTF( "wpr_system::_InitPickLevel() : Could not create a sky box.\n" );
			goto _EXIT_WITH_ERROR;
		}
		_pSkyBox->CreateFromCSV( hFile );
	}

	///////////////////////////////////////////////
	// allocate enough room for the needed textures
	_paTexInsts = wpr_datatypes_InitTextureArray( hFile, _pszTextureTableName, bWrappers ? WPR_DATATYPES_TEXTURES_COUNT : WPR_DATATYPES_IG_TEXTURES_COUNT );
	if( !_paTexInsts ) {
		goto _EXIT_WITH_ERROR;
	}
#if defined(MA_PC_INPUT)
	if( bWrappers ) {
		_MouseInstall();
	}
#endif

	/////////////////////////
	// find the phrases table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszPhrasesTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_system::_Init() : Could not find the phrases table named '%s'.\n", _pszPhrasesTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTable );
	#if FANG_WINGC
	hCommonPhrases = hTable;
	nNumCommonPhrases = i;
	if( i < WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT ) {
	#else
	if( i != WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT ) {
	#endif
		DEVPRINTF( "wpr_system::_Init() : The phrases table in '%s' didn't contain %d strings as expected.\n", _pszPhrasesTableName, WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT );
		goto _EXIT_WITH_ERROR;
	}
	#if FANG_WINGC
	if( i > WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT ) {
		DEVPRINTF( "wpr_system::_Init() : Ignoring %d trailing phrases in newer table '%s'.\n", i - WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT, _pszPhrasesTableName );
	}
	#endif
	// walk the string table adding each string to the string table and recording a ptr to them
	for( i=0; i < WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if( !fclib_wcscmp( pwszText, L"NULL_" ) ) {
			_apwszPhrases[i] = _pStringTable->AddString( L"" );
		} else {
			_apwszPhrases[i] = _pStringTable->AddString( pwszText );
		}
	}
	// now the platform specific table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszPlatformPhrasesTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_system::_Init() : Could not find the phrases table named '%s'.\n", _pszPlatformPhrasesTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTable );
	#if FANG_WINGC
	if( i < WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT ) {
	#else
	if( i != WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT ) {
	#endif
		DEVPRINTF( "wpr_system::_Init() : The phrases table in '%s' didn't contain %d strings as expected.\n", _pszPlatformPhrasesTableName, WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT );
		goto _EXIT_WITH_ERROR;
	}
	#if FANG_WINGC
	if( i > WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT ) {
		DEVPRINTF( "wpr_system::_Init() : Ignoring %d trailing phrases in newer table '%s'.\n", i - WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT, _pszPlatformPhrasesTableName );
	}
	#endif
	// walk the string table adding each string to the string table and recording a ptr to them
	for( i=0; i < WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT; i++ ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if( !fclib_wcscmp( pwszText, L"NULL_" ) ) {
			_apwszPhrases[WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT + i] = _pStringTable->AddString( L"" );
		} else {
			_apwszPhrases[WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT + i] = _pStringTable->AddString( pwszText );
		}
	}
	#if FANG_WINGC
	if( nNumCommonPhrases == _RETAIL_COMMON_PHRASE_COUNT && fgamedata_GetNumFields( hTable ) == _RETAIL_PLATFORM_PHRASE_COUNT ) {
		for( i=0; i < WPR_DATATYPES_PHRASES_COUNT; i++ ) {
			u32 nField = _anRetailPhraseField[i];
			if( nField == _RETAIL_PHRASE_LITERAL ) {
				pwszText = (i == WPR_DATATYPES_PHRASES_LOCKED) ? L"Locked" : L"Time to Beat";
			} else if( nField & 0x100 ) {
				pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nField & 0xFF, nDataType );
			} else {
				pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hCommonPhrases, nField, nDataType );
			}
			_apwszPhrases[i] = _pStringTable->AddString( fclib_wcscmp( pwszText, L"NULL_" ) ? pwszText : L"" );
		}
	}
	#endif

	/////////////////////////////////////////////
	// allocate enough room for the needed meshes
	_paMeshInsts = wpr_datatypes_InitMeshArray( hFile, _pszMeshTableName, _nNumMeshes );
	if( !_paMeshInsts ) {
		goto _EXIT_WITH_ERROR;
	}

	////////////////////////////////////
	// find the screen table names table
	hTableNames = fgamedata_GetFirstTableHandle( hFile, _pszScreenTableName );
	if( hTableNames == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_system::_Init() : Could not find the screen table names table named '%s'.\n", _pszScreenTableName );
		goto _EXIT_WITH_ERROR;
	}
	// get the number of fields in the table
	i = fgamedata_GetNumFields( hTableNames );
	#if FANG_WINGC
	if( i < (WPR_DATATYPES_SCREEN_COUNT*6) ) {
	#else
	if( i != (WPR_DATATYPES_SCREEN_COUNT*5) ) {
	#endif
		DEVPRINTF( "wpr_system::_Init() : The screen table names table contains %d entries, but %d are expected.\n", i, WPR_DATATYPES_SCREEN_COUNT );
		goto _EXIT_WITH_ERROR;
	}
	#if FANG_WINGC
	if( i > (WPR_DATATYPES_SCREEN_COUNT*6) ) {
		DEVPRINTF( "wpr_system::_Init() : Ignoring %d trailing screen table entries from newer screens.\n", i - (WPR_DATATYPES_SCREEN_COUNT*6) );
	}
	#endif

	//////////////////////////////////////
	// load the table data for each screen
	nNameTableIndex = 0;
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	nPlatformOffset = 0;
#else
	nPlatformOffset = 1;
#endif
	for( i=0; i < WPR_DATATYPES_SCREEN_COUNT; i++ ) {
		// grab a ptr to the screen data
		pScreen = &Wpr_DataTypes_paScreenData[i];

		///////////////////////////
		// grab the text table name
		pszTableName = (cchar *)fgamedata_GetPtrToFieldData( hTableNames, nNameTableIndex + nPlatformOffset, nDataType );
		nNameTableIndex += 2;
		if( fclib_stricmp( pszTableName, _pszNoTableUsed ) != 0 ) {
			// init the text layout array
			if( !wpr_datatypes_InitScreenTextLayoutArray( hFile, pszTableName, *pScreen, *_pStringTable ) ) { 
				// failed to init the text layout array
				goto _EXIT_WITH_ERROR;
			}
		} else {
			pScreen->nNumTextElements = 0;
			pScreen->pText = NULL;
		}

		///////////////////////////
		// grab the mesh table name
		pszTableName = (cchar *)fgamedata_GetPtrToFieldData( hTableNames, nNameTableIndex, nDataType );
	#if FANG_WINGC
		nNameTableIndex += 2;
	#else
		nNameTableIndex++;
	#endif
		if( fclib_stricmp( pszTableName, _pszNoTableUsed ) != 0 ) {
			// init the mesh layout array
			if( !wpr_datatypes_InitScreenMeshLayoutArray( hFile, pszTableName, *pScreen, _nNumMeshes, _paMeshInsts ) ) {
				// failed to init the mesh layout array
				goto _EXIT_WITH_ERROR;
			}			
		} else {
			pScreen->nNumMeshElements = 0;
			pScreen->pMesh = NULL;
		}

		/////////////////////////////
		// grab the button table name
		pszTableName = (cchar *)fgamedata_GetPtrToFieldData( hTableNames, nNameTableIndex + nPlatformOffset, nDataType );
		nNameTableIndex += 2;
		if( fclib_stricmp( pszTableName, _pszNoTableUsed ) != 0 ) {
			// init the button layout array
			if( bWrappers ) {
				if( !wpr_datatypes_InitScreenButtonLayoutArray( hFile, pszTableName, *pScreen, *_pStringTable,
																&_paTexInsts[WPR_DATATYPES_TEXTURES_A_BUTTON],
																&_paTexInsts[WPR_DATATYPES_TEXTURES_B_BUTTON],
																&_paTexInsts[WPR_DATATYPES_TEXTURES_Y_BUTTON],
																&_paTexInsts[WPR_DATATYPES_TEXTURES_X_BUTTON] ) ) { 
					// failed to init the text layout array
					goto _EXIT_WITH_ERROR;
				}
			} else {
				// there are no texture buttons for the ingame menus - just pass in NULL for all four button textures
				if( !wpr_datatypes_InitScreenButtonLayoutArray( hFile, pszTableName, *pScreen, *_pStringTable, NULL, NULL, NULL, NULL ) ) { 
					// failed to init the text layout array
					goto _EXIT_WITH_ERROR;
				}
			}
		} else {
			pScreen->nNumButtons = 0;
			pScreen->paButtons = NULL;
		}
	}


	// ME:  moved this out of wrappers
	///////////////////////////////////////
	// setup the memory unit select screen
	_MenuState.paMUEntries = _InitMUSelectVars( _MenuState.nMUNumEntries );
	if( !_MenuState.paMUEntries ) {
		DEVPRINTF( "wpr_system::_Init() : Could not init the Memory Unit Select screens.\n" );
		goto _EXIT_WITH_ERROR;
	}
	  
	if( bWrappers ) {
		/////////////////////////////////
		// setup the keyboard menu screen
		for( i=0; i < _MAX_ROWS_OF_KEYS; i++ ) {
			_MenuState.anPNColumnsPerRow[i] = 0;
		}	
		_MenuState.nPNNumRows = 0;
		_MenuState.bPNLower = TRUE;
		_MenuState.nPNCurCol = 0;
		_MenuState.nPNCurRow = 0;

		pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME];
		nNumElements = pScreen->nNumTextElements - _MENU_ITEMS_PN_START_OFFSET;
		// determine how many rows we have and how many cols are in each one
		for( i=0; i < nNumElements; i++ ) {
			pTextLayout = &pScreen->pText[i + _MENU_ITEMS_PN_START_OFFSET];
			
			if( i != 0 ) {
				if( fLastY != pTextLayout->fUnitY ) {
					// a new row starts here
					fLastY = pTextLayout->fUnitY;
					
					_MenuState.nPNNumRows++;
					_MenuState.anPNColumnsPerRow[_MenuState.nPNNumRows-1]++;
					
					if( _MenuState.nPNNumRows > (_MAX_ROWS_OF_KEYS-2) ) {
						DEVPRINTF( "wpr_system::_Init() : There are too many rows of text for the keyboard screen\n" );
						goto _EXIT_WITH_ERROR;
					}
				} else {
					_MenuState.anPNColumnsPerRow[_MenuState.nPNNumRows-1]++;
				}
			} else {
				fLastY = pTextLayout->fUnitY;
				_MenuState.nPNNumRows++;
				_MenuState.anPNColumnsPerRow[0]++;
			}
		}
		// add the last 2 rows of button text
		_MenuState.anPNColumnsPerRow[_MenuState.nPNNumRows] = 3;
		_MenuState.anPNColumnsPerRow[_MenuState.nPNNumRows+1] = 1;	
		_MenuState.nPNNumRows += 2;
	}

	//////////////////////////////////////////
	// setup the controller config screen vars
	if( !wpr_system_InitControllerConfigData( hFile, &_MenuState.CCData, *_pStringTable ) ) {
		DEVPRINTF( "wpr_system::_Init() : Could not init the Controller Config screens.\n" );
		goto _EXIT_WITH_ERROR;
	}

	//////////////////////////////////////////
	// setup the Advanced Settings screen vars
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszFFBMeshIndexTable );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_system::_Init() : Could not find the table named '%s'.\n", _pszFFBMeshIndexTable );
		goto _EXIT_WITH_ERROR;
	}
	fValue = *(f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	_MenuState.nASIndexOfFFBMesh = (u32)fValue;

	if( bWrappers ) {
		///////////////////////////////////
		// setup the pick level screen vars
		hTable = fgamedata_GetFirstTableHandle( hFile, _pszLevelsTableName );
		if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			DEVPRINTF( "wpr_system::_Init() : Could not find the table named '%s'.\n", _pszLevelsTableName );
			goto _EXIT_WITH_ERROR;
		}
		if( !_InitLevelSelectInfo( &_MenuState.PLSelectInfo, hTable, _pszLevelsTableName ) ) {
			// already printed an error message
			goto _EXIT_WITH_ERROR;
		}
		// make sure that all levels are listed
		if( _MenuState.PLSelectInfo.nNumLevels != LEVEL_SINGLE_PLAYER_COUNT ) {
			DEVPRINTF( "wpr_system::_Init() : The table '%s' should have %d entries, but only has %d, please update either the code or csv file.\n", 
						_pszLevelsTableName,
						LEVEL_SINGLE_PLAYER_COUNT,	
						_MenuState.PLSelectInfo.nNumLevels );
			goto _EXIT_WITH_ERROR;
		}
		
		//////////////
		// Music
		hTable = fgamedata_GetFirstTableHandle( hFile, _pszMusicTableName );
		if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			pszText = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
			_pAudioStream = CFAudioStream::Create( pszText );
		}

		///////////////
		// Audio Bank
		hTable = fgamedata_GetFirstTableHandle( hFile, _pszSoundTableName );
		if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			pszText = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
			fresload_Load( FSNDFX_RESTYPE, pszText );
		}

		/////////////////////////////////////
		// Setup the Multiplayer Level Select
		hTable = fgamedata_GetFirstTableHandle( hFile, _pszMultiplayerLevelTableName );
		if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			DEVPRINTF( "wpr_system::_Init() : Could not find the table named '%s'.\n", _pszMultiplayerLevelTableName );
			goto _EXIT_WITH_ERROR;
		}
		if( !_InitLevelSelectInfo( &_MenuState.MLLevelSelectInfo, hTable, _pszMultiplayerLevelTableName ) ) {
			// already printed an error message
			goto _EXIT_WITH_ERROR;
		}
		hTable = fgamedata_GetFirstTableHandle( hFile, _pszMPLevelUnLockInfoTableName );
		if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			DEVPRINTF( "wpr_system::_Init() : Could not find the table named '%s'.\n", _pszMPLevelUnLockInfoTableName );
			goto _EXIT_WITH_ERROR;
		}
		if( !_InitUnLockInfo( &_MenuState.MLUnLockInfo, hTable, _pszMPLevelUnLockInfoTableName ) ) {
			// already printed an error message
			goto _EXIT_WITH_ERROR;
		}

		//////////////////////
		// init the 3d camera
		_pCamAnimInst = CFCamAnimInst::Load( _pszCamFile );
		if( !_pCamAnimInst ) {
			DEVPRINTF( "wpr_system::_InitPickLevel() : Could not load the camera animation file %s.\n", _pszCamFile  );
			goto _EXIT_WITH_ERROR;
		}
	}

	//////////////////////
	// Sound Effect Tags
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszSoundsEffectTable );
	if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		// grab the number of fields
		nCount = fgamedata_GetNumFields( hTable );
		if( nCount != WPR_DATATYPES_SOUNDS_COUNT ) {
			DEVPRINTF( "wpr_system::_Init() : The code expects %d sound fx tags, there are only %d, there will be no sound fxs.\n",
				WPR_DATATYPES_SOUNDS_COUNT,
				nCount );
		} else {
			for( i=0; i < WPR_DATATYPES_SOUNDS_COUNT; i++ ) {
				pszText = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
				_ahSounds[i] = fsndfx_GetFxHandle( pszText );
			}
		}		
	}

	//////////////////////////////////////
	// everything is cool, we are ready...
	_bSystemOK = TRUE;

	if( bWrappers ) {
		//////////////////////////////////////
		// start on the startup screen for now
		wpr_system_ResetToStartupScreen( _bBootup );

		floop_Reset();
		CEntity::ResolveEntityPointerFixups();

		// clear any pending text prints
		ftext_ClearAllPending();

		_bPrevGovernorState = floop_IsGovernorEnabled();
		floop_EnableGovernor( TRUE );
	}

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );

	#if !GAMELOOP_EXTERNAL_DEMO
		if( bWrappers ) {
			ffile_LogStop();
		}
	#endif

	#if !FANG_PRODUCTION_BUILD
		if( _bInGame ) {
			DEVPRINTF( "In-game wrappers used %u bytes of memory.\n", nFreeBytes - fres_GetFreeBytes() );
		}
	#endif

	if( bWrappers ) {
		DEVPRINTF( "******** LOAD MARKER - END OF WRAPPERS***********.\n" );
		fresload_SetProgressCallback( NULL );
	}

	return TRUE;

_EXIT_WITH_ERROR:
	fdelete_array( _paMeshInsts );
	fdelete_array( _paTexInsts );
	if( _pSkyBox ) {
		_pSkyBox->Destroy();
		fdelete( _pSkyBox );
		_pSkyBox = NULL;
	}
	fdelete_array( _MenuState.PLSelectInfo.paTexInsts );
    fdelete_array( _MenuState.MLLevelSelectInfo.paTexInsts );

	if( bWrappers ) {
		fresload_SetProgressCallback( NULL );
		CGColl::ClearMaterialTable();
		DEVPRINTF( "******** LOAD MARKER - END OF WRAPPERS***********.\n" );
	}

	_ResetSystem();

	fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( hMemFrame );

#if !GAMELOOP_EXTERNAL_DEMO
	if( bWrappers ) {
		ffile_LogStop();
	}
#endif

	launcher_LoadFailure();
	return TRUE;
	//return FALSE;// don't return this because it will break out of the gameloop, we will stay in a message box state
}// _Init()

static BOOL _WrappersLoadingCallback( cchar *pszLoadingString ) {
	// Just check to see if the platform reset button is being pressed.
	gameloop_CheckForPlatformReset( TRUE );

	return TRUE;
}

static BOOL _Work( void ) {
	Wpr_DataTypes_NavCode_e nNavCode;
	s32 nExitAttract, i;
	GameSave_SaveInfo_t *pSaveInfo;

	if( !_bSystemOK ) {
		return FALSE;
	}

	BOOL bNewControllerPluggedIn = FALSE;
	u32 nOldPortMask = Gamepad_nPortOnlineMask;
	gamepad_Sample();
#if defined(MA_PC_INPUT)
	_MouseFrame();
#endif
	if( Gamepad_nPortOnlineMask != nOldPortMask ) {
		bNewControllerPluggedIn = TRUE;
	}

	if( _MenuState.nMode != WPR_DATATYPES_MODES_MA_DEMO &&
		_MenuState.nMode != WPR_DATATYPES_MODES_SIERRA_LOGO &&
#if	FANG_PLATFORM_PS2												//ARG
		_MenuState.nMode != WPR_DATATYPES_MODES_MMI_LOGO &&			//CPS 5.5.03
#endif																//ARG
		_MenuState.nMode != WPR_DATATYPES_MODES_SWINGIN_LOGO &&
		_MenuState.nMode != WPR_DATATYPES_MODES_VUG_LOGO ) {

		CMsgBox::WorkAll();
	
		CEntity::CallAllAutoWorks();
	
		smoketrail_WorkAll();

		// do our camera work
		_pCamAnimInst->DeltaTime( FLoop_fPreviousLoopSecs * 0.05f, FALSE );
		
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

	////////////////////
	// do the world work
	switch( _MenuState.nMode ) {

	case WPR_DATATYPES_MODES_VUG_LOGO:
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		nExitAttract = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		if( (fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING) ||
			(_MenuState.fModeTimer > 0.5f && (nExitAttract >= 0)) ) {
			// done playing the movie, move to the next state
			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
			_MenuState.nMode = WPR_DATATYPES_MODES_SIERRA_LOGO;

			fmovie2_Unload();
			fmovie2_Play( _pszSierraLogoMovie, 0.5f );	
		}
		// don't progress any further in this function
		return TRUE;
		break;

	case WPR_DATATYPES_MODES_SIERRA_LOGO:
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		nExitAttract = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		if( (fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING) ||
			(_MenuState.fModeTimer > 0.5f && (nExitAttract >= 0)) ) {
			// done playing the movie, move to the next state
			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
			_MenuState.nMode = WPR_DATATYPES_MODES_SWINGIN_LOGO;

			fmovie2_Unload();
			fmovie2_Play( _pszSASLogoMovie, 0.25f );	
		}
		// don't progress any further in this function
		return TRUE;
		break;

	case WPR_DATATYPES_MODES_SWINGIN_LOGO:
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		nExitAttract = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		if( (fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING) ||
			(_MenuState.fModeTimer > 0.5f && (nExitAttract >= 0)) ) {
			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
#if FANG_PLATFORM_PS2		//CPS 5.5.03	
			_MenuState.nMode = WPR_DATATYPES_MODES_MMI_LOGO;	//CPS 5.5.03
			fmovie2_Unload();									//CPS 5.5.03
			fmovie2_Play( _pszMMILogoMovie, 0.5f );				//CPS 5.5.03
#else						//CPS 5.5.03
			_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;
			_MenuState.bFadeScreen = TRUE;
			fmovie2_Unload();
#endif						//CPS 5.5.03
		}
		// don't progress any further in this function
		return TRUE;
		break;

// CPS 5.5.03 -->
#if FANG_PLATFORM_PS2
	case WPR_DATATYPES_MODES_MMI_LOGO:
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		nExitAttract = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		if( (fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING) ||
			(_MenuState.fModeTimer > 0.5f && (nExitAttract >= 0)) ) {
			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
			_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;
			_MenuState.bFadeScreen = TRUE;
			fmovie2_Unload();
		}
		// don't progress any further in this function
		return TRUE;
		break;
#endif
// <-- CPS 5.5.03
		
	case WPR_DATATYPES_MODES_MA_DEMO:
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		nExitAttract = wpr_system_FindActiveControllerPort( TRUE, FALSE, FALSE );
		if( nExitAttract >= 0 ||
			bNewControllerPluggedIn ) {
			// stop the movie
			fmovie2_Unload();
			if( _pAudioStream ) {
				_pAudioStream->Pause( FALSE );
				_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );
			}

			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
			_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;	
			_MenuState.bFadeScreen = TRUE;		
		} else if( fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING ) {
			// the movie is over, go to the ma logo page
			if( _pAudioStream ) {
				_pAudioStream->Pause( FALSE );
				_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );
			}
			
			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
			_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;
			_MenuState.bFadeScreen = TRUE;
		}
		// don't progress any further in this function
		return TRUE;
		break;

	case WPR_DATATYPES_MODES_MAIN_MENU:	
		if( bNewControllerPluggedIn ) {
			_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
			_MenuState.fModeTimer = 0.0f;
		}
		// start the music
		if( _pAudioStream &&
			_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );		
			_pAudioStream->Play( 0 );
		}	
		break;

	case WPR_DATATYPES_MODES_SINGLE_PLAYER:
		if( !_HandleUnpluggedController( _MenuState.nControllerIndex ) ) {
			return TRUE;
		}
		// start the music
		if( _pAudioStream &&
			_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );		
			_pAudioStream->Play( 0 );
		}
		break;

	case WPR_DATATYPES_MODES_MULTIPLAYER:
		for( i=0; i < MAX_PLAYERS; i++ ) {
			if( _MenuState.aMJSections[i].nState != _MULTI_JOIN_STATE_ENTER ) {
				if( !_HandleUnpluggedController( i ) ) {
					return TRUE;
				}
			}
		}
		// start the music
		if( _pAudioStream &&
			_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
			_pAudioStream->SetVolume( _MENU_MUSIC_VOLUME );		
			_pAudioStream->Play( 0 );
		}		
		break;
	
	case WPR_DATATYPES_MODES_SIERRA_DEMOS:
		break;
	default:
		FASSERT_NOW;
		break;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// make sure if we are in the correct mode and have selected a memory card, that it is still plugged in
	switch( _MenuState.nMode ) {
		
	case WPR_DATATYPES_MODES_SINGLE_PLAYER:
		pSaveInfo = &_paProfiles[0].m_SaveInfo;
		if( _MenuState.nCurrentScreen != WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT &&
            pSaveInfo->nStorageDeviceID != FSTORAGE_DEVICE_ID_NONE ) {

			if( !_IsCardStillAttached( pSaveInfo->nStorageDeviceID, pSaveInfo->wszCardName ) ) {
				// go back to the memory unit select screen and start over
				_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
				_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;						
			}
		}				
		break;

	case WPR_DATATYPES_MODES_MULTIPLAYER:
		break;

	default:
		break;
	}

	//////////////////////////////
	// do the screen specific work
	if( _aScreenFunctions[ _MenuState.nCurrentScreen ].pWork ) {
		// call the work function
        nNavCode = _aScreenFunctions[ _MenuState.nCurrentScreen ].pWork();
	
		if( nNavCode != WPR_DATATYPES_NAV_CODE_NOTHING ) {
			// do different things depending on what mode we are in
			switch( _MenuState.nMode ) {
			case WPR_DATATYPES_MODES_SIERRA_LOGO:
				break;
			case WPR_DATATYPES_MODES_VUG_LOGO:
				break;
			case WPR_DATATYPES_MODES_SWINGIN_LOGO:
				break;
#if	FANG_PLATFORM_PS2								//ARG
			case WPR_DATATYPES_MODES_MMI_LOGO:		//CPS 5.5.03
				break;								//CPS 5.5.03
#endif												//ARG
			case WPR_DATATYPES_MODES_MAIN_MENU:
			case WPR_DATATYPES_MODES_SINGLE_PLAYER:
			case WPR_DATATYPES_MODES_MULTIPLAYER:
				if( _aScreenFunctions[_MenuState.nCurrentScreen].pExitDecisions ) {
                    _aScreenFunctions[_MenuState.nCurrentScreen].pExitDecisions( nNavCode );
				}
				break;
			case WPR_DATATYPES_MODES_MA_DEMO:
				break;
			case WPR_DATATYPES_MODES_SIERRA_DEMOS:
				break;
			default:
				FASSERT_NOW;
				break;
			}
		}
	}
	return TRUE;
}

static BOOL _Draw( void ) {
	
	if( !_bSystemOK ) {
		// tell the user about the lack of assets
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
		ftext_Printf( 0.5f, 0.35f,
			L"~f1~C%ls~w0~a%lc~s%.2fThe wrapper system didn't init properly,\nyou are probably missing assets.",
			WprDataTypes_pwszSubtitleTextColor,
			L'c',
			1.25f );
		return TRUE;
	}

	f32 fScaleMultiplier = _pViewportOrtho3D->Res.x * (1.0f/640.0f);
	FViewport_t *pPrevViewport;

	/////////////////////////////
	// do the mode specific draw
	switch( _MenuState.nMode ) {

	case WPR_DATATYPES_MODES_MA_DEMO:
		{
			u32 nSecs = (u32)_MenuState.fModeTimer;
			if( nSecs & 0x3 ) {
				if( Gamepad_nPortOnlineMask ) {
					ftext_Printf( 0.5f, 0.65f, 
								L"~f1~C%ls~w0~ac~s1.20%ls",
								WprDataTypes_pwszPressStartColor,
								_apwszPhrases[WPR_DATATYPES_PHRASES_PRESS_START] );
				} else {
					ftext_Printf( 0.5f, 0.65f, 
								L"~f1~C%ls~w0~ac~s1.20%ls",
								WprDataTypes_pwszPressStartColor,
								Game_apwszPhrases[GAMEPHRASE_INSERT_CONTROLLER] );
				}			
			}
		}	
		// fall through to next case...

	case WPR_DATATYPES_MODES_SIERRA_LOGO:
	case WPR_DATATYPES_MODES_VUG_LOGO:
	case WPR_DATATYPES_MODES_SWINGIN_LOGO:
#if	FANG_PLATFORM_PS2							//ARG
	case WPR_DATATYPES_MODES_MMI_LOGO:			//CPS 5.5.03
#endif											//ARG
		FXfm_Identity.InitStackWithView();
		pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

		fmovie2_Draw();	

		fviewport_SetActive( pPrevViewport );
		break;
		
	case WPR_DATATYPES_MODES_MAIN_MENU:		
	case WPR_DATATYPES_MODES_SINGLE_PLAYER:
	case WPR_DATATYPES_MODES_MULTIPLAYER:
		ftex_HandleRenderTargets();
		
		// Clear the viewport to black...
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

		// setup the 3d viewport and init the camera stack
		gamecam_SetViewportAndInitCamStack();

		// draw the sky box
		frenderer_Push( FRENDERER_MESH, NULL );
		_pSkyBox->Draw( &_CamMtx.m_vPos );
		frenderer_Pop();

		// draw the world
		fvis_FrameBegin();
		fvis_Draw( NULL, FALSE, 0 );
		fvis_SetupShadows();
		fvis_FrameEnd();

		FXfm_Identity.InitStackWithView();

		// clear the z buffer so that none of our stuff clips into anything
		fviewport_Clear( FVIEWPORT_CLEARFLAG_DEPTH | FVIEWPORT_CLEARFLAG_STENCIL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
		
		// set our ortho viewport as active
		pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );

		wpr_system_IG_Draw();

		CMsgBox::DrawAll();
#if defined(MA_PC_INPUT)
		_bMouseDrawPointer = TRUE;
		pcinput_DrawsMenuPointer();
#endif
			
		// restore the viewport
		fviewport_SetActive( pPrevViewport );
		break;

	case WPR_DATATYPES_MODES_SIERRA_DEMOS:
		break;

	default:
		FASSERT_NOW;
		break;
	}	

	return TRUE;
}


void wpr_system_IG_SelectScreen( Wpr_DataTypes_Screens_e nScreenIndex ) {
	if( !_bSystemOK ) {
		return;
	}

	_MenuState.nCurrentScreen = nScreenIndex;
	_MenuState.nControllerIndex = Player_aPlayer[0].m_nControllerIndex;
	_MenuState.nCurItemIndex = 0;

	switch( nScreenIndex ) {

	case WPR_DATATYPES_SCREENS_SOUND_OPTIONS:
		_MenuState.nSSNumMusicTicks = (s32)( faudio_GetMusicMasterVol() * (_MENU_ITEMS_SS_NUM_TICKS-1) );
		_MenuState.fSSOrigSoundPercent = faudio_GetSfxMasterVol();
		_MenuState.nSSNumSoundTicks = (s32)( _MenuState.fSSOrigSoundPercent * (_MENU_ITEMS_SS_NUM_TICKS-1) );
		break;

	case WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS:
		_MenuState.bASInvertAnalog = CPlayer::m_pCurrent->GetInvertLook();
		_MenuState.bASAssistedTargeting = (CPlayer::m_pCurrent->GetTargetingAssistance() > 0.0f);
		_MenuState.bASForceFeedbackON = FALSE;
		_MenuState.bASAutoCenter = FALSE;
		_MenuState.nASVibrationTicks = (s16)(fforce_GetMasterIntensity(Player_aPlayer[0].m_nControllerIndex) * (f32)(_MENU_ITEMS_AS_NUM_TICKS-1));
		_MenuState.nASLookSensitivity = (s16)(CPlayer::m_pCurrent->GetLookSensitivity() * (f32)(_MENU_ITEMS_AS_NUM_TICKS-1));
		_MenuState.bASFourWayQuickSelect = CPlayer::m_pCurrent->GetFourWayQuickSelect();
		_MenuState.fASForceFeedbackOrigIntensity = fforce_GetMasterIntensity(Player_aPlayer[0].m_nControllerIndex);
		fforce_NullHandle( &_MenuState.hASForceFeedback );
		break;
	};
}


// Returns TRUE if the user has exited the screen.
BOOL wpr_system_IG_Work( void ) {
	
	if( !_bSystemOK ) {
		return FALSE;
	}

	f32 fUnitVal;
	BOOL bScreenExited;

	GamepadMap_e nPrevGamePadMap = gamepad_SetMapping( _MenuState.nControllerIndex, GAMEPAD_MAP_MENU );

	if( _aScreenFunctions[ _MenuState.nCurrentScreen ].pWork ) {
        Wpr_DataTypes_NavCode_e nNavCode = _aScreenFunctions[ _MenuState.nCurrentScreen ].pWork();

		switch( nNavCode ) {
		
		default:
		case WPR_DATATYPES_NAV_CODE_NOTHING:
			bScreenExited = FALSE;
			break;
			
		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// Save settings...
			bScreenExited = TRUE;
			
			switch( _MenuState.nCurrentScreen ) {

			case WPR_DATATYPES_SCREENS_SOUND_OPTIONS:
				faudio_SetMusicMasterVol( _TICKS_TO_PERCENT( _MenuState.nSSNumMusicTicks, _MENU_ITEMS_SS_NUM_TICKS ) );
				faudio_SetSfxMasterVol( _TICKS_TO_PERCENT( _MenuState.nSSNumSoundTicks, _MENU_ITEMS_SS_NUM_TICKS ) );
				break;

			case WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS:
				CPlayer::m_pCurrent->SetInvertLook( _MenuState.bASInvertAnalog );
				CPlayer::m_pCurrent->SetFourWayQuickSelect( _MenuState.bASFourWayQuickSelect );
				CPlayer::m_pCurrent->SetTargetingAssistance( _MenuState.bASAssistedTargeting );

				if( _MenuState.nASVibrationTicks == 0 ) {
					fforce_SetMasterIntensity( _MenuState.nControllerIndex, 0.0f );
				} else {
					fUnitVal = 0.01f + ((f32)_MenuState.nASVibrationTicks / (f32)(_MENU_ITEMS_AS_NUM_TICKS-1));
					FMATH_CLAMPMAX( fUnitVal, 1.0f );
					fforce_SetMasterIntensity( _MenuState.nControllerIndex, fUnitVal );
				}
				
				fUnitVal = 0.01f + (f32)_MenuState.nASLookSensitivity / (f32)(_MENU_ITEMS_AS_NUM_TICKS-1);
				FMATH_CLAMPMAX( fUnitVal, 1.0f );
				CPlayer::m_pCurrent->SetLookSensitivity( fUnitVal );
				break;
			};
			break;
			
		case WPR_DATATYPES_NAV_CODE_BACK:
			// Cancel...
			bScreenExited = TRUE;
			
			if( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS ) {
				fforce_SetMasterIntensity( _MenuState.nControllerIndex, _MenuState.fASForceFeedbackOrigIntensity );
			} else if( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_SOUND_OPTIONS ) {
				faudio_SetSfxMasterVol( _MenuState.fSSOrigSoundPercent );
			}
			break;
		}		
	}

	gamepad_SetMapping( _MenuState.nControllerIndex, nPrevGamePadMap );

	return bScreenExited;
}


void wpr_system_IG_Draw( void ) {
	if( !_bSystemOK ) {
		return;
	}
#if defined(MA_PC_INPUT)
	_MouseBeginDraw();
#endif
	if( CMsgBox::IsActive() ) {
		// don't draw anything while the message box is up
		return;
	}

	FViewport_t *pViewport = fviewport_GetActive();

	f32 fScaleMultiplier = pViewport->Res.x * (1.0f/640.0f);

	// push the mesh renderer
	frenderer_Push( FRENDERER_MESH, NULL );

	fmesh_Ambient_Set( 0.40f, 0.40f, 0.40f, 1.0f );

	// draw our ortho objects
	if( _aScreenFunctions[ _MenuState.nCurrentScreen ].pDrawOrtho ) {
		_aScreenFunctions[ _MenuState.nCurrentScreen ].pDrawOrtho( fScaleMultiplier,
																pViewport->HalfRes.x,
																pViewport->HalfRes.y );
	}
	// pop the mesh renderer off
	frenderer_Pop();

	// draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );

	if( _aScreenFunctions[ _MenuState.nCurrentScreen ].pDrawFDraw ) {
		_aScreenFunctions[ _MenuState.nCurrentScreen ].pDrawFDraw( fScaleMultiplier,
																pViewport->HalfRes.x,
																pViewport->HalfRes.y );
	}
	wpr_drawutils_DrawButtonOverlay( &Wpr_DataTypes_paScreenData[_MenuState.nCurrentScreen], 
		_MenuState.nButtonDrawMask,
		fScaleMultiplier,
		pViewport->HalfRes.x,
		pViewport->HalfRes.y );

	// push the fdraw renderer off 
	frenderer_Pop();
}

BOOL wpr_system_IG_SaveGame( CPlayerProfile *pProfile0,
							CPlayerProfile *pProfile1/* = NULL*/,
							CPlayerProfile *pProfile2/* = NULL*/,
							CPlayerProfile *pProfile3/* = NULL*/ ) {

	if( _IGSaveState != _IGSAVE_NONE ) {
		return FALSE;
	}

	_apIGSaveProfiles[0] = pProfile0;
	_apIGSaveProfiles[1] = pProfile1;
	_apIGSaveProfiles[2] = pProfile2;
	_apIGSaveProfiles[3] = pProfile3;

	BOOL bSave = FALSE;

	for( s32 i=0; i<MAX_PLAYERS; i++ ) {
		if( _apIGSaveProfiles[i] && !_apIGSaveProfiles[i]->IsVirtual() && 
			(_apIGSaveProfiles[i]->m_SaveInfo.nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING) ) {
				_auIGPlayerSaveStatus[i] = _IGSAVESTATUS_SAVE;
				bSave = TRUE;
			} else {
				_auIGPlayerSaveStatus[i] = _IGSAVESTATUS_SAVED;
			}
	}

	if( bSave ) {
		_IG_StartSave();
	} else {
		return FALSE;
	}

	return TRUE;
}

BOOL wpr_system_CreateLoadHeading( u8 nLevelNum,
								   wchar *pwszBuffer, u32 nBufSize, 
								   wchar *pwszLocBuf/*=NULL*/, u32 nLocBufSize/*=0*/,
								   wchar *pwszTagBuf/*=NULL*/, u32 nTagBufSize/*=0*/ ) {
	FMemFrame_t Frame;
	cwchar *pwszTag = NULL, *pwszLoc = NULL;
	
	Frame = fmem_GetFrame();
	
	if( _bSystemOK && !_bInGame ) {
		// the wrappers are loaded, use the already parsed fields to put the buffer together
		if( nLevelNum < LEVEL_SINGLE_PLAYER_COUNT ) {
			// single player
			FASSERT( nLevelNum < _MenuState.PLSelectInfo.nNumLevels );
			
			pwszTag = _MenuState.PLSelectInfo.papwzNames[nLevelNum];
			pwszLoc = _MenuState.PLSelectInfo.papwzLocations[nLevelNum];		

		} else if( nLevelNum < (LEVEL_SINGLE_PLAYER_COUNT + LEVEL_MULTIPLAYER_COUNT) ) {
			// multiplayer level
			nLevelNum -= LEVEL_SINGLE_PLAYER_COUNT;
			FASSERT( nLevelNum < _MenuState.MLLevelSelectInfo.nNumLevels );

			pwszTag = _MenuState.MLLevelSelectInfo.papwzNames[nLevelNum];
		}
	} else {
		// the wrappers aren't loaded...we will need to load the correct csv file so that we can get
		FGameDataFileHandle_t hFile;
		FGameDataTableHandle_t hTable;
		FGameData_VarType_e nDataType;
		
		u32 nNumFields, nIndex;
		
		if( nLevelNum < LEVEL_SINGLE_PLAYER_COUNT ) {
			// single player
			hFile = fgamedata_LoadFileToFMem( _pszSPLevelCSV );
			if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
				return FALSE;
			}
			hTable = fgamedata_GetFirstTableHandle( hFile, _pszLevelsTableName );
			if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
				fmem_ReleaseFrame( Frame );
				return FALSE;
			}
			nNumFields = fgamedata_GetNumFields( hTable );
			FASSERT( (nNumFields%6) == 0 );
			FASSERT( nNumFields );

			nIndex = nLevelNum * 6;
			pwszLoc = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );
			pwszTag = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
			FASSERT( pwszLoc );
			FASSERT( pwszTag );

		} else if( nLevelNum < (LEVEL_SINGLE_PLAYER_COUNT + LEVEL_MULTIPLAYER_COUNT) ) {
			// multiplayer level
			nLevelNum -= LEVEL_SINGLE_PLAYER_COUNT;
			FASSERT( nLevelNum < _MenuState.MLLevelSelectInfo.nNumLevels );

			hFile = fgamedata_LoadFileToFMem( _pszMPLevelCSV );
			if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
				return FALSE;
			}
			hTable = fgamedata_GetFirstTableHandle( hFile, _pszMultiplayerLevelTableName );
			if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
				fmem_ReleaseFrame( Frame );
				return FALSE;
			}
			nNumFields = fgamedata_GetNumFields( hTable );
			FASSERT( (nNumFields%6) == 0 );
			FASSERT( nNumFields );

			nIndex = nLevelNum * 6;
			pwszTag = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
			FASSERT( pwszTag );		
		}
	}
	
	if( !pwszTag && !pwszLoc ) {
		// didn't find a tag or location string, you've failed me for the last time...
		return FALSE;
	}
	
	// put the load string buffer together, if requested
	if( pwszBuffer ) {
		if( pwszTag && pwszLoc ) {
			// there is both a location and tag name, put the load buffer together
			_snwprintf( pwszBuffer, nBufSize, L"%ls : %ls", pwszLoc, pwszTag );	
		} else if( pwszTag && !pwszLoc ) {
			// there is only a tag name, copy into the load buffer
			fclib_wcsncpy( pwszBuffer, pwszTag, nBufSize );	
		}
	}	
	// make copies of the tag, if requested
	if( pwszTag && pwszTagBuf ) {
		fclib_wcsncpy( pwszTagBuf, pwszTag, nTagBufSize );	
	}
	// make copies of the location, if requested
	if( pwszLoc && pwszLocBuf ) {
		fclib_wcsncpy( pwszLocBuf, pwszLoc, nLocBufSize );
	}
	
	// free any allocated memory
	fmem_ReleaseFrame( Frame );
	
	return TRUE;
}

///////////////////////
// CONTROLLER ROUTINES
///////////////////////
static _UpDown_e _CheckUpDownAxis_Pad( u32 nControllerID ) {

	if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_ANALOG_Y]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_ANALOG_Y]->fCurrentState > 0.1f ) {
			return _UP;			
		} else {
			return _DOWN;
		}
	} else if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_ANALOG_Y]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_ANALOG_Y]->fCurrentState > 0.1f ) {
			return _UP;			
		} else {
			return _DOWN;
		}
	} else if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_DPAD_Y]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
		
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_DPAD_Y]->fCurrentState > 0.1f ) {
			return _UP;
		} else {
			return _DOWN;
		}
	}

	return _NOT_UP_OR_DOWN;
}

static _LeftRight_e _CheckLeftRightAxis_Pad( u32 nControllerID ) {

	if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_ANALOG_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_ANALOG_X]->fCurrentState > 0.1f ) {
			return _RIGHT;
		} else {
			return _LEFT;
		}		
	} else if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_ANALOG_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_ANALOG_X]->fCurrentState > 0.1f ) {
			return _RIGHT;
		} else {
			return _LEFT;
		}		
	} else if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_DPAD_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
		
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_DPAD_X]->fCurrentState > 0.1f ) {
			return _RIGHT;
		} else {
			return _LEFT;
		}
	}

	return _NOT_LEFT_OR_RIGHT;
}

// returns TRUE if "A" button is pressed
static BOOL _CheckAcceptButtons_Pad( u32 nControllerID ) {

	if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
		Gamepad_aapSample[nControllerID][GAMEPAD_MENU_START]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		return TRUE;
	}	
	return FALSE;
}

// returns TRUE if "B" button is pressed
static BOOL _CheckBackButtons_Pad( u32 nControllerID ) {
	if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ||
		Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK2]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		return TRUE;
	}
	return FALSE;	
}

// returns TRUE if "Y" button is pressed
static BOOL _CheckYButton_Pad( u32 nControllerID ) {
	if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_TOP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		return TRUE;
	}
	return FALSE;	
}

// returns TRUE if "X" button is pressed
static BOOL _CheckXButton_Pad( u32 nControllerID ) {
	if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		return TRUE;
	}
	return FALSE;	
}

static _UpDown_e _CheckUpDownAxis( u32 nControllerID ) {
	_UpDown_e nUpDown = _CheckUpDownAxis_Pad( nControllerID );
#if defined(MA_PC_INPUT)
	if( nUpDown != _NOT_UP_OR_DOWN ) {
		_MouseOtherInput( nControllerID );
	} else {
		nUpDown = _MouseUpDown( nControllerID );
	}
#endif
	return nUpDown;
}

static _LeftRight_e _CheckLeftRightAxis( u32 nControllerID ) {
	_LeftRight_e nLeftRight = _CheckLeftRightAxis_Pad( nControllerID );
#if defined(MA_PC_INPUT)
	if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
		_MouseOtherInput( nControllerID );
	} else {
		nLeftRight = _MouseLeftRight( nControllerID );
	}
#endif
	return nLeftRight;
}

// returns TRUE if "A" button is pressed
static BOOL _CheckAcceptButtons( u32 nControllerID ) {
	if( _CheckAcceptButtons_Pad( nControllerID ) ) {
#if defined(MA_PC_INPUT)
		_MouseOtherInput( nControllerID );
#endif
		return TRUE;
	}
#if defined(MA_PC_INPUT)
	return _MouseAccept( nControllerID );
#else
	return FALSE;
#endif
}

// returns TRUE if "B" button is pressed
static BOOL _CheckBackButtons( u32 nControllerID ) {
	if( _CheckBackButtons_Pad( nControllerID ) ) {
#if defined(MA_PC_INPUT)
		_MouseOtherInput( nControllerID );
#endif
		return TRUE;
	}
#if defined(MA_PC_INPUT)
	return _MouseBack( nControllerID );
#else
	return FALSE;
#endif
}

// returns TRUE if "Y" button is pressed
static BOOL _CheckYButton( u32 nControllerID ) {
	if( _CheckYButton_Pad( nControllerID ) ) {
#if defined(MA_PC_INPUT)
		_MouseOtherInput( nControllerID );
#endif
		return TRUE;
	}
#if defined(MA_PC_INPUT)
	return _MousePrompt( nControllerID, _MOUSE_BUTTON_Y );
#else
	return FALSE;
#endif
}

// returns TRUE if "X" button is pressed
static BOOL _CheckXButton( u32 nControllerID ) {
	if( _CheckXButton_Pad( nControllerID ) ) {
#if defined(MA_PC_INPUT)
		_MouseOtherInput( nControllerID );
#endif
		return TRUE;
	}
#if defined(MA_PC_INPUT)
	return _MousePrompt( nControllerID, _MOUSE_BUTTON_X );
#else
	return FALSE;
#endif
}

// returns TRUE if passed in controller is plugged, FALSE if it is not
static BOOL _HandleUnpluggedController( u32 nControllerIndex ) {

	if( Gamepad_nPortOnlineMask & (1 << nControllerIndex) ) {
		// plugged in
		return TRUE;
	}
	// controller is not plugged in
	_MenuState.nUnpluggedControllerState = nControllerIndex;
	_snwprintf( Wpr_DataTypes_wszTempString, 128, Game_apwszPhrases[GAMEPHRASE_LOST_CONTROLLER_FORMATSTRING], '\n', '\n', _MenuState.nUnpluggedControllerState+1 );
	CMsgBox::Display( "CtlReconnect", NULL, Wpr_DataTypes_wszTempString, NULL, NULL, NULL, 0, TRUE, _ControllerWaitForReconnect );

	return FALSE;
}

static BOOL _ControllerWaitForReconnect( void ) {

	if( Gamepad_nPortOnlineMask & (1<<_MenuState.nUnpluggedControllerState) ) {
		CMsgBox::Clear();

		_snwprintf( Wpr_DataTypes_wszTempString, 128, Game_apwszPhrases[GAMEPHRASE_PRESS_START_TO_CONTINUE], '\n' );
		CMsgBox::Display( "CtlPressStart", NULL, Wpr_DataTypes_wszTempString, NULL, NULL, NULL, 0, TRUE, _ControllerWaitForStart );
	}

	return TRUE;
}

static BOOL _ControllerWaitForStart( void ) {
	
	if( Gamepad_nPortOnlineMask & (1<<_MenuState.nUnpluggedControllerState) ) {
		if( Gamepad_aapSample[_MenuState.nUnpluggedControllerState][GAMEPAD_MENU_START]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			// the user pressed start...clear the message box
			CMsgBox::Clear();
			_MenuState.nUnpluggedControllerState = -1;		
				
			// see if we should rescan the memory cards
			if( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT ) {
				Wpr_DataTypes_MUSelectEntry_t *pEntry = &_MenuState.paMUEntries[_MenuState.nCurItemIndex];
				u32 i;
				
				if( pEntry->pDeviceInfo ) {
					// the user had a selection picked, is it still available?
					if( !_IsCardStillAttached( pEntry->pDeviceInfo->oeID, pEntry->pDeviceInfo->wszName ) ) {
						// is any card attached with the same name as the previously selected card
						
						// copy the old card name
						fclib_wcsncpy( Wpr_DataTypes_wszTempString, pEntry->pDeviceInfo->wszName, 128 );
						
						// update MUs					
						_MUSelectUpdate( _MenuState.paMUEntries, TRUE );	
						
						// look for the old card
						_MenuState.nCurItemIndex = 0;
						for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
							if( _MenuState.paMUEntries[i+1].bOfferDevice && !_MenuState.paMUEntries[i+1].bDeviceUnusable ) {
								if( fclib_wcsicmp( Wpr_DataTypes_wszTempString, _MenuState.paMUEntries[i+1].pDeviceInfo->wszName ) == 0 ) {
									_MenuState.nCurItemIndex = i+1;
								}
							}
						}
						if( _MenuState.nCurItemIndex == 0 ) {
							_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
						}					
					}				
				} else {
					// the user had no MU picked, pick a default MU
					_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
					_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
				}
			}
		}	
						
	} else {
		CMsgBox::Clear();
		_snwprintf( Wpr_DataTypes_wszTempString, 128, Game_apwszPhrases[GAMEPHRASE_LOST_CONTROLLER_FORMATSTRING], '\n', '\n', _MenuState.nUnpluggedControllerState+1 );
		CMsgBox::Display( "CtlReconnect", NULL, Wpr_DataTypes_wszTempString, NULL, NULL, NULL, 0, TRUE, _ControllerWaitForReconnect );
	}
	return TRUE;
}

/////////////////
// MISC ROUTINES
/////////////////

static BOOL _InitUnLockInfo( _MPLevelUnlockInfo_t *pUnLockInfo, FGameDataTableHandle_t hTable, cchar *pszTableName ) {
	u32 nNumEntries, i, nIndex = 0, nNumUnlockFields;
	FGameData_VarType_e nDataType;

    // figure out how may entries there are
	nNumEntries = fgamedata_GetNumFields( hTable );
	if( nNumEntries < 2 ) {
		DEVPRINTF( "Wrappers : The table named '%s' does not have the proper number of elements.\n", pszTableName );
		return FALSE;
	}
	// read the first element
	pUnLockInfo->nNumFreebies = (u8)( *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType ) );
	FMATH_CLAMP( pUnLockInfo->nNumFreebies, 0, LEVEL_MULTIPLAYER_COUNT );
	nNumUnlockFields = LEVEL_MULTIPLAYER_COUNT - pUnLockInfo->nNumFreebies;

#if FANG_WINGC
	// The retail game shipped 14 multiplayer levels; this source has 15 (WE13multi13 is not on the
	// disc). The retail table has one chip count per locked level in retail order, i.e. for this
	// source's levels up to 12 and then 14 and 15. Level 13 gets a count that can never be reached.
	const u32 nMissingLevel = LEVEL_MULTIPLAYER_13 - LEVEL_MULTIPLAYER_1;
	const BOOL bRetailLayout = ( nNumEntries == nNumUnlockFields ) && ( nMissingLevel >= pUnLockInfo->nNumFreebies );
	if( nNumEntries != (nNumUnlockFields + 1) && !bRetailLayout ) {
#else
	// make sure that there are a proper number of elements in the table
	if( nNumEntries != (nNumUnlockFields + 1) ) {
#endif
		DEVPRINTF( "Wrappers : The table named '%s' does not have the proper number of elements.\n", pszTableName );
		return FALSE;
	}

	// allocate enough room for the unlock data
	pUnLockInfo->panChipsToUnlock = (u8 *)fres_Alloc( sizeof( u8 ) * nNumUnlockFields );
	if( !pUnLockInfo->panChipsToUnlock ) {
		DEVPRINTF( "Wrappers : Could not allocate %d u8s for the MP level unlock info, out of memory.\n", nNumUnlockFields );
		return FALSE;
	}

	// read the unlock data in
	nIndex = 1;
	for( i=0; i < nNumUnlockFields; i++ ) {
#if FANG_WINGC
		if( bRetailLayout && i == nMissingLevel - pUnLockInfo->nNumFreebies ) {
			pUnLockInfo->panChipsToUnlock[i] = 255;
			continue;
		}
#endif
		pUnLockInfo->panChipsToUnlock[i] = (u8)( *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType ) );
		nIndex++;
	}

	return TRUE;
}

static BOOL _InitLevelSelectInfo( _LevelSelectInfo_t *pSelectInfo, FGameDataTableHandle_t hTable, cchar *pszTableName ) {
	u32 nIndex, i;
	cchar *pszText;
	cwchar *pwszText;
	FGameData_VarType_e nDataType;

    // figure out how may entries there are
	pSelectInfo->nNumLevels = fgamedata_GetNumFields( hTable );
	if( ((pSelectInfo->nNumLevels % 6) != 0) ||
		pSelectInfo->nNumLevels == 0 ) {
			DEVPRINTF( "Wrappers : The table named '%s' does not have the proper number of elements.\n", pszTableName );
			return FALSE;
		}
		pSelectInfo->nNumLevels /= 6;
	
	// allocate enough room for the need textures
	pSelectInfo->paTexInsts = fnew CFTexInst[pSelectInfo->nNumLevels];
	if( !pSelectInfo->paTexInsts ) {
		DEVPRINTF( "Wrappers : Could not allocate %d tex insts for the pick level screenshots, out of memory.\n", pSelectInfo->nNumLevels );
		return FALSE;
	}
	// allocate enough room to store ptrs to the level names
	pSelectInfo->papwzNames = (u16 **)fres_Alloc( sizeof( u16 * ) * pSelectInfo->nNumLevels );
	if( !pSelectInfo->papwzNames ) {
		DEVPRINTF( "Wrappers : Could not allocate %d wchar ptrs for the pick level names, out of memory.\n", pSelectInfo->nNumLevels );
		return FALSE;
	}
	// allocate enough room to store the level numbers
	pSelectInfo->panLevelNumbers = (u32 *)fres_Alloc( sizeof( u32 ) * pSelectInfo->nNumLevels );
	if( !pSelectInfo->panLevelNumbers ) {
		DEVPRINTF( "Wrappers : Could not allocate %d u32 ptrs for the pick level numbers, out of memory.\n", pSelectInfo->nNumLevels );
		return FALSE;
	}
	// allocate enough room to store ptrs to the location names
	pSelectInfo->papwzLocations = (u16 **)fres_Alloc( sizeof( u16 * ) * pSelectInfo->nNumLevels );
	if( !pSelectInfo->papwzLocations ) {
		DEVPRINTF( "Wrappers : Could not allocate %d wchar ptrs for the pick level location names, out of memory.\n", pSelectInfo->nNumLevels );
		return FALSE;
	}
	// allocate enough room to store the time to beat
	pSelectInfo->pafTimeToBeat = (f32 *)fres_Alloc( sizeof( f32 ) * pSelectInfo->nNumLevels );
	if( !pSelectInfo->pafTimeToBeat ) {
		DEVPRINTF( "Wrappers : Could not allocate %d f32 ptrs for the time to beat, out of memory.\n", pSelectInfo->nNumLevels );
		return FALSE;
	}
	// allocate enough room to store the number of secret chips
	pSelectInfo->panSecretChips = (u32 *)fres_Alloc( sizeof( u32 ) * pSelectInfo->nNumLevels );
	if( !pSelectInfo->panSecretChips ) {
		DEVPRINTF( "Wrappers : Could not allocate %d u32 ptrs for the number of secret chips, out of memory.\n", pSelectInfo->nNumLevels );
		return FALSE;
	}

	// load the textures that we will need
	nIndex = 0;
	for( i=0; i < pSelectInfo->nNumLevels; i++ ) {
		// save the location name
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex, nDataType );
		pSelectInfo->papwzLocations[i] = (u16 *)_pStringTable->AddString( pwszText );

		// save the level name
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 1, nDataType );
		pSelectInfo->papwzNames[i] = (u16 *)_pStringTable->AddString( pwszText );

		// load the screen shot texture file
		pszText = (cchar *)fgamedata_GetPtrToFieldData( hTable, nIndex + 2, nDataType );
		pSelectInfo->paTexInsts[i].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszText ) );
		if( !pSelectInfo->paTexInsts[i].GetTexDef() ) {
			DEVPRINTF( "Wrappers : Could not load the screenshot texture file '%s'.\n", pszText );
			return FALSE;
		}
		
		// load the number of secret chips
		pSelectInfo->panSecretChips[i] = (u32)( *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 3, nDataType ) );

		// load the time to beat
		pSelectInfo->pafTimeToBeat[i] = *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 4, nDataType );
		
		// load the level number
		pSelectInfo->panLevelNumbers[i] = (u32)( *(f32 *)fgamedata_GetPtrToFieldData( hTable, nIndex + 5, nDataType ) );

		nIndex += 6;
	}

	return TRUE;
}

static void _DrawText( Wpr_DataTypes_TextLayout_t *pText,
						BOOL bSelected,
						f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
						BOOL bDisabled/*=FALSE*/,
						BOOL bDrawSelectedArrows/*=FALSE*/ ) {
	wchar wcAlignment;

	// set our alignment character
	switch( pText->nAlignment ) {
	case WPR_DATATYPES_ALIGN_LEFT:
		wcAlignment = L'L';
		break;
	case WPR_DATATYPES_ALIGN_RIGHT:
		wcAlignment = L'R';
		break;
	case WPR_DATATYPES_ALIGN_CENTERED:
		wcAlignment = L'C';
		break;
	}

	switch( pText->nType ) {

	case WPR_DATATYPES_TITLE:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszTitleTextColor, wcAlignment, pText->fScale, pText->pwszText );
		break;

	case WPR_DATATYPES_SUBTITLE:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszSubtitleTextColor, wcAlignment, pText->fScale, pText->pwszText );
		break;

	case WPR_DATATYPES_REGULAR:
		if( !bDisabled || bDrawSelectedArrows ) {
			if( !bDrawSelectedArrows ) {
				if( bSelected ) {
					ftext_Printf( pText->fUnitX, pText->fUnitY, L"%ls~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszHiLightedBlink, WprDataTypes_pwszWhiteTextColor, wcAlignment, pText->fScale, pText->pwszText );
				} else {
					ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszBlueTextColor, wcAlignment, pText->fScale, pText->pwszText );
				}
			} else {
				// draw selection arrows or at least space the print out for them
				if( bSelected ) {
					ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1"	// font
																L"~w1"	// fixed width
																L"~a%lc"	// alignment
																L"~s%.2f"// scale	
																L"~C%ls"	// color
																L"{"	// arrow 
																L"%ls"	// blink
																L"~C%ls"	// color
																L"~w0"	// not fixed width
																L"%ls"	// text
																L"~b0"	// no blink
																L"~w1"	// fixed width
																L"~C%ls}",// color & arrow
																wcAlignment,
																pText->fScale,
																WprDataTypes_pwszBlueTextColor,
																WprDataTypes_pwszHiLightedBlink,
																WprDataTypes_pwszWhiteTextColor,
																pText->pwszText,
																WprDataTypes_pwszBlueTextColor );
				} else {
					ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1"	// font
																L"~a%lc"	// alignment	
																L"~w1"	// fixed width
																L"~s%.2f"// scale	
																L"~C%ls"	// color
																L" "	// arrow (or where one would be) 
																L"~w0"	// not fixed width
																L"%ls"	// text
																L"~w1"	// fixed width
																L" ",	// arrow (or where one would be)
																wcAlignment,
																pText->fScale,
																bDisabled ? WprDataTypes_pwszGrayTextColor : WprDataTypes_pwszBlueTextColor,
																pText->pwszText );
				}
			}
		} else {
			ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszGrayTextColor, wcAlignment, pText->fScale, pText->pwszText );
		}		
		break;

	case WPR_DATATYPES_KEYBOARD:
		fclib_wcscpy( Wpr_DataTypes_wszTempString, pText->pwszText );
		if( _MenuState.bPNLower ) {
			fclib_wcslwr( Wpr_DataTypes_wszTempString );
		} else {
			fclib_wcsupr( Wpr_DataTypes_wszTempString );
		}
		if( bSelected ) {
			ftext_Printf( pText->fUnitX, pText->fUnitY, L"~B5~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszWhiteTextColor, wcAlignment, pText->fScale, Wpr_DataTypes_wszTempString );
		} else {
			ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszGrayTextColor2, wcAlignment, pText->fScale, Wpr_DataTypes_wszTempString );
		}
		break;

	case WPR_DATATYPES_MESSAGE:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszMessageTextColor, wcAlignment, pText->fScale, pText->pwszText );
		break;

	case WPR_DATATYPES_HEADING:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszHeadingTextColor, wcAlignment, pText->fScale, pText->pwszText );
		break;

	case WPR_DATATYPES_INSTRUCTION:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f8~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszInstructionTextColor, wcAlignment, pText->fScale, pText->pwszText );
		break;

	case WPR_DATATYPES_TEAM_A:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszTeamAColor, wcAlignment, pText->fScale, pText->pwszText );
		break;

	case WPR_DATATYPES_TEAM_B:
		ftext_Printf( pText->fUnitX, pText->fUnitY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", WprDataTypes_pwszTeamBColor, wcAlignment, pText->fScale, pText->pwszText );
		break;
	}
}

static void _DrawSelectingPlayerMsg( f32 fX/*=0.155f*/, f32 fY/*=0.215f*/ ) {

	ftext_Printf( fX, fY, L"~f8~C%ls~w0~aL~s%.2f%ls %d %ls...",
		WprDataTypes_pwszInstructionTextColor,
		0.7f,
		_apwszPhrases[WPR_DATATYPES_PHRASES_PLAYER],
		_MenuState.nControllerIndex + 1,
		_apwszPhrases[WPR_DATATYPES_PHRASES_SELECTING] );
}

static void _DrawOnOffSelection( f32 fX, f32 fY, BOOL bActiveItem, f32 fScale, BOOL bSelected ) {
	ftext_Printf( fX, fY,
				L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
				bActiveItem ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
				L'C',
				fScale, 
				bSelected ? _apwszPhrases[WPR_DATATYPES_PHRASES_ON] : _apwszPhrases[WPR_DATATYPES_PHRASES_OFF] );
}

static void _ResetSystem( void ) {
	u32 i;

	_bSystemOK = FALSE;
#if defined(MA_PC_INPUT)
	_MouseUninstall();
#endif

	_pAudioStream = NULL;
	_pViewportOrtho3D = NULL;
	_paMeshInsts = NULL;
	_paTexInsts = NULL;
	_nNumMeshes = 0;
	_pStringTable = NULL;
	Wpr_DataTypes_paScreenData = NULL;
	_pSkyBox = NULL;

	for( i=0; i < WPR_DATATYPES_SOUNDS_COUNT; i++ ) {
		_ahSounds[i] = FSNDFX_INVALID_FX_HANDLE;
	}

	_MenuState.paMUEntries = NULL;
	_MenuState.PLSelectInfo.paTexInsts = NULL;
	_MenuState.PLSelectInfo.papwzNames = NULL;
	_MenuState.PLSelectInfo.panLevelNumbers = NULL;
	_MenuState.CCData.paCCScreenPosInfos = NULL;
	_MenuState.CCData.paCCConfigs = NULL;

	_MenuState.MLLevelSelectInfo.paTexInsts = NULL;
	_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
}

// Returns TRUE if the card named pwszCardName is still attached and ready for use
static BOOL _IsCardStillAttached( FStorage_DeviceID_e nStorageDeviceID,
								  cwchar *pwszCardName, BOOL bCheckCardIsReadyForUse ) {
	u32 nConnected, nInserted, nRemoved;
	const FStorage_DeviceInfo_t *pDeviceInfo;

	// update the memory card
	fstorage_UpdateDeviceInfos( &nConnected, &nInserted, &nRemoved );

	// grab the device info
	pDeviceInfo = fstorage_GetDeviceInfo( nStorageDeviceID );
	if( !pDeviceInfo ) {
		return FALSE;
	}
	// make sure that the card is ready to be used
	if( bCheckCardIsReadyForUse ) {
		if( ( pDeviceInfo->uStatus & FSTORAGE_DEVICE_READY_FOR_USE) != FSTORAGE_DEVICE_READY_FOR_USE ) {
			return FALSE;
		}
	} else {
		// Only check to see that the card is still connected...
		if( !( pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_CONNECTED ) ) {
			return FALSE;
		}
	}
	// compare the name of the device to the passed in card name
	if( fclib_wcscmp( pDeviceInfo->wszName, pwszCardName ) != 0 ) {
		return FALSE;
	}

	// must be still attached
	return TRUE;
}

// it is up to the user to make sure that the card is still attached - call _IsCardStillAttached()
static BOOL _DoesNameExistOnCard( FStorage_DeviceID_e nStorageDeviceID,
								  cwchar *pwszProfileName ) {
	u32 nBuffer;

	// ok so the card is in the correct slot, see if pwszProfileName already exists by trying to read a little chunk
	if( fstorage_ReadProfile( nStorageDeviceID, pwszProfileName, 0, &nBuffer, sizeof( u32 ) ) == FSTORAGE_ERROR_NONE ) {
		return TRUE;
	}
	// the profile must not exist
	return FALSE;
}

// checks to see whether there is free space on the device for one saved profile
BOOL _IsSpaceAvailForProfile( const FStorage_DeviceInfo_t *pDevInfo ) {
#if _DEBUG_NO_SPACE 
	return FALSE;
#endif

	u32 uBytesNeeded;
	
	if( fstorage_CalcSaveGameSize( pDevInfo->oeID, sizeof( GameSave_ProfileData_t ), &uBytesNeeded ) != FSTORAGE_ERROR_NONE ) {
		return FALSE;
	}

	return uBytesNeeded <= pDevInfo->uBytesAvailable;
}

// it is up to the user to make sure that the card is still attached - call _IsCardStillAttached()
static BOOL _CreateUniqueName( FStorage_DeviceID_e nStorageDeviceID,
							  wchar *pwszNewName ) {
	u32 i;
	
	i = 0;
	do {
		i++;
		_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"Profile%d", i );

		fclib_wcscpy( pwszNewName, Wpr_DataTypes_wszTempString );
	} while( _DoesNameExistOnCard( nStorageDeviceID, pwszNewName ) );

	return TRUE;
}

static s32 _FindMemCardProfileIndexFromName( const FStorage_DeviceInfo_t *pDevInfo, wchar *pwszProfileName ) {
	u32 i, nReturned;
	FStorage_ProfileInfo_t ProfileInfo;

	for( i=0; i < pDevInfo->uNumProfiles; i++ ) {
		if( fstorage_GetProfileInfos( pDevInfo->oeID,
			&ProfileInfo,
			1,
			&nReturned,
			i ) != FSTORAGE_ERROR_NONE ) {
			return -1;
		}
		if( fclib_wcscmp( pwszProfileName, ProfileInfo.wszName ) == 0 ) {
			// found a match
			return i;
		}
	}
	return -1;
}

// Special version of axis selections for rules screen
static BOOL _HandleRuleAxisSelections( BOOL bLeftRight, u32 nControllerIndex, s8 &rnCurrent,
								       u32 nMaxValidIndex, BOOL bWrapAround,
									   FSndFx_FxHandle_t hSfxToPlayOnChange )
{
	s32 nCache = rnCurrent;

	// change the current level
	s32 nBump = 0;
	if( bLeftRight ) {
		nBump = (s32)_CheckLeftRightAxis( nControllerIndex );
		rnCurrent += nBump;
	} else {
		nBump = (s32)_CheckUpDownAxis( nControllerIndex );
		rnCurrent += nBump;
	}
#if defined(MA_PC_INPUT)
	if( !nBump && &rnCurrent == &_MenuState.nCurItemIndex ) {
		s32 nHover = _MouseHoverSelect( nControllerIndex, (s32)nMaxValidIndex );
		if( nHover >= 0 && !_RuleDisabled( (s16)nHover ) ) {
			rnCurrent = (s8)nHover;
		}
	}
#endif

	// Skip disabled options
	if (nBump != 0) {
		while ( _RuleDisabled(rnCurrent) )
			rnCurrent += nBump;
	}

	if( rnCurrent < 0 ) {
		rnCurrent = (bWrapAround) ? nMaxValidIndex : 0;
	} else if( rnCurrent > (s32)nMaxValidIndex ) {
		rnCurrent = (bWrapAround) ? 0 : nMaxValidIndex;
	}
	if( nCache != rnCurrent ) {
		fsndfx_Play2D( hSfxToPlayOnChange );
		return TRUE;
	}

	return FALSE;
}

static BOOL _RuleDisabled( s16 nCurrent ) {
	switch ( _MenuState.MR_WorkingRules.nGameType ) {
		case GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH:
			return (nCurrent == _MENU_ITEMS_MR_KOTH_SWAP_TIME);
			break;
		case GAME_MULTIPLAYER_BASE_TYPES_KING_OF_THE_HILL:
			return (nCurrent == _MENU_ITEMS_MR_DM_POINTS_TO_WIN);
			break;
		case GAME_MULTIPLAYER_BASE_TYPES_TAG:
		case GAME_MULTIPLAYER_BASE_TYPES_REVERSE_TAG:
			return ( (nCurrent == _MENU_ITEMS_MR_KOTH_SWAP_TIME) ||
					 (nCurrent == _MENU_ITEMS_MR_DM_POINTS_TO_WIN) );
	}
	return FALSE;
}

static BOOL _HandleAxisSelections( BOOL bLeftRight, u32 nControllerIndex, s8 &rnCurrent,
								u32 nMaxValidIndex, BOOL bWrapAround, FSndFx_FxHandle_t hSfxToPlayOnChange ) {
	s32 nCache = rnCurrent;

	// change the current level
	if( bLeftRight ) {
		_LeftRight_e nLeftRight = _CheckLeftRightAxis( nControllerIndex );
		rnCurrent += nLeftRight;
#if defined(MA_PC_INPUT)
		// the wheel pages sideways lists: up = previous
		if( nLeftRight == _NOT_LEFT_OR_RIGHT && &rnCurrent == &_MenuState.nCurItemIndex ) {
			rnCurrent += _MouseUpDown( nControllerIndex );
		}
#endif
	} else {
		_UpDown_e nUpDown = _CheckUpDownAxis( nControllerIndex );
		rnCurrent += nUpDown;
	}
#if defined(MA_PC_INPUT)
	if( nCache == rnCurrent && &rnCurrent == &_MenuState.nCurItemIndex ) {
		s32 nHover = _MouseHoverSelect( nControllerIndex, (s32)nMaxValidIndex );
		if( nHover >= 0 ) {
			rnCurrent = (s8)nHover;
		}
	}
#endif
	if( rnCurrent < 0 ) {
		rnCurrent = (bWrapAround) ? nMaxValidIndex : 0;
	} else if( rnCurrent > (s32)nMaxValidIndex ) {
		rnCurrent = (bWrapAround) ? 0 : nMaxValidIndex;
	}
	if( nCache != rnCurrent ) {
		ftext_ResetBlinkTimers();
		fsndfx_Play2D( hSfxToPlayOnChange );
		return TRUE;
	}

	return FALSE;
}

/////////////////////////
// 'main menu' functions
/////////////////////////
static Wpr_DataTypes_NavCode_e _MainMenu_Work( void ) {

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_NO_BUTTONS;
	
	if( _MenuState.bFadeScreen ) {
		// fade the screen in before allowing the user to make a selection
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		if( _MenuState.fModeTimer >= _TOTAL_FADE_IN_TIME ) {
			_MenuState.bFadeScreen = FALSE;	
		}
		return WPR_DATATYPES_NAV_CODE_NOTHING;	
	}

	// see if we have a valid controller index, if not see if a controller has been pressed
	s32 nControllerIndex = _MenuState.nControllerIndex;
	if( nControllerIndex < 0 ) {
		// wait for the user to press a controller button on one of the controllers
		nControllerIndex = wpr_system_FindActiveControllerPort( TRUE, TRUE, TRUE );
		if( nControllerIndex >= 0 ) {
			// the user finally picked a controller, use this controller
			_MenuState.nControllerIndex = nControllerIndex;
			_MenuState.fModeTimer = 0.0f;// reset the timer
		}
	}
	s32 nCache = _MenuState.nCurItemIndex;

	if( nControllerIndex >= 0 ) {
		// check for the A button
		if( _CheckAcceptButtons( nControllerIndex ) ) {
			return WPR_DATATYPES_NAV_CODE_FORWARD;
		}

		// check for up/down changes
		_UpDown_e nUpDown = _CheckUpDownAxis( nControllerIndex );
		if( nUpDown == _UP ) {
			_MenuState.nCurItemIndex--;
			if( _MenuState.nCurItemIndex < 0 ) {
				_MenuState.nCurItemIndex = 0;
			}
		} else if( nUpDown == _DOWN ) {
			_MenuState.nCurItemIndex++;
			if( _MenuState.nCurItemIndex >= _MENU_ITEMS_MM_COUNT ) {
				_MenuState.nCurItemIndex = (_MENU_ITEMS_MM_COUNT-1);
			}
		}
#if defined(MA_PC_INPUT)
		s32 nHover = _MouseHoverSelect( nControllerIndex, _MENU_ITEMS_MM_COUNT-1 );
		if( nHover >= 0 ) {
			_MenuState.nCurItemIndex = (s8)nHover;
		}
#endif
	}

	if( nCache != _MenuState.nCurItemIndex ) {
		ftext_ResetBlinkTimers();
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
		_MenuState.fModeTimer = 0.0f;// reset the timer
	} else {
		// no change, time how long the player is on this screen with no change, if too long, switch to demo mode
		_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
		if( _MenuState.fModeTimer >= _MAIN_MENU_INACTIVE_TIME ) {
			return WPR_DATATYPES_NAV_CODE_BACK;
		}
	}
		
	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MainMenu_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MAIN_MENU];

	// draw the highlight behind the selected world
	Wpr_DataTypes_MeshLayout_t Layout, *pCurrent;
	Layout.pMeshInst = wpr_datatypes_FindMeshInst( "gfh_logo05", _nNumMeshes, _paMeshInsts );
	if( Layout.pMeshInst ) {
		pCurrent = &pScreen->pMesh[_MENU_ITEMS_MM_START_OFFSET + _MenuState.nCurItemIndex];

		Layout.fBiPolarUnitX = pCurrent->fBiPolarUnitX;
		Layout.fBiPolarUnitY = pCurrent->fBiPolarUnitY;
		Layout.fDrawZ = pCurrent->fDrawZ + WPR_DATATYPES_LAYER_Z;
		Layout.fScale = pCurrent->fScale; 
				
		wpr_drawutils_DrawMesh_WithRot( &Layout, 
			fScaleMultiplier, fHalfXRes, fHalfYRes, 
			0.0f, 0.0f, 0.0f );
	}

	// draw the rest of the screen
	wpr_system_DrawBasicScreen( pScreen,
		_MENU_ITEMS_MM_START_OFFSET + _MenuState.nCurItemIndex,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );
#if defined(MA_PC_INPUT)
	// the items are 3D text meshes
	for( s32 nItem=0; nItem < _MENU_ITEMS_MM_COUNT; nItem++ ) {
		if( _MENU_ITEMS_MM_START_OFFSET + nItem < (s32)pScreen->nNumMeshElements ) {
			_MouseAddMesh( nItem, &pScreen->pMesh[_MENU_ITEMS_MM_START_OFFSET + nItem], fScaleMultiplier, fHalfXRes, fHalfYRes );
		}
	}
#endif
        
	if( _MenuState.bFadeScreen ) {
		f32 fUnitPercent = _MenuState.fModeTimer * (1.0f/_TOTAL_FADE_IN_TIME);
		if( fUnitPercent >= 1.0f ) {
			fUnitPercent = 1.0f;
		}
		//fUnitPercent = fmath_UnitLinearToSCurve( fUnitPercent );

		game_DrawSolidFullScreenOverlay( fUnitPercent, 0.0f );
	}
}

static void _MainMenu_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// we need to change modes

		// setup the next screen
		switch( _MenuState.nCurItemIndex ) {

		case _MENU_ITEMS_MM_SINGLE:
			_MenuState.nMode = WPR_DATATYPES_MODES_SINGLE_PLAYER;
			// setup the memory unit select
			_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
			_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
			_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MAIN_MENU;

#if FANG_PLATFORM_XB
			// if it's an xbox, have to check hard drive space here to make sure there's room for at least one save game
			const FStorage_DeviceInfo_t *pXBHD;
			u32 nBlocksAvailable, nBytesNeededToSave, nBlocksNeededToSave;

			pXBHD = fstorage_GetDeviceInfo( FSTORAGE_DEVICE_ID_XB_PC_HD );
			if( pXBHD ) {
				fstorage_BytesToBlocks( FSTORAGE_DEVICE_ID_XB_PC_HD, (u32)pXBHD->uBytesAvailable, &nBlocksAvailable );
			} else {
				nBlocksAvailable = 0;
			}
#if _DEBUG_NO_SPACE
			nBlocksAvailable = 0;
#endif

			fstorage_CalcSaveGameSize( FSTORAGE_DEVICE_ID_XB_PC_HD, sizeof( GameSave_ProfileData_t ), &nBytesNeededToSave );
			fstorage_BytesToBlocks( FSTORAGE_DEVICE_ID_XB_PC_HD, nBytesNeededToSave, &nBlocksNeededToSave );

			if( nBlocksAvailable >= nBlocksNeededToSave ) {
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
			} else {
				_MenuState.uHDNeededBlocks = nBlocksNeededToSave - nBlocksAvailable;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_WARNING_HDSPACE;
			}
#else
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
#endif
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			break;

		case _MENU_ITEMS_MM_MULTI:
			_MenuState.nMode = WPR_DATATYPES_MODES_MULTIPLAYER;
			// setup the multiplayer join screen
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_JOIN;
			_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MAIN_MENU;

			_MJ_InitJoinInfoAndSections( &_MenuState.MJJoinInfo, _MenuState.aMJSections, MAX_PLAYERS );

			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			break;

		default:
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] ); 
			break;
		}
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// inactive for too long, start a demo mode
		_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
		_MenuState.fModeTimer = 0.0f;
		_MenuState.nMode = WPR_DATATYPES_MODES_MA_DEMO;
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
		fmovie2_Play( _pszDemoMovie, 0.35f );
#else
		fmovie2_Play( _pszDemoMovie, 0.5f );
#endif
		if( _pAudioStream ) {
			_pAudioStream->SetVolume( 0.0f );
			_pAudioStream->Pause( TRUE );
		}
		break;

	default:
		FASSERT_NOW;
		break;
	}
}


////////////////////////////
// 'sound setting' functions
////////////////////////////

static Wpr_DataTypes_NavCode_e _SoundSettings_Work( void ) {
	
	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

	// see if there are up/down changes
	if( !_HandleAxisSelections( FALSE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex, (_MENU_ITEMS_SS_COUNT-1),
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] ) ) {
		
		// see if there are left/right changes
		u32 nCache;
		_LeftRight_e nLeftRight = _CheckLeftRightAxis( _MenuState.nControllerIndex );
		if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
			
			switch( _MenuState.nCurItemIndex ) {
			case _MENU_ITEMS_SS_SOUND:
				nCache = _MenuState.nSSNumSoundTicks;
				_MenuState.nSSNumSoundTicks += nLeftRight;
				FMATH_CLAMP( _MenuState.nSSNumSoundTicks, 0, (_MENU_ITEMS_SS_NUM_TICKS-1) );
				if( nCache != _MenuState.nSSNumSoundTicks ) {
					if( _bInGame ) {
						// adjust the master volume then play the tone at full volume
						faudio_SetSfxMasterVol( _TICKS_TO_PERCENT( _MenuState.nSSNumSoundTicks, _MENU_ITEMS_SS_NUM_TICKS ) );
						fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_VOLUME_ADJUST], 1.0f );	
					} else {
						// adjust the volume of the tone played to reflect the new volume level
						fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_VOLUME_ADJUST], _TICKS_TO_PERCENT( _MenuState.nSSNumSoundTicks, _MENU_ITEMS_SS_NUM_TICKS ) );
					}
				}
				break;
			case _MENU_ITEMS_SS_MUSIC:
				nCache = _MenuState.nSSNumMusicTicks;
				_MenuState.nSSNumMusicTicks += nLeftRight;
				FMATH_CLAMP( _MenuState.nSSNumMusicTicks, 0, (_MENU_ITEMS_SS_NUM_TICKS-1) );
				if( nCache != _MenuState.nSSNumMusicTicks ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_VOLUME_ADJUST], _TICKS_TO_PERCENT( _MenuState.nSSNumMusicTicks, _MENU_ITEMS_SS_NUM_TICKS ) );
				}
				break;
			default:
				FASSERT_NOW;
				break;
			}
		}
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
	
	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _SoundSettings_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_SOUND_OPTIONS];
	
    // draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_SS_START_OFFSET],
						  &_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
						  fScaleMultiplier, fHalfXRes, fHalfYRes );

	
	// draw the tick marks for the 2 level boxes
	wpr_drawutils_DrawTickMarks( _MenuState.nSSNumSoundTicks,
					(_MENU_ITEMS_SS_NUM_TICKS-1),
					&_paTexInsts[WPR_DATATYPES_TEXTURES_TICK],
					-0.55802816f,
					-0.0060718199238181114f,
					0.19267919100821018f,
					0.063015616498887539f, 
 					fScaleMultiplier,
					fHalfXRes,
					fHalfYRes );

	wpr_drawutils_DrawTickMarks( _MenuState.nSSNumMusicTicks,
					(_MENU_ITEMS_SS_NUM_TICKS-1),
					&_paTexInsts[WPR_DATATYPES_TEXTURES_TICK],
					-0.55802816f,
					-0.45566194737330079f,
					0.19267919100821018f,
					0.063015616498887539f, 
 					fScaleMultiplier,
					fHalfXRes,
					fHalfYRes );
}

static void _SoundSettings_ProfileToWorkingVars() {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	_MenuState.nSSNumMusicTicks = (s32)( pProfile->fUnitMusicLevel * (_MENU_ITEMS_SS_NUM_TICKS-1) );
	_MenuState.nSSNumSoundTicks = (s32)( pProfile->fUnitSoundLevel * (_MENU_ITEMS_SS_NUM_TICKS-1) );
}

static void _SoundSettings_WorkingVarsToProfile() {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	pProfile->fUnitMusicLevel = _TICKS_TO_PERCENT( _MenuState.nSSNumMusicTicks, _MENU_ITEMS_SS_NUM_TICKS );
	pProfile->fUnitSoundLevel = _TICKS_TO_PERCENT( _MenuState.nSSNumSoundTicks, _MENU_ITEMS_SS_NUM_TICKS );
}

static void _SoundSettings_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the profile settings screen, lose changes
		_SoundSettings_ProfileToWorkingVars();
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_AUDIO;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// save changes and go back to the profile settings screen
		_SoundSettings_WorkingVarsToProfile();
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_AUDIO;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		
		// flag the profile for saving
		_paProfiles[0].m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_SOUND_OPTIONS;
}

///////////////////////////////////////
// 'don't remove mu' warning functions
///////////////////////////////////////

// returns WPR_DATATYPES_NAV_CODE_FORWARD when we can move on to the next state
static Wpr_DataTypes_NavCode_e _RemoveWarning_Work( void ) {
	GameSave_SaveInfo_t *pSaveInfo = &_MenuState.pDRProfile->m_SaveInfo;
    
	if( _MenuState.fDRTimer >= 0.09f &&	// by wait a small amount, we ensure that the message is on the screen and not in the backbuffer
		pSaveInfo->nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING ) {
		// we need to save the profile

		// time how long it takes to save to the card
		CFTimer Timer;
		Timer.Reset();

		if( pSaveInfo->nFlags & GAMESAVE_SAVE_INFO_FLAGS_RENAME ) {
			// rename the profile on the card
			_MenuState.bWriteSuccessful = _MenuState.pDRProfile->RenameProfile( _MenuState.pwszDRNewName );
		} else {
			// save out the profile to the card
			_MenuState.bWriteSuccessful = _MenuState.pDRProfile->SaveToCard(); 
		}
        
		// uncheck the save flags
		pSaveInfo->nFlags &= ~(GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW | GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING | GAMESAVE_SAVE_INFO_FLAGS_RENAME);

		// increase the timer (make sure that a min time is added)
		_MenuState.fDRTimer += ( FLoop_fRealPreviousLoopSecs + Timer.SampleSeconds() );
	} else {
		// just wait enough time to meet TCR specs
		_MenuState.fDRTimer += FLoop_fRealPreviousLoopSecs;
		if( _MenuState.fDRTimer >= 1.1f ) {
			// enough time has past, move on
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			return WPR_DATATYPES_NAV_CODE_FORWARD;
		}
	}
	
	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_NO_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _RemoveWarning_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	u32 i;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING];
	GameSave_SaveInfo_t *pSaveInfo = &_MenuState.pDRProfile->m_SaveInfo;
	
	BOOL bSaveToHD = (pSaveInfo->nStorageDeviceID == FSTORAGE_DEVICE_ID_XB_PC_HD);

	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {	
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}

	// draw the text
	for( i=0; i < pScreen->nNumTextElements; i++ ) {
		if( bSaveToHD ) {
			if( i > _MENU_ITEMS_DR_HD_SAVE_LAST_OFFSET ) {
				continue;
			}
		} else {
			if( i >= _MENU_ITEMS_DR_HD_SAVE_FIRST_OFFSET && i <= _MENU_ITEMS_DR_HD_SAVE_LAST_OFFSET ) {
				continue;
			}
		}
		// draw text
		_DrawText( &pScreen->pText[i], FALSE, fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	if( !bSaveToHD ) {
		// draw the name of the memory unit
		f32 fY = pScreen->pText[_MENU_ITEMS_DR_HD_SAVE_LAST_OFFSET + 1].fUnitY;
		fY += pScreen->pText[_MENU_ITEMS_DR_HD_SAVE_LAST_OFFSET + 2].fUnitY;
		fY *= 0.5f;
		_SafeCopyFStorageName( Wpr_DataTypes_wszTempString, pSaveInfo->wszCardName, _MAX_MU_NAME_DISPLAY_LEN );
		ftext_Printf( 0.5f, fY, L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
			WprDataTypes_pwszInstructionTextColor,
			L'C',
			pScreen->pText[_MENU_ITEMS_DR_HD_SAVE_LAST_OFFSET + 1].fScale,  
			Wpr_DataTypes_wszTempString );
	}	
#else
	// gc is a simple msg
	_GenericDrawOrtho_NoSelections( fScaleMultiplier, fHalfXRes, fHalfYRes );
#endif
}

static void _RemoveWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	FASSERT( nNavCode == WPR_DATATYPES_NAV_CODE_FORWARD );

	// go to different places, depending on where we came from
	switch( _MenuState.nDRScreenToReturn ) {

	case WPR_DATATYPES_SCREENS_PROFILE_SELECT:
		if( _MenuState.bWriteSuccessful ) {
			// find out if we should be able to create a new profile
			_MenuState.bPPRoomForNewProfiles = _IsSpaceAvailForProfile( _MenuState.pPPDevInfo );
			_MenuState.nPPNumSelections = _MenuState.pPPDevInfo->uNumProfiles;
			if( _MenuState.bPPRoomForNewProfiles ) {
				_MenuState.nPPNumSelections++;
			}		
			// find the newly created profile and make that the current selected
			_SelectProfile_SetupReturnVisit( _MenuState.pPPDevInfo, _MenuState.pDRProfile->m_SaveInfo.wszProfileName );			
		} else {
			// there was an error saving the profile
			_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ERROR_CREATE;
		}
		break;

	case WPR_DATATYPES_SCREENS_PROFILE_SETTINGS:
		if( _MenuState.bWriteSuccessful ) {
			_MenuState.nCurItemIndex = _MENU_ITEMS_PS_EDIT_NAME;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		} else {
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ERROR_RENAME;
		}
		break;

	case WPR_DATATYPES_SCREENS_NONE:
		// we are moving into the game
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NONE;
		_MenuState.fModeTimer = 0.0f;// reset the timer
		break;

	case WPR_DATATYPES_SCREENS_LAUNCH:
		if( _MenuState.nLastScreen == WPR_DATATYPES_SCREENS_ERROR_LOAD ) {
			//if( _MenuState.bWriteSuccessful ) {
				_SetupLaunchMenu();
			//}
		} else {
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_EDIT_SETTINGS;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;	
		}		
		break;

	default:
		// If we don't have any special instructions, just go there
		_MenuState.nCurrentScreen = (Wpr_DataTypes_Screens_e)_MenuState.nDRScreenToReturn;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING;
}

static void _SetupMemCardSave( Wpr_DataTypes_Screens_e nScreenToReturn, CPlayerProfile *pProfile ) {

	pProfile->m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
	_MenuState.nDRScreenToReturn = nScreenToReturn;
	_MenuState.fDRTimer = 0.0f;
	_MenuState.pwszDRNewName = pProfile->m_SaveInfo.wszProfileName;
	_MenuState.pDRProfile = pProfile;
	
	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING;
}

//////////////////////////////////////
// 'already exists' warning functions
//////////////////////////////////////

static Wpr_DataTypes_NavCode_e _AlreadyExistsWarning_Work( void ) {

	return _GenericButtonWork( TRUE, FALSE, WPR_DATATYPES_DRAW_A_BUTTON_ONLY );	
}

static void _AlreadyExistsWarning_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
		
	_GenericDrawOrtho_NoSelectionsWithString( &Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ],
											  0.32f,
											  _MenuState.wszPNCurProfileName, 
											  fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _AlreadyExistsWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	
	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// back to the profile name screen to pick a new name
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;		
		// don't set this, the profile name screen uses the last to determine where to go back to
		// _MenuState.nLastScreen = WPR_DATATYPES_SCREENS_ALREADY_EXISTS_WARNING;
		break;

	default:
		FASSERT_NOW;
		break;
	}
}

///////////////////////////////////////////
// 'delete profile' confirmation functions
///////////////////////////////////////////

static Wpr_DataTypes_NavCode_e _DeleteProfile_Work( void ) {

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );		
}

static void _DeleteProfile_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	_GenericDrawOrtho_NoSelectionsWithString( &Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ],
											0.36f,
											_paProfiles[0].m_SaveInfo.wszProfileName, 
											fScaleMultiplier, fHalfXRes, fHalfYRes );	
}

static void _DeleteProfile_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// delete the profile
		_paProfiles[0].DeleteFromCard();
		pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
				
		// return to the select profile screen
		_MenuState.nCurItemIndex = 0;// we don't know how many profiles there are, default to creating a new entry
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SELECT;

		// determine if we cleared up enough space on the card for a new profile
		_MenuState.bPPRoomForNewProfiles = _IsSpaceAvailForProfile( _MenuState.pPPDevInfo );		// (_MenuState.pPPDevInfo->uBytesAvailable >= sizeof( GameSave_ProfileData_t ));
		_MenuState.nPPNumSelections = _MenuState.pPPDevInfo->uNumProfiles;
		if( _MenuState.bPPRoomForNewProfiles ) {
			_MenuState.nPPNumSelections++;
		}
		_MenuState.nPPTopSelectionIndex = 0;
		_MenuState.nPPBottomSelectionIndex = (_MENU_ITEMS_PP_ON_SCREEN_LIMIT-1);
		_MenuState.bPPNeedToCacheProfiles = TRUE;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// don't delete the profile, return to the select profile screen
		_SelectProfile_SetupReturnVisit( _MenuState.pPPDevInfo, pSaveInfo->wszProfileName );
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_DELETE_PROFILE;
}

/////////////////////////////////
// 'advanced settings' functions
/////////////////////////////////

static Wpr_DataTypes_NavCode_e _AdvSettings_Work( void ) {
	f32 fPercent;

	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );

		if( _MenuState.bASForceFeedbackON ) {
			// stop the force feedback and return the intensity back to its original state
			fforce_Kill( &_MenuState.hASForceFeedback );
			fforce_NullHandle( &_MenuState.hASForceFeedback );
			fforce_SetMasterIntensity( _MenuState.nControllerIndex, _MenuState.fASForceFeedbackOrigIntensity );
			_MenuState.bASForceFeedbackON = FALSE;
		}			
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {

		if( _MenuState.bASForceFeedbackON ) {
			// stop the force feedback and return the intensity back to its original state
			fforce_Kill( &_MenuState.hASForceFeedback );
			fforce_NullHandle( &_MenuState.hASForceFeedback );
			fforce_SetMasterIntensity( _MenuState.nControllerIndex, _MenuState.fASForceFeedbackOrigIntensity );
			_MenuState.bASForceFeedbackON = FALSE;
		}
		return WPR_DATATYPES_NAV_CODE_BACK;			
	}
	s32 nCache = _MenuState.nCurItemIndex;

#if defined(MA_PC_INPUT)
	s32 nHover = _MouseHoverSelect( _MenuState.nControllerIndex, _MENU_ITEMS_AS_COUNT-1 );
	if( nHover >= 0 ) {
		_MenuState.nCurItemIndex = (s8)nHover;
	}
#endif
	// check for up/down changes
    _UpDown_e nUpDown = _CheckUpDownAxis( _MenuState.nControllerIndex );
	if( nUpDown == _UP ) {
		_MenuState.nCurItemIndex--;
		if( _MenuState.nCurItemIndex < 0 ) {
			_MenuState.nCurItemIndex = 0;
		}
	} else if( nUpDown == _DOWN ) {
		_MenuState.nCurItemIndex++;
		if( _MenuState.nCurItemIndex >= _MENU_ITEMS_AS_COUNT ) {
			_MenuState.nCurItemIndex = (_MENU_ITEMS_AS_COUNT-1);
		}
	} else {
		_LeftRight_e nLeftRight = _CheckLeftRightAxis( _MenuState.nControllerIndex );
		if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
			s32 nCache2;

			// the left right axis has changed
			switch( _MenuState.nCurItemIndex ) {

			case _MENU_ITEMS_AS_INVERT:
				nCache2 = _MenuState.bASInvertAnalog;
				_MenuState.bASInvertAnalog = (nLeftRight == _LEFT) ? FALSE : TRUE;

				if( nCache2 != _MenuState.bASInvertAnalog ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				}
				break;

			case _MENU_ITEMS_AS_VIBRATION:
				nCache2 = _MenuState.nASVibrationTicks;					
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
				_MenuState.nASVibrationTicks += nLeftRight;
				FMATH_CLAMP( _MenuState.nASVibrationTicks, 0, (_MENU_ITEMS_AS_NUM_TICKS-1) );
#else
				// only allow on/off
				if( nLeftRight == _RIGHT ) {
                    _MenuState.nASVibrationTicks = (_MENU_ITEMS_AS_NUM_TICKS-1);
				} else {
					_MenuState.nASVibrationTicks = 0;
				}
#endif
				if( nCache2 != _MenuState.nASVibrationTicks ) {
					fPercent = _TICKS_TO_PERCENT( _MenuState.nASVibrationTicks, _MENU_ITEMS_AS_NUM_TICKS );

					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_VOLUME_ADJUST], fPercent );
					fforce_SetMasterIntensity( _MenuState.nControllerIndex, fPercent );
					fforce_Kill( &_MenuState.hASForceFeedback );
					
					if( _MenuState.nASVibrationTicks ) {	                    	
						fforce_Play( _MenuState.nControllerIndex, FFORCE_EFFECT_MENU_RUMBLE, &_MenuState.hASForceFeedback, FLoop_bGamePaused ? FFORCE_DOMAIN_PAUSED : FFORCE_DOMAIN_NORMAL );
					}
				}
				break;
#if WPR_SYSTEM_ALLOW_AUTO_CENTER_CHANGES
			case _MENU_ITEMS_AS_AUTO_CENTER:
				nCache2 = _MenuState.bASAutoCenter;
				_MenuState.bASAutoCenter = (nLeftRight == _LEFT) ? FALSE : TRUE;

				if( nCache2 != _MenuState.bASAutoCenter ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				}
				break;
#endif
			case _MENU_ITEMS_AS_LOOK_SENSITIVITY:
				nCache2 = _MenuState.nASLookSensitivity;
				_MenuState.nASLookSensitivity += nLeftRight;
				FMATH_CLAMP( _MenuState.nASLookSensitivity, 0, (_MENU_ITEMS_AS_NUM_TICKS-1) );

				if( nCache2 != _MenuState.nASLookSensitivity ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_VOLUME_ADJUST], _TICKS_TO_PERCENT( _MenuState.nASLookSensitivity, _MENU_ITEMS_AS_NUM_TICKS ) );
				}
				break;

			case _MENU_ITEMS_AS_DPAD_COMBO:
				nCache2 = _MenuState.bASFourWayQuickSelect;
				_MenuState.bASFourWayQuickSelect = (nLeftRight != _LEFT);

				if( nCache2 != _MenuState.bASFourWayQuickSelect ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				}
				break;

			case _MENU_ITEMS_AS_ASSISTED_TARGETING:
				nCache2 = _MenuState.bASAssistedTargeting;
				_MenuState.bASAssistedTargeting = (nLeftRight == _LEFT) ? FALSE : TRUE;

				if( nCache2 != _MenuState.bASAssistedTargeting ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				}
				break;				

			default:
				FASSERT_NOW;
				break;
			}
		}			
	}
	if( nCache != _MenuState.nCurItemIndex ) {
		ftext_ResetBlinkTimers();
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

		if( _MenuState.nCurItemIndex == _MENU_ITEMS_AS_VIBRATION ) {
			// start up force feedback 
			if( !_MenuState.bASForceFeedbackON ) {
				fforce_NullHandle( &_MenuState.hASForceFeedback );
				if( _MenuState.nASVibrationTicks ) {
					fforce_Play( _MenuState.nControllerIndex, FFORCE_EFFECT_MENU_RUMBLE, &_MenuState.hASForceFeedback, FLoop_bGamePaused ? FFORCE_DOMAIN_PAUSED : FFORCE_DOMAIN_NORMAL );
				}
				_MenuState.fASForceFeedbackOrigIntensity = fforce_SetMasterIntensity( _MenuState.nControllerIndex,
																					_TICKS_TO_PERCENT( _MenuState.nASVibrationTicks, _MENU_ITEMS_AS_NUM_TICKS ) );
				_MenuState.bASForceFeedbackON = TRUE;
			}
		} else if( nCache == _MENU_ITEMS_AS_VIBRATION ) {
			if( _MenuState.bASForceFeedbackON ) {
				// stop the force feedback and return the intensity back to its original state
				fforce_SetMasterIntensity( _MenuState.nControllerIndex, _MenuState.fASForceFeedbackOrigIntensity );
				fforce_Kill( &_MenuState.hASForceFeedback );
				fforce_NullHandle( &_MenuState.hASForceFeedback );					
				_MenuState.bASForceFeedbackON = FALSE;
			}
		}
	} else if( _MenuState.nCurItemIndex == _MENU_ITEMS_AS_VIBRATION ) {
		//FASSERT( _MenuState.bASForceFeedbackON );
		//fforce_Play( _MenuState.nControllerIndex, FFORCE_EFFECT_MENU_RUMBLE, &_MenuState.hASForceFeedback, FLoop_bGamePaused ? FFORCE_DOMAIN_PAUSED : FFORCE_DOMAIN_NORMAL );			
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
	
	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _AdvSettings_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i, nItem;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS];
	f32 fOptionsX = 0.75f;
	
	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {	
		if( i == _MenuState.nASIndexOfFFBMesh ) {
			fOptionsX = (pScreen->pMesh[i].fBiPolarUnitX + 1.0f) * 0.5f;
#if !WPR_DATATYPES_XBOX_GRAPHICS_ON
			continue;
#endif
		}
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	
	// draw the text
	BOOL bSelected;
	for( i=0; i < pScreen->nNumTextElements; i++ ) {
		if( !fclib_wcsicmp( pScreen->pText[i].pwszText, L"None" ) ) {
			continue;
		}

		if( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_AS_START_OFFSET) ) {
			bSelected = TRUE;
		} else {
			bSelected = FALSE;
		}
		_DrawText( &pScreen->pText[i], bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes );
#if defined(MA_PC_INPUT)
		if( i >= _MENU_ITEMS_AS_START_OFFSET ) {
			_MouseAddItem( (s32)i - _MENU_ITEMS_AS_START_OFFSET, _MOUSE_ITEM_ADJUST );
		}
#endif

		if( i >= _MENU_ITEMS_AS_START_OFFSET ) {
			nItem = i - _MENU_ITEMS_AS_START_OFFSET;
			switch( nItem  ) {

			// draw on / off text
			case _MENU_ITEMS_AS_VIBRATION:
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
				// no text to draw on xbox, it is a bar graph				
#else
				_DrawOnOffSelection( fOptionsX,
									pScreen->pText[i].fUnitY,
									bSelected,
									pScreen->pText[i].fScale,
									_MenuState.nASVibrationTicks );
#endif
				break;
#if WPR_SYSTEM_ALLOW_AUTO_CENTER_CHANGES
			case _MENU_ITEMS_AS_AUTO_CENTER:
				_DrawOnOffSelection( fOptionsX,
									pScreen->pText[i].fUnitY,
									bSelected,
									pScreen->pText[i].fScale,
									_MenuState.bASAutoCenter );
				break;
#endif
			case _MENU_ITEMS_AS_ASSISTED_TARGETING:
				_DrawOnOffSelection( fOptionsX,
									pScreen->pText[i].fUnitY,
									bSelected,
									pScreen->pText[i].fScale,
									_MenuState.bASAssistedTargeting );
				break;

			case _MENU_ITEMS_AS_INVERT:
				_DrawOnOffSelection( fOptionsX,
									pScreen->pText[i].fUnitY,
									bSelected,
									pScreen->pText[i].fScale,
									_MenuState.bASInvertAnalog );				
				break;
							
			case _MENU_ITEMS_AS_LOOK_SENSITIVITY:
				// no text to draw
				break;

			case _MENU_ITEMS_AS_DPAD_COMBO:
				ftext_Printf( fOptionsX,
							pScreen->pText[i].fUnitY,
							L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
							bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
							L'C',
							pScreen->pText[i].fScale, 
							_MenuState.bASFourWayQuickSelect ? _apwszPhrases[WPR_DATATYPES_PHRASES_4WAY] : _apwszPhrases[WPR_DATATYPES_PHRASES_2WAY] );
				break;		
			}
		}
	}	
}

static void _AdvSettings_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS];
	
    // draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_AS_START_OFFSET],
						  &_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],
						  fScaleMultiplier, fHalfXRes, fHalfYRes );
	
	// draw the tick marks for the 2 boxes
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	wpr_drawutils_DrawTickMarks( _MenuState.nASVibrationTicks,
					(_MENU_ITEMS_AS_NUM_TICKS-1),
					&_paTexInsts[WPR_DATATYPES_TEXTURES_TICK],
					_ADV_SETTINGS_TICK_X,
					_ADV_SETTINGS_FFB_TICK_Y,
					_ADV_SETTINGS_TICK_HEIGHT,
					_ADV_SETTINGS_TICK_SPACE, 
 					fScaleMultiplier,
					fHalfXRes,
					fHalfYRes );
#endif

	wpr_drawutils_DrawTickMarks( _MenuState.nASLookSensitivity,
					(_MENU_ITEMS_AS_NUM_TICKS-1),
					&_paTexInsts[WPR_DATATYPES_TEXTURES_TICK],
					_ADV_SETTINGS_TICK_X,
					_ADV_SETTINGS_LOOK_TICK_Y,
					_ADV_SETTINGS_TICK_HEIGHT,
					_ADV_SETTINGS_TICK_SPACE,
 					fScaleMultiplier,
					fHalfXRes,
					fHalfYRes );
}

static void _AdvSettings_ProfileToWorkingVars() {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	_MenuState.bASInvertAnalog = (pProfile->nFlags & GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG);

#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	_MenuState.nASVibrationTicks = (s32)( pProfile->fUnitVibrationIntensity * (_MENU_ITEMS_AS_NUM_TICKS-1) );
#else
	_MenuState.nASVibrationTicks = ( pProfile->fUnitVibrationIntensity > 0.0f ) ? (_MENU_ITEMS_AS_NUM_TICKS-1) : 0;	
#endif
	
	_MenuState.bASAutoCenter = (pProfile->nFlags & GAMESAVE_PROFILE_FLAGS_AUTO_CENTER);
	
	_MenuState.nASLookSensitivity = (s32)( pProfile->fUnitLookSensitivity * (_MENU_ITEMS_AS_NUM_TICKS-1) );
	
	_MenuState.bASFourWayQuickSelect = (pProfile->nFlags & GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT);
	
	_MenuState.bASAssistedTargeting = (pProfile->nFlags & GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING);
}

static void _AdvSettings_WorkingVarsToProfile() {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	if( _MenuState.bASInvertAnalog ) {
		pProfile->nFlags |= GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG;
	} else {
		pProfile->nFlags &= ~GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG;
	}

	pProfile->fUnitVibrationIntensity = _TICKS_TO_PERCENT( _MenuState.nASVibrationTicks, _MENU_ITEMS_AS_NUM_TICKS );
	
	if( _MenuState.bASAutoCenter ) {
		pProfile->nFlags |= GAMESAVE_PROFILE_FLAGS_AUTO_CENTER;
	} else {
		pProfile->nFlags &= ~GAMESAVE_PROFILE_FLAGS_AUTO_CENTER;
	}

	pProfile->fUnitLookSensitivity = _TICKS_TO_PERCENT( _MenuState.nASLookSensitivity, _MENU_ITEMS_AS_NUM_TICKS );

	if( _MenuState.bASFourWayQuickSelect ) {
		pProfile->nFlags |= GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT;
	} else {
		pProfile->nFlags &= ~GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT;
	}
	
	if( _MenuState.bASAssistedTargeting ) {
		pProfile->nFlags |= GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING;
	} else {
		pProfile->nFlags &= ~GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING;
	}
}

static void _AdvSettings_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// save the changes and return to the profile settings screen
		_AdvSettings_WorkingVarsToProfile();
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_ADV_CONTROLLER;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS;

		// flag the profile for saving
		_paProfiles[0].m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// don't save the changes and return to the profile settings screen
		_AdvSettings_ProfileToWorkingVars();
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_ADV_CONTROLLER;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS;
		break;

	default:
		FASSERT_NOW;
		break;
	}
}

////////////////////////////////
// 'profile settings' functions
////////////////////////////////

static Wpr_DataTypes_NavCode_e _ProfileSettings_Work( void ) {

	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}	
	
	// check for up/down changes
	_HandleAxisSelections( FALSE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex, (_MENU_ITEMS_PS_COUNT-1), 
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _ProfileSettings_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_PROFILE_SETTINGS];
	
	// draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_PS_START_OFFSET],
						  &_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
						  fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _ProfileSettings_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// go to the various options screens
		switch( _MenuState.nCurItemIndex ) {

		case _MENU_ITEMS_PS_EDIT_NAME:
			// copy the profile name into the proper spot
			_SafeCopyFStorageName( _MenuState.wszPNCurProfileName, _paProfiles[0].m_SaveInfo.wszProfileName );
			// set the first char as the current item
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;
			_MenuState.bPNLower = TRUE;
			_MenuState.nPNCurCol = 0;
			_MenuState.nPNCurRow = 0;
			break;
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
		case _MENU_ITEMS_PS_EDIT_CONTROLLER:
			_ControllerConfig_ProfileToWorkingVars();
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS;
			break;
#endif
		case _MENU_ITEMS_PS_ADV_CONTROLLER:
			_AdvSettings_ProfileToWorkingVars();
			_MenuState.bASForceFeedbackON = FALSE;
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS;
			break;

		case _MENU_ITEMS_PS_AUDIO:
			_SoundSettings_ProfileToWorkingVars();
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SOUND_OPTIONS;
			break;

		case _MENU_ITEMS_PS_COLOR:
			_PickColor_ProfileToWorkingVars();
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PICK_COLOR;
			break;

		default:
			FASSERT_NOW;
			break;
		}
				
		_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		{
            GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;
			GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

			if( pSaveInfo->nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING ) {
				_SetupMemCardSave( WPR_DATATYPES_SCREENS_LAUNCH, &_paProfiles[0] ); 
			} else {
				// nothing to save, just return to the launch screen
				_MenuState.nCurItemIndex = _MENU_ITEMS_LM_EDIT_SETTINGS;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;
				_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
			}
		}
		break;

	default:
		FASSERT_NOW;
		break;
	}
}

////////////////////////////
// 'profile name' functions
////////////////////////////

static Wpr_DataTypes_NavCode_e _ProfileName_Work( void ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME];

	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

#if defined(MA_PC_INPUT)
	{
		// the key under a moving pointer or a click becomes the selection; a click then types it below in
		// the same frame, so quick clicks on different keys all land
		s32 nNumCells = 0, nRow;
		for( nRow=0; nRow < (s32)_MenuState.nPNNumRows; nRow++ ) {
			nNumCells += _MenuState.anPNColumnsPerRow[nRow];
		}
		s32 nHover = _MouseHoverSelect( _MenuState.nControllerIndex, nNumCells-1 );
		if( nNumCells && nHover >= 0 ) {
			_MenuState.nCurItemIndex = (s8)nHover;
			nRow = 0;
			while( nRow < (s32)_MenuState.nPNNumRows-1 && nHover >= (s32)_MenuState.anPNColumnsPerRow[nRow] ) {
				nHover -= _MenuState.anPNColumnsPerRow[nRow];
				nRow++;
			}
			_MenuState.nPNCurRow = nRow;
			_MenuState.nPNCurCol = nHover;
			ftext_ResetBlinkTimers();
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
		}
	}
#endif
	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		// get the length of the string
		s32 nLenOfString = fclib_wcslen( _MenuState.wszPNCurProfileName );

		if( _MenuState.nPNCurRow == (_MenuState.nPNNumRows-1) ) {
			// make sure that we have a valid profile name
			if( _ProfileName_IsNameValid( _MenuState.wszPNCurProfileName ) ) {
				// done
				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
				return WPR_DATATYPES_NAV_CODE_FORWARD;
			} else {
				// don't allow invalid profile names
				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
			}
		} else if( _MenuState.nPNCurRow == (_MenuState.nPNNumRows-2) ) {
			// this is the action button row
			switch( _MenuState.nPNCurCol ) {
			case _MENU_ITEMS_PN_CAPS_COLUMN:
				// toggle upper/lower case
				_MenuState.bPNLower = !_MenuState.bPNLower;
				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				break;
			case _MENU_ITEMS_PN_SPACE_COLUMN:
				if( nLenOfString >= PROFILE_NAME_MAX_LENGTH ) {
					// there is no more room for an additional letter, play a noise
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
				} else if( nLenOfString == 0 ) {
					// don't allow the first character to be a space
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
				} else {
					// add the current
					fclib_wcscpy( Wpr_DataTypes_wszTempString, L" " );
					fclib_wcscpy( &_MenuState.wszPNCurProfileName[nLenOfString], Wpr_DataTypes_wszTempString );
					nLenOfString++;
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				}
				break;
			case _MENU_ITEMS_PN_BACK_COLUMN:
				if( nLenOfString ) {
					_MenuState.wszPNCurProfileName[nLenOfString-1] = 0;
					nLenOfString--;
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
				}
				break;
			default:
				FASSERT_NOW;
				break;
			}
		} else {
			if( nLenOfString >= PROFILE_NAME_MAX_LENGTH ) {
				// there is no more room for an additional letter, play a noise
				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
			} else {
				// add the current
				fclib_wcscpy( Wpr_DataTypes_wszTempString, 
								pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET].pwszText );
				if( _MenuState.bPNLower ) {
					fclib_wcslwr( Wpr_DataTypes_wszTempString ); 						
				} else {
					fclib_wcsupr( Wpr_DataTypes_wszTempString ); 						
				}
				fclib_wcscpy( &_MenuState.wszPNCurProfileName[nLenOfString], Wpr_DataTypes_wszTempString );
				nLenOfString++;
				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
			}
		}
	} else {

		// check for up/down changes
		_UpDown_e nUpDown = _CheckUpDownAxis( _MenuState.nControllerIndex );
		BOOL bComputeActiveIndex = FALSE;
		u32 i;

		if( nUpDown == _UP ) {
			// try to go up a row
			_MenuState.nPNCurRow--;
			if( _MenuState.nPNCurRow < 0 ) {
				_MenuState.nPNCurRow = _MenuState.nPNNumRows - 1;
			}
			// fix up the current column
			if( _MenuState.nPNCurRow == (_MenuState.nPNNumRows - 2) ) {
				// select the middle button
				_MenuState.nPNCurCol = _MENU_ITEMS_PN_SPACE_COLUMN;
			} else if( _MenuState.nPNCurRow == (_MenuState.nPNNumRows - 3) ) {
				switch( _MenuState.nPNCurCol ) {
				case _MENU_ITEMS_PN_CAPS_COLUMN:
					_MenuState.nPNCurCol = 0;
					break;
				case _MENU_ITEMS_PN_SPACE_COLUMN:
					_MenuState.nPNCurCol = _MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] >> 1;
					break;
				case _MENU_ITEMS_PN_BACK_COLUMN:
					_MenuState.nPNCurCol = _MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] - 1;
					break;
				}
			}

			// make sure that we haven't exceeded the number of columns in the current row
			if( _MenuState.nPNCurCol >= (s32)_MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] ) {
				_MenuState.nPNCurCol = _MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] - 1;
			}
			bComputeActiveIndex = TRUE;
		} else if( nUpDown == _DOWN ) {
			// go down a row
			_MenuState.nPNCurRow++;
			if( _MenuState.nPNCurRow == (_MenuState.nPNNumRows - 2) ) {
				if( _MenuState.nPNCurCol == 0 ) {
					_MenuState.nPNCurCol = _MENU_ITEMS_PN_CAPS_COLUMN;
				} else if( _MenuState.nPNCurCol == (_MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow - 3 ] - 1) ) {
					_MenuState.nPNCurCol = _MENU_ITEMS_PN_BACK_COLUMN;
				} else {
					// make the space bar the new column
					_MenuState.nPNCurCol = _MENU_ITEMS_PN_SPACE_COLUMN;
				}
			}

			if( _MenuState.nPNCurRow >= (s32)_MenuState.nPNNumRows ) {
				_MenuState.nPNCurRow = _MenuState.nPNNumRows - 1;
			}
			if( _MenuState.nPNCurCol >= (s32)_MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] ) {
				_MenuState.nPNCurCol = _MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] - 1;
			}
			bComputeActiveIndex = TRUE;
		}
		if( bComputeActiveIndex ) {
			_MenuState.nCurItemIndex = 0;
			for( i=0; i < (u32)_MenuState.nPNCurRow; i++ ) {
				_MenuState.nCurItemIndex += _MenuState.anPNColumnsPerRow[i];
			}
			_MenuState.nCurItemIndex += _MenuState.nPNCurCol;

			ftext_ResetBlinkTimers();
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
		}

		if( nUpDown == _NOT_UP_OR_DOWN ) {
			// check for left/right changes
			_MenuState.nCurItemIndex -= _MenuState.nPNCurCol;
			_HandleAxisSelections( TRUE, _MenuState.nControllerIndex,
				_MenuState.nPNCurCol,
				_MenuState.anPNColumnsPerRow[ _MenuState.nPNCurRow ] - 1,
				TRUE,
				_ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
			_MenuState.nCurItemIndex += _MenuState.nPNCurCol;
		}
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _ProfileName_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME];
	Wpr_DataTypes_TextLayout_t TempTextLayout;
	
	wpr_system_DrawBasicScreen( pScreen, 
		_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	// draw the control buttons
	i = pScreen->nNumTextElements;
	if( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) {
		// selected
		wpr_drawutils_DrawPhrase( 0.18f, 0.53f, 
					 WprDataTypes_pwszWhiteTextColor,
					 L"L",
					 1.4f,
					 _apwszPhrases[WPR_DATATYPES_PHRASES_CAPS],
					 WprDataTypes_pwszHiLightedBlink );		
	} else {
		// not selected
		wpr_drawutils_DrawPhrase( 0.18f, 0.53f, 
					 ( _MenuState.bPNLower ) ? WprDataTypes_pwszGrayTextColor2 : WprDataTypes_pwszBlueTextColor,
					 L"L",
					 1.4f, 
					 _apwszPhrases[WPR_DATATYPES_PHRASES_CAPS],
					 NULL );
	}
#if defined(MA_PC_INPUT)
	_MouseAddItem( (s32)i - _MENU_ITEMS_PN_START_OFFSET );
#endif
	i++;

	wpr_drawutils_DrawPhrase( 0.50f, 0.53f,
				 ( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor2,
				 L"C",
				 1.4f, 
				 _apwszPhrases[WPR_DATATYPES_PHRASES_SPACE],
				 ( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) ? WprDataTypes_pwszHiLightedBlink : NULL );
#if defined(MA_PC_INPUT)
	_MouseAddItem( (s32)i - _MENU_ITEMS_PN_START_OFFSET );
#endif
	i++;

	wpr_drawutils_DrawPhrase( 0.80f, 0.53f,
				 ( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor2,
				 L"R",
				 1.4f, 
				 _apwszPhrases[WPR_DATATYPES_PHRASES_DELETE],
				 ( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) ? WprDataTypes_pwszHiLightedBlink : NULL );
#if defined(MA_PC_INPUT)
	_MouseAddItem( (s32)i - _MENU_ITEMS_PN_START_OFFSET );
#endif
	i++;

	wpr_drawutils_DrawPhrase( 0.50f, 0.59f,
				 ( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor2,
				 L"C",
				 1.4f, 
				 _apwszPhrases[WPR_DATATYPES_PHRASES_DONE],
				 ( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_PN_START_OFFSET) ) ? WprDataTypes_pwszHiLightedBlink : NULL );
#if defined(MA_PC_INPUT)
	_MouseAddItem( (s32)i - _MENU_ITEMS_PN_START_OFFSET );
#endif
	i++;

	// fill in a temp text layout struct
	TempTextLayout.fScale = 1.0f;
	TempTextLayout.fUnitX = 0.5f;
	TempTextLayout.fUnitY = 0.23f;
	TempTextLayout.nAlignment = WPR_DATATYPES_ALIGN_CENTERED;
	TempTextLayout.nType = WPR_DATATYPES_MESSAGE;
	TempTextLayout.pwszText = Wpr_DataTypes_wszTempString;
	TempTextLayout.fTickSpaceFactor = 17.0f;

	// convert the string from wide char to char
	_SafeCopyFStorageName( Wpr_DataTypes_wszTempString, _MenuState.wszPNCurProfileName );

	// add a cursor to the string
	fclib_wcscat( Wpr_DataTypes_wszTempString, L"~b6|" );

	_DrawText( &TempTextLayout, TRUE, fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _ProfileName_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {
			
	case WPR_DATATYPES_NAV_CODE_FORWARD:

		switch( _MenuState.nLastScreen ) {

		case WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING:
		case WPR_DATATYPES_SCREENS_PROFILE_SELECT:
		case WPR_DATATYPES_SCREENS_ERROR_CREATE:
			// the only way to get here from profile select is "new", just make
			// sure that the name is unique and then create the profile
			if( _DoesNameExistOnCard( pSaveInfo->nStorageDeviceID, 
				_MenuState.wszPNCurProfileName ) ) {
				// the name already exists, you must create a new one
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ALREADY_EXISTS_WARNING;
			} else {
				// the name doesn't exist, copy it to our save info and save to to the card
				_SafeCopyFStorageName( pSaveInfo->wszProfileName, _MenuState.wszPNCurProfileName );
				pSaveInfo->nFlags |= GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW;				
				// init a brand new profile
				_paProfiles[0].InitNewProfile( FALSE );
				_SetupMemCardSave( WPR_DATATYPES_SCREENS_PROFILE_SELECT, &_paProfiles[0] );				
			}
			break;

		case WPR_DATATYPES_SCREENS_PROFILE_SETTINGS:
			// determine if the new name is unchanged or unique
			if( fclib_wcscmp( _MenuState.wszPNCurProfileName, pSaveInfo->wszProfileName ) == 0 ) {
				// there was no change in the name
				_MenuState.nCurItemIndex = _MENU_ITEMS_PS_EDIT_NAME;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
			} else {
				// the name changed

				// does this name already exist
				if( _DoesNameExistOnCard( pSaveInfo->nStorageDeviceID, _MenuState.wszPNCurProfileName ) ) {
					// duplicate name, goto duplicate name warning screen
					_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ALREADY_EXISTS_WARNING;
				} else {
					// the name doesn't exist, delete the old profile and save the new one to the card
					pSaveInfo->nFlags |= GAMESAVE_SAVE_INFO_FLAGS_RENAME;	
					_SetupMemCardSave( WPR_DATATYPES_SCREENS_PROFILE_SETTINGS, &_paProfiles[0] );
					_MenuState.pwszDRNewName = _MenuState.wszPNCurProfileName;
				}
			}
			break;

		default:
			FASSERT_NOW;
			break;
		}		
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// don't save the profile name, revert to the old one
		
		// go to different places, depending on where we came from
		switch( _MenuState.nLastScreen ) {

		case WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING:
		case WPR_DATATYPES_SCREENS_PROFILE_SELECT:
			// set the "create new profile" as the default
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SELECT;			
			break;

		case WPR_DATATYPES_SCREENS_PROFILE_SETTINGS:
            _MenuState.nCurItemIndex = _MENU_ITEMS_PS_EDIT_NAME;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
			break;

		default:
			FASSERT_NOW;
			break;
		}	
		_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;	
		break;

	default:
		FASSERT_NOW;
		break;
	}
}

static BOOL _ProfileName_IsNameValid( cwchar *pwszProfileName ) {
	// get the length of the string
	s32 nLenOfString = fclib_wcslen( _MenuState.wszPNCurProfileName );

	// don't allow 0 length strings
	if( nLenOfString <= 0 ) {
		return FALSE;
	}

	// don't allow the first character to be a space
	if( pwszProfileName[0] == ' ' ) {
		return FALSE;
	}

	// don't allow strings that are only spaces
	s32 i;
	for( i=0; i < nLenOfString; i++ ) {
		if( pwszProfileName[i] != ' ' ) {
			break;
		}
	}
	if( i == nLenOfString ) {
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////
// 'start over' warning functions
//////////////////////////////////

static Wpr_DataTypes_NavCode_e _StartOverWarning_Work( void ) {
	
	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );		
}

static void _StartOverWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// reset the player's level progress back to the first level, but keep their settings
		_paProfiles[0].ResetToBeginning();
		pSaveInfo->nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;

		_MenuState.bLMNewProfile = (_paProfiles[0].m_Data.nCurrentLevel == 0);

		// pick the game difficulty level
		_MenuState.nCurItemIndex = _paProfiles[0].m_Data.nDifficulty;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL;		
		_MenuState.nStartGameMethod = _START_METHOD_NEW;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// don't start over, return to the launch screen
		_MenuState.nCurItemIndex = _MENU_ITEMS_LM_START_NEW;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;		
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_START_OVER;
}

///////////////////////////////
// 'no save' warning functions
///////////////////////////////

static Wpr_DataTypes_NavCode_e _NoSaveWarning_Work( void ) {

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );	
}

static void _NoSaveWarning_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// ok, continue forward with no saving, create a default profile
		_paProfiles[0].InitNewProfile( TRUE );	
		
		// pick the difficulty
		_MenuState.nCurItemIndex = _paProfiles[0].m_Data.nDifficulty;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL;
		_MenuState.fModeTimer = 0.0f;// reset the timer
		_MenuState.nStartGameMethod = _START_METHOD_NEW;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// the user wants to pick a memory card
		_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_NO_SAVE_WARNING;
}

//////////////////////////
// 'launch menu' functions
//////////////////////////

static Wpr_DataTypes_NavCode_e _LaunchMenu_Work( void ) {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	// check for the code to open all levels 
	_MenuState.nLMCodeState = _HandleUnlockLevelsCode( _MenuState.nLMCodeState, _MenuState.nControllerIndex );
	if( _MenuState.nLMCodeState != _LAUNCH_CODE_NOT_STARTED &&
		_MenuState.nLMCodeState != _LAUNCH_CODE_ENTERED ) {
		// in the middle of a code entry, don't accept other input
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
		return WPR_DATATYPES_NAV_CODE_NOTHING;
	}

	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}
	s32 nCache = _MenuState.nCurItemIndex;

	// check for up/down changes
	_UpDown_e nUpDown = _CheckUpDownAxis( _MenuState.nControllerIndex );
	if( nUpDown == _UP ) {
		_MenuState.nCurItemIndex--;
		if( _MenuState.nCurItemIndex < 0 ) {
			_MenuState.nCurItemIndex = 0;
		}
		if( _MenuState.bLMNewProfile ) {
			// both continue & replay are disabled
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_START_NEW;
		}
	} else if( nUpDown == _DOWN ) {
		_MenuState.nCurItemIndex++;
		if( _MenuState.nCurItemIndex >= _MENU_ITEMS_LM_COUNT ) {
			_MenuState.nCurItemIndex = (_MENU_ITEMS_LM_COUNT-1);
		}

		if( _MenuState.bLMNewProfile ) {
			// both continue & replay are disabled
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_EDIT_SETTINGS;
		}
	}
#if defined(MA_PC_INPUT)
	s32 nHover = _MouseHoverSelect( _MenuState.nControllerIndex, _MENU_ITEMS_LM_COUNT-1 );
	if( nHover >= 0 ) {
		_MenuState.nCurItemIndex = (s8)nHover;
	}
#endif
	if( pProfile->nFlags & GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER &&
		_MenuState.nCurItemIndex == _MENU_ITEMS_LM_CONTINUE ) {
		// continue is disabled
		_MenuState.nCurItemIndex += nUpDown;
	}
	if( nCache != _MenuState.nCurItemIndex ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _LaunchMenu_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_LAUNCH];
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;
	
	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {		
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	
	// draw the text
	BOOL bSelected, bDisabled;
	u32 nIndex;
	for( i=0; i < pScreen->nNumTextElements; i++ ) {
		if( i == (_MenuState.nCurItemIndex + _MENU_ITEMS_LM_START_OFFSET) ) {
			bSelected = TRUE;
		} else {
			bSelected = FALSE;
		}

		bDisabled = FALSE;
		if( _MenuState.bLMNewProfile ) {
			// disable continue & replay
			if( i >= _MENU_ITEMS_LM_START_OFFSET ) {
				nIndex = i - _MENU_ITEMS_LM_START_OFFSET;
				if( nIndex == _MENU_ITEMS_LM_CONTINUE ||
					nIndex == _MENU_ITEMS_LM_REPLAY ) {
					bDisabled = TRUE;
				}
			}
		} else if( pProfile->nFlags & GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER ) {
			// the user has finished the game, don't offer to continue
			if( i >= _MENU_ITEMS_LM_START_OFFSET ) {
				nIndex = i - _MENU_ITEMS_LM_START_OFFSET;
				if( nIndex == _MENU_ITEMS_LM_CONTINUE ) {
					bDisabled = TRUE;
				}
			}
		}

		_DrawText( &pScreen->pText[i], bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes, bDisabled );
#if defined(MA_PC_INPUT)
		if( i >= _MENU_ITEMS_LM_START_OFFSET && !bDisabled ) {
			_MouseAddItem( (s32)i - _MENU_ITEMS_LM_START_OFFSET );
		}
#endif
	}
}

static void _LaunchMenu_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_LAUNCH];
	
	// draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_LM_START_OFFSET],
						  &_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
						  fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _LaunchMenu_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// go to the various launch screens
		switch( _MenuState.nCurItemIndex ) {

		case _MENU_ITEMS_LM_START_NEW:
			if( !_MenuState.bLMNewProfile ) {
				// the user wants to start over, confirm this
				_MenuState.nCurItemIndex = 0;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_START_OVER;
			} else {
				// pick the difficulty level
				_MenuState.fModeTimer = 0.0f;// reset the timer
				_MenuState.nCurItemIndex = _paProfiles[0].m_Data.nDifficulty;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL;				
				_MenuState.nStartGameMethod = _START_METHOD_NEW;
			}
			break;

		case _MENU_ITEMS_LM_CONTINUE:
			// the user wants to continue their game, see if we need to save their profile
			if( pSaveInfo->nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING ) {
				_SetupMemCardSave( WPR_DATATYPES_SCREENS_NONE, &_paProfiles[0] );
			} else {
				// start the game
				_MenuState.nCurItemIndex = 0;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NONE;
				_MenuState.fModeTimer = 0.0f;// reset the timer
			}
			_MenuState.nStartGameMethod = _START_METHOD_CONTINUE;
			break;

		case _MENU_ITEMS_LM_REPLAY:
			// allow the user to select completed levels, make the last level the current selection
			_MenuState.nCurItemIndex = pProfile->nCurrentLevel - 1;// should only be on this screen if there has been progress made
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PICK_LEVEL;
			break;
			
		case _MENU_ITEMS_LM_EDIT_SETTINGS:
			// go to the profile settings screen
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;	
			break;

		default:
			FASSERT_NOW;
			break;
		}
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// return to profile select
		
		// find the selected profile and make that the current selected
		_SelectProfile_SetupReturnVisit( _MenuState.pPPDevInfo, pSaveInfo->wszProfileName );
		break;

	default:
		FASSERT_NOW;
		break;
	}

	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_LAUNCH;	
}

// the Code is: RTrigger Down + A then B then X then Y
static _LaunchCode_e _HandleUnlockLevelsCode( _LaunchCode_e nCurState, u32 nControllerID ) {
	
#if FANG_ENABLE_DEV_FEATURES
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;
	u32 i;
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	s32 nLevelIndex;

	switch( nCurState ) {

	case _LAUNCH_CODE_NOT_STARTED:
		// look for the left trigger up, right trigger down
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState <= 0.1f ) {
			return _LAUNCH_CODE_TRIGGER_DOWN;
		}
		break;

	case _LAUNCH_CODE_TRIGGER_DOWN:
		// make sure that the trigger is still down, and NOT B, X, Y
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState <= 0.1f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK]->fCurrentState < 0.5f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_SIDE]->fCurrentState < 0.5f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_TOP]->fCurrentState < 0.5f ) {
			// see if the A button is down
			if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_ACCEPT]->fCurrentState >= 0.5f ) {
				return _LAUNCH_CODE_A_DOWN;
			}				
		} else {
			return _LAUNCH_CODE_NOT_STARTED;
		}
		break;
		
	case _LAUNCH_CODE_A_DOWN:
		// make sure that the trigger is still down
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState <= 0.1f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_SIDE]->fCurrentState < 0.5f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_TOP]->fCurrentState < 0.5f ) {
			// see if the B button is down
			if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK]->fCurrentState >= 0.5f ) {
				return _LAUNCH_CODE_B_DOWN;
			}				
		} else {
			return _LAUNCH_CODE_NOT_STARTED;
		}
		break;

	case _LAUNCH_CODE_B_DOWN:
		// make sure that the trigger is still down
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState <= 0.1f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_ACCEPT]->fCurrentState < 0.5f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_TOP]->fCurrentState < 0.5f ) {
			// see if the X button is down
			if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_SIDE]->fCurrentState >= 0.5f ) {
				return _LAUNCH_CODE_X_DOWN;
			}				
		} else {
			return _LAUNCH_CODE_NOT_STARTED;
		}
		break;

	case _LAUNCH_CODE_X_DOWN:
		// make sure that the trigger is still down
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState <= 0.1f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_ACCEPT]->fCurrentState < 0.5f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK]->fCurrentState < 0.5f ) {
			// see if the Y button is down
			if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_TOP]->fCurrentState >= 0.5f ) {
				return _LAUNCH_CODE_Y_DOWN;
			}				
		} else {
			return _LAUNCH_CODE_NOT_STARTED;
		}
		break;

	case _LAUNCH_CODE_Y_DOWN:
		// make sure that the trigger is still down
		if( Gamepad_aapSample[nControllerID][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState <= 0.1f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_ACCEPT]->fCurrentState < 0.5f &&
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK]->fCurrentState < 0.5f && 
			Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BUTTON_SIDE]->fCurrentState < 0.5f ) {
			
			// the code has been entered, open up all of the levels
			pProfile->nCurrentLevel = LEVEL_SINGLE_PLAYER_COUNT;
			pProfile->nFlags |= GAMESAVE_PROFILE_FLAGS_100_PERCENT_FINISHED | GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER;
			pProfile->nNumMPLevelsUnlocked = LEVEL_MULTIPLAYER_COUNT - _MenuState.MLUnLockInfo.nNumFreebies;
			pProfile->nTotalSecretChipsGathered = _MenuState.MLUnLockInfo.panChipsToUnlock[pProfile->nNumMPLevelsUnlocked - 1];

			nLevelIndex = level_Find( LEVEL_SEAL_THE_MINES_1 );

			{
				BOOL bSetToDefaults;
				CInventory TempInv;
				CMemCardInventory MCInvInitial, MCInvDefaults;

				TempInv.SetToInitial();
				CMemCardInventory::SaveInventory( &MCInvInitial, &TempInv );

				TempInv.SetToDefaults();
				CMemCardInventory::SaveInventory( &MCInvDefaults, &TempInv );

				for( i=0; i < pProfile->nCurrentLevel; i++ ) {
					pProfile->aLevelProgress[i].fTimeToComplete = fmath_RandomFloatRange( 200.0f, 5000.0f );
					pProfile->aLevelProgress[i].nWashersCollected = 0;
					pProfile->aLevelProgress[i].nSecretChipsCollected = 0;
					pProfile->aLevelProgress[i].nEnemiesKilled = 0;
					pProfile->aLevelProgress[i].nBonusSecretChipEarned = 0;

					// grab a memory frame
					hMemFrame = fmem_GetFrame();
					
					bSetToDefaults = TRUE;
                    // see if the level csv file defines an inventory, load the level csv file
					hFile = (nLevelIndex >= 0) ? fgamedata_LoadFileToFMem( Level_aInfo[nLevelIndex + i].pszCSVFile ) : FGAMEDATA_INVALID_FILE_HANDLE;
					if( hFile != FGAMEDATA_INVALID_FILE_HANDLE ) {
						// find the inventory table
						hTable = fgamedata_GetFirstTableHandle( hFile, LEVEL_INVENTORY_TABLE_NAME );
						if( TempInv.InitFromCSVTable( hTable ) ) {
							CMemCardInventory::SaveInventory( &pProfile->aLevelProgress[i].Inventory, &TempInv );
							bSetToDefaults = FALSE;
						}
					}
					if( bSetToDefaults ) {							
						if( i == 0 ) {
							// set the initial
							pProfile->aLevelProgress[i].Inventory = MCInvInitial;
						} else {
							pProfile->aLevelProgress[i].Inventory = MCInvDefaults;
						}
					}
			
					// release the memory frame
					fmem_ReleaseFrame( hMemFrame );
				}		
			}

			_MenuState.bLMNewProfile = FALSE;
			_paProfiles[0].m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			return _LAUNCH_CODE_ENTERED;			
		} else {
			return _LAUNCH_CODE_NOT_STARTED;
		}
		break;

	case _LAUNCH_CODE_ENTERED:
		break;

	default:
		FASSERT_NOW;
		break;
	}
#endif
	return nCurState;
}

//////////////////////////
// 'pick level' functions
//////////////////////////

static Wpr_DataTypes_NavCode_e _PickLevel_Work( void ) {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}
	
	// allow the level selection to be changed (L/R changes)
	_HandleAxisSelections( TRUE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex,
		pProfile->nCurrentLevel-1,
		TRUE,
		_ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );		

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _PickLevel_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_PICK_LEVEL];
	GameSave_LevelProgress_t *pLevelToReplay = &_paProfiles[0].m_Data.aLevelProgress[_MenuState.nCurItemIndex];
	BOOL b100PercentComplete;

	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	// print "Level 36:"
	ftext_Printf( 0.16f, 0.22f,
				  L"~f1~C%ls~w0~a%lc~s%.2f%ls %d:", 
				  WprDataTypes_pwszBlueTextColor,
				  L'L',
				  1.0f, 
				  _apwszPhrases[WPR_DATATYPES_PHRASES_LEVEL],
				  _MenuState.nCurItemIndex+1 );
	// print the "location: level name"
	ftext_Printf( 0.16f, 0.25f,
				  L"~f1~C%ls~w0~a%lc~s%.2f%ls: ~C%ls%ls", 
				  WprDataTypes_pwszBlueTextColor,
				  L'L',
				  0.85f, 
				  _MenuState.PLSelectInfo.papwzLocations[_MenuState.nCurItemIndex],
				  WprDataTypes_pwszWhiteTextColor,
				  _MenuState.PLSelectInfo.papwzNames[_MenuState.nCurItemIndex] );

	f32 fX = 0.64f;
	f32 fY = 0.30f;
	f32 fDeltaY = 0.06f;
	f32 fScale = 0.75f;

	// determine if the player has 100% completed the level yet
	b100PercentComplete = FALSE;
	if( !_MenuState.PLSelectInfo.panSecretChips[_MenuState.nCurItemIndex] &&
		_MenuState.PLSelectInfo.pafTimeToBeat[_MenuState.nCurItemIndex] == 0.0f ) {
		// there are no secret chips to collect and no bonus chip possible
		b100PercentComplete = TRUE;
	} else if( pLevelToReplay->nSecretChipsCollected == _MenuState.PLSelectInfo.panSecretChips[_MenuState.nCurItemIndex] ) {
		// the player has collected all of the chips, have they earned the bonus
		if( _MenuState.PLSelectInfo.pafTimeToBeat[_MenuState.nCurItemIndex] > 0.0f ) {
			if( pLevelToReplay->nBonusSecretChipEarned ) {
				// the player has earned the available bonus chip
				b100PercentComplete = TRUE;
			}
		} else {
			// there is no bonus chip to earn
			b100PercentComplete = TRUE;
		}
	}	

	////////////////////////////////////////
	// draw the right side "Collected" stats
	
	// print "Secret Chips: 1 / 3"
	ftext_Printf( fX, fY,
				L"~f1~C%ls~w0~a%lc~s%.2f%ls:\n~C%ls%d / %d", 
				WprDataTypes_pwszBlueTextColor,
				L'L',
				fScale, 
				_apwszPhrases[WPR_DATATYPES_PHRASES_SECRET_CHIPS],
				WprDataTypes_pwszWhiteTextColor,
				pLevelToReplay->nSecretChipsCollected,
				_MenuState.PLSelectInfo.panSecretChips[_MenuState.nCurItemIndex] );
	
	if( b100PercentComplete ) {
		// draw "level complete" under the center screen shot
		ftext_Printf( 0.5f, 0.55f,
				L"~f1~C%ls%ls~w0~a%lc~s%.2f%ls %ls", 
				WprDataTypes_pwszBlueTextColor,
				WprDataTypes_pwszHiLightedBlink,
				L'C',
				0.80f, 
				_apwszPhrases[WPR_DATATYPES_PHRASES_LEVEL],
				_apwszPhrases[WPR_DATATYPES_PHRASES_COMPLETE] );
	}

	////////////////////////////////////////
	// draw the left side "Stats" stats

	fX = 0.16f;
	fY = 0.30f;
	fDeltaY = 0.05f;
	fScale = 0.75f;
	
	// print "Best Time:"
	u32 nSecs, nHours, nMins;
	wpr_datatypes_BreakDownTimeUnits( pLevelToReplay->fTimeToComplete, nHours, nMins, nSecs );
	ftext_Printf( fX, fY,
		          L"~f1~C%ls~w0~a%lc~s%.2f%ls:\n~C%ls%ls~w0%02d~w1:~w0%02d~w1.~w0%d", 
				  WprDataTypes_pwszBlueTextColor,
				  L'L',
				  fScale, 
				  _apwszPhrases[WPR_DATATYPES_PHRASES_FINISHED_IN],
				  WprDataTypes_pwszWhiteTextColor,	
				  ( pLevelToReplay->nBonusSecretChipEarned ) ? WprDataTypes_pwszHiLightedBlink : L"~B0",
				  nHours, nMins, nSecs );
	fY += (1.5f * fDeltaY);

	if( _MenuState.PLSelectInfo.pafTimeToBeat[_MenuState.nCurItemIndex] > 0.0f ) {
		// there is a possible bonus chip
        if( pLevelToReplay->nBonusSecretChipEarned ) {
			// the player earned the bonus chip already
			ftext_Printf( fX, fY,
						L"~f1%ls~C%ls~w0~a%lc~s%.2f%ls", 
						WprDataTypes_pwszHiLightedBlink,
						WprDataTypes_pwszBlueTextColor,
						L'L',
						fScale, 
						_apwszPhrases[WPR_DATATYPES_PHRASES_BONUS_SECRET_CHIP_EARNED] );			
		} else {
			// draw the time to beat
			wpr_datatypes_BreakDownTimeUnits( _MenuState.PLSelectInfo.pafTimeToBeat[_MenuState.nCurItemIndex],
											  nHours, nMins, nSecs );
			ftext_Printf( fX, fY,
						L"~f1~C%ls~w0~a%lc~s%.2f%ls:\n~C%ls%ls~w0%02d~w1:~w0%02d~w1.~w0%d", 
						WprDataTypes_pwszBlueTextColor,
						L'L',
						fScale, 
						_apwszPhrases[WPR_DATATYPES_PHRASES_TIME_TO_BEAT],
						WprDataTypes_pwszWhiteTextColor,	
						( pLevelToReplay->nBonusSecretChipEarned ) ? WprDataTypes_pwszHiLightedBlink : L"~B0",
						nHours, nMins, nSecs );
		}
	}
}

static void _PickLevel_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	// draw the current screenshot
	wpr_drawutils_DrawTextureToScreen( TRUE,
					 &_MenuState.PLSelectInfo.paTexInsts[_MenuState.nCurItemIndex],
					 -0.0f, -0.10f,
					 0.65f,
                     fScaleMultiplier, fHalfXRes, fHalfYRes );
	// draw the inactive screenshots
	if( pProfile->nCurrentLevel > 1 ) {
		s32 nLeftIndex = _MenuState.nCurItemIndex - 1;
		if( nLeftIndex < 0 ) {
			// draw the last item
			nLeftIndex = (pProfile->nCurrentLevel - 1);
		}
		wpr_drawutils_DrawTextureToScreen( FALSE,
						 &_MenuState.PLSelectInfo.paTexInsts[nLeftIndex],
						 -0.45f, -0.475f,
						 0.35f,
						 fScaleMultiplier, fHalfXRes, fHalfYRes );

		s32 nRightIndex = _MenuState.nCurItemIndex + 1;
		if( nRightIndex >= pProfile->nCurrentLevel ) {
			nRightIndex = 0;
		}
		if( nRightIndex != nLeftIndex ) {
			wpr_drawutils_DrawTextureToScreen( FALSE,
							 &_MenuState.PLSelectInfo.paTexInsts[nRightIndex],
							 0.45f, -0.475f,
							 0.35f,
							 fScaleMultiplier, fHalfXRes, fHalfYRes );
		}		
	}

	// draw left right arrows
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	wpr_drawutils_DrawLeftRightArrow( -0.61f,
						 0.61f,
						 -0.36f,
						 45.0f,
						 fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _PickLevel_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// the user wants to continue their game, see if we need to save their profile
		_MenuState.nLevelToPlay = (u8)_MenuState.nCurItemIndex;
		if( pSaveInfo->nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING ) {
			_SetupMemCardSave( WPR_DATATYPES_SCREENS_NONE, &_paProfiles[0] );
		} else {
			// start the game
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NONE;
			_MenuState.fModeTimer = 0.0f;// reset the timer
		}
		_MenuState.nStartGameMethod = _START_METHOD_REPLAY;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// back to the launch screen
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;
		_MenuState.nCurItemIndex = _MENU_ITEMS_LM_REPLAY;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_PICK_LEVEL;
}

//////////////////////
// 'controller config'
//////////////////////

static Wpr_DataTypes_NavCode_e _ControllerConfig_Work( void ) {

	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
	// change the current controller config (L/R changes)
	_HandleAxisSelections( TRUE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex,
		_MenuState.CCData.nCCNumConfigs - 1,
		TRUE,
		_ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
#endif

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _ControllerConfig_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS];
	Wpr_DataTypes_ControllerConfig_t *pConfig = &_MenuState.CCData.paCCConfigs[ _MenuState.nCurItemIndex ];

	wpr_system_ControllerConfig_DrawOrtho( pScreen, 
										pConfig,
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
										_apwszPhrases[WPR_DATATYPES_PHRASES_CONFIGURATION],
#else
										NULL,
#endif
										_MenuState.nCurItemIndex,
                                        fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _ControllerConfig_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ControllerConfig_t *pConfig = &_MenuState.CCData.paCCConfigs[ _MenuState.nCurItemIndex ];

	wpr_system_ControllerConfig_DrawFDraw( pConfig, 
		&_paTexInsts[WPR_DATATYPES_TEXTURES_CONTROLLER],
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
		TRUE,
#else
		FALSE,
#endif
		fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _ControllerConfig_ProfileToWorkingVars() {
	_MenuState.nCurItemIndex = _paProfiles[0].m_Data.nControllerConfigIndex;
}

static void _ControllerConfig_WorkingVarsToProfile() {
	_paProfiles[0].m_Data.nControllerConfigIndex = (u8)_MenuState.nCurItemIndex;
}

static void _ControllerConfig_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_BACK:
		// lose changes and go back to the profile settings
		_ControllerConfig_ProfileToWorkingVars();
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_EDIT_CONTROLLER;
#endif
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// save changes and go back to profile settings
		_ControllerConfig_WorkingVarsToProfile();
#if WPR_SYSTEM_ALLOW_CONTROLLER_CONFIG_CHANGES
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_EDIT_CONTROLLER;
#endif
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		
		// flag the profile for saving
		pSaveInfo->nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
		break;	

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS;	
}

static Wpr_DataTypes_ControllerTextPos_t *_FindControllerTextPos( cchar *pszKey, u32 nNumInList, Wpr_DataTypes_ControllerTextPos_t *paList ) {
	u32 i;

	for( i=0; i < nNumInList; i++ ) {
		if( fclib_stricmp( pszKey, paList[i].pszKeyString ) == 0 ) {
			return &paList[i];
		}
	}
	return NULL;
}

///////////////
// 'select mu'
///////////////

static Wpr_DataTypes_NavCode_e _SelectMU_Work( void ) {

#if !WPR_DATATYPES_XBOX_GRAPHICS_ON
	u32 i;
	// Check to see if we have a memory card inserted thats corrupted or incorrectly formated
	for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
		// Search for unusable offered devices that have a status of corrupted
		if( _MenuState.paMUEntries[i+1].bOfferDevice && _MenuState.paMUEntries[i+1].bDeviceUnusable &&
			( _MenuState.paMUEntries[i+1].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_ENCODING ) &&
			!_MenuState.paMUEntries[i+1].bFormatOffered ) {
				// We need to inform this guy immediately that he needs a format...
				_MenuState.nFCEntryArrayID = i+1;
				_MenuState.pwszFCDeviceName = _MenuState.paMUEntries[i+1].pDeviceInfo->wszName;
				_MenuState.eFCMemCardID = _MenuState.paMUEntries[i+1].pDeviceInfo->oeID;	
				return WPR_DATATYPES_NAV_CODE_CONFIRM;
		}
	}
#endif

	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

	if( (_MenuState.nButtonDrawMask == WPR_DATATYPES_DRAW_ABY_BUTTONS) && _CheckYButton( _MenuState.nControllerIndex ) ) {
		
		// Make sure this device is still available, and that it needs formatting!
		if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice && 
			(_MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_ENCODING ) ) {

			// Use the format card variables for the confirm format screen as well
			_MenuState.nFCEntryArrayID = _MenuState.nCurItemIndex;
			_MenuState.pwszFCDeviceName = _MenuState.paMUEntries[ _MenuState.nCurItemIndex ].pDeviceInfo->wszName;
			_MenuState.eFCMemCardID = _MenuState.paMUEntries[ _MenuState.nCurItemIndex ].pDeviceInfo->oeID;	

			return WPR_DATATYPES_NAV_CODE_CONFIRM;
		} else {
			// This memory card is full...
			return WPR_DATATYPES_NAV_CODE_ALTERNATE;
		}		
	}

	s32 nCache = _MenuState.nCurItemIndex;

	_LeftRight_e nLeftRight = _CheckLeftRightAxis( _MenuState.nControllerIndex );

	// do some tests to see if we have an error condition that should be handled
	BOOL bForceUpdate = FALSE;
	if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo ) {
		u32 nBytes, nBlocks;
		if( fstorage_CalcSaveGameSize( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->oeID,
									sizeof( GameSave_ProfileData_t ),
									&nBytes ) != FSTORAGE_ERROR_NONE ) {
			bForceUpdate = TRUE;
		} else if( fstorage_BytesToBlocks( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->oeID,
										nBytes,
										&nBlocks ) != FSTORAGE_ERROR_NONE ) {
			bForceUpdate = TRUE;
		}
	} else {
		// do a full check every 32 frames
		bForceUpdate = ((FVid_nFrameCounter & 0x1F) == 0x1F);
	}		
	_MUSelectUpdate( _MenuState.paMUEntries, bForceUpdate );

	if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
		do {
            _MenuState.nCurItemIndex += nLeftRight;

			if( _MenuState.nCurItemIndex >= (s32)_MenuState.nMUNumEntries ) {
				_MenuState.nCurItemIndex = 0;
			} else if( _MenuState.nCurItemIndex < 0 ) {
				_MenuState.nCurItemIndex = _MenuState.nMUNumEntries - 1;
			}

		} while( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice );			
	} else {
		// we need to make sure that the card we had selected is still available
		if( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice ) {
			// pick a new selected index
            _MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
		}
	}
	if( nCache != _MenuState.nCurItemIndex ) {
		ftext_ResetBlinkTimers();
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
	}


	if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo &&
        !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bDeviceUnusable &&
	    !_IsSpaceAvailForProfile( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo ) ) {
		// There is no space available for the profile, so provide the option to manage memory
		
		// Remap the y button text to pwszMuManageMemory
		Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ].paButtons[ 2 ].pwszInstructions = _apwszPhrases[WPR_DATATYPES_PHRASES_MANAGEMEMORY];
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABY_BUTTONS;
		
#if !WPR_DATATYPES_XBOX_GRAPHICS_ON
	} else if( (_MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo) &&
		     (_MenuState.paMUEntries[_MenuState.nCurItemIndex].bDeviceUnusable) &&
		     (_MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_ENCODING ) ) {
		// Remap the y button text to Format
		Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ].paButtons[ 2 ].pwszInstructions = _apwszPhrases[WPR_DATATYPES_PHRASES_FORMAT];
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABY_BUTTONS;
#endif
	} else {
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _SelectMU_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 nBlocks, nBlocksNeededToSaveProfile, nBytesNeededToSaveProfile;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT];
	Wpr_DataTypes_MUSelectEntry_t *pEntry = &_MenuState.paMUEntries[_MenuState.nCurItemIndex];
	BOOL bPrintDeviceName, bWarnAboutNewProfiles;
			
	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	ftext_Printf( _MU_SELECT_DISPLAY_NAME_X, _MU_SELECT_DISPLAY_NAME_Y,
			L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
			pEntry->bDeviceUnusable ? WprDataTypes_pwszGrayTextColor2 : WprDataTypes_pwszWhiteTextColor,
			L'C',
			_MU_SELECT_DISPLAY_NAME_SCALE,
			pEntry->pwszDisplayName );
#if defined(MA_PC_INPUT)
	_MouseAddItem( _MenuState.nCurItemIndex );
	_MouseAddArrowZones( 0.08f );
#endif
	
	if( pEntry->pDeviceInfo ) {		
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
		bPrintDeviceName = (pEntry->pDeviceInfo->oeID != FSTORAGE_DEVICE_ID_XB_PC_HD);
#else
		bPrintDeviceName = FALSE;
#endif
		// print customized device name
		if( bPrintDeviceName ) {
			_SafeCopyFStorageName( Wpr_DataTypes_wszTempString, pEntry->pDeviceInfo->wszName, _MAX_MU_NAME_DISPLAY_LEN );
			ftext_Printf( _MU_SELECT_DISPLAY_NAME_X, _MU_SELECT_CUSTOM_NAME_Y,
						L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
						WprDataTypes_pwszWhiteTextColor,
						L'C',
						1.0f, 
						Wpr_DataTypes_wszTempString );
		}
		
		// print the number of free blocks
		if( fstorage_BytesToBlocks( pEntry->pDeviceInfo->oeID, (u32)pEntry->pDeviceInfo->uBytesAvailable, &nBlocks ) != FSTORAGE_ERROR_NONE ) {
			nBlocks = 0;			
		}		
		ftext_Printf( _MU_SELECT_STAT_X, pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_BLOCKS_FREE].fUnitY,
					  L"~f1~C%ls~w0~a%lc~s%.2f%d%lc", 
					  WprDataTypes_pwszGrayTextColor,
					  L'L',
					  pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_BLOCKS_FREE].fScale, 
					  ( nBlocks <= 50000 ) ? nBlocks : 50000,
					  ( nBlocks > 50000 ) ? '+' : ' ' );
		
		// print the number of saved profiles
		ftext_Printf( _MU_SELECT_STAT_X, pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_PROFILES_SAVED].fUnitY,
					  L"~f1~C%ls~w0~a%lc~s%.2f%d", 
					  WprDataTypes_pwszGrayTextColor,
					  L'L',
					  pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_PROFILES_SAVED].fScale, 
					  pEntry->pDeviceInfo->uNumProfiles );
					  
		// print a warning if need be
		if( pEntry->bDeviceUnusable ) {
			if( pEntry->pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_DAMAGED ) {
				ftext_Printf( 0.5f, 0.48f, L"~B9~f1~C%ls~w0~a%lc~s%.2f%ls", 
					WprDataTypes_pwszInstructionTextColor,
					L'C',
					1.05f,  
					_apwszPhrases[WPR_DATATYPES_PHRASES_DAMAGED_MEMORY_CARD_WARNING] );
			} else if( pEntry->pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_DEVICE ) {
				ftext_Printf( 0.5f, 0.48f, L"~B9~f1~C%ls~w0~a%lc~s%.2f%ls\n%ls", 
					WprDataTypes_pwszInstructionTextColor,
					L'C',
					1.05f,  
					_apwszPhrases[WPR_DATATYPES_PHRASES_WRONG_DEVICE],
					_apwszPhrases[WPR_DATATYPES_PHRASES_INSERT_MEMORY_CARD] );
			} else {
				// unusable device
				ftext_Printf( 0.5f, 0.48f,
							L"~B9~f1~C%ls~w0~a%lc~s%.2f%ls", 
							WprDataTypes_pwszInstructionTextColor,
							L'C',
							1.05f, 
							_apwszPhrases[WPR_DATATYPES_PHRASES_MU_UNUSABLE] );
			}
		} else {
			// see if we there is enough free space for a new profile to be saved
			if( fstorage_CalcSaveGameSize( pEntry->pDeviceInfo->oeID, sizeof( GameSave_ProfileData_t ), &nBytesNeededToSaveProfile ) != FSTORAGE_ERROR_NONE ) {
				bWarnAboutNewProfiles = TRUE;
			} else if( fstorage_BytesToBlocks( pEntry->pDeviceInfo->oeID, nBytesNeededToSaveProfile, &nBlocksNeededToSaveProfile ) != FSTORAGE_ERROR_NONE ) {
				bWarnAboutNewProfiles = TRUE;
			} else {
				bWarnAboutNewProfiles = (nBlocksNeededToSaveProfile > nBlocks );
#if !WPR_DATATYPES_XBOX_GRAPHICS_ON
				// on GC we could be out of files too
				if( !bWarnAboutNewProfiles ) {
					// see if we have enough free files
					u32 nRetStatus;
					if( fstorage_CanSaveNewProfile( pEntry->pDeviceInfo->oeID, sizeof( GameSave_ProfileData_t ), &nRetStatus ) != FSTORAGE_ERROR_NONE ) {
						bWarnAboutNewProfiles = TRUE;
					} else {
						bWarnAboutNewProfiles = (nRetStatus & (FSTORAGE_DEVICE_STATUS_NO_FREE_MEM | FSTORAGE_DEVICE_STATUS_NO_FREE_FILE_ENTRIES));
					}
				}
#endif				
			}
			if( bWarnAboutNewProfiles ) {
				ftext_Printf( 0.5f, 0.45f,
							L"~B9~f1~C%ls~w0~a%lc~s%.2f%ls", 
							WprDataTypes_pwszInstructionTextColor,
							L'C',
							0.75f, 
							_apwszPhrases[WPR_DATATYPES_PHRASES_NO_ROOM_FOR_NEW_PROFILE] );
				// print out a platform specific message about not have enough room for a saved game			
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
				u32 nAdditionalBlocksNeeded = nBlocksNeededToSaveProfile - nBlocks;
				if( nAdditionalBlocksNeeded > 1 ) {
					ftext_Printf( 0.5f, 0.5f, L"~f1~C%ls~w0~a%lc~s%.2f%ls %d %ls", 
								WprDataTypes_pwszMessageTextColor,
								 L'C',
								  0.95f,  
								  _apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_0],
								  nAdditionalBlocksNeeded,
								   _apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_1] );
				} else {
					ftext_Printf( 0.5f, 0.5f, L"~f1~C%ls~w0~a%lc~s%.2f%ls %d %ls", 
								WprDataTypes_pwszMessageTextColor, 
								L'C',
								 0.95f,  
								 _apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_0],
								  nAdditionalBlocksNeeded,
								   _apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_2] );
				}
#else
				// always print out how many blocks and files we need
				// print out how many blocks and file MA uses
				ftext_Printf( 0.5f, 0.49f, L"~f1~C%ls~w0~a%lc~s%.2f%ls %ls\n%d %ls", 
					WprDataTypes_pwszMessageTextColor, 
					L'C',
					1.06f,  
					_apwszPhrases[WPR_DATATYPES_PHRASES_MA_NEEDS_0],
					_apwszPhrases[WPR_DATATYPES_PHRASES_MA_NEEDS_1],
					nBlocksNeededToSaveProfile,					
					_apwszPhrases[WPR_DATATYPES_PHRASES_MA_NEEDS_2] );				
#endif			
			}
		}
	} else {
		// THIS IS THE NONE CASE, THERE ARE NO BLOCKS OR PROFILES SAVED
		
		// print the number of free blocks
		ftext_Printf( 0.55f, pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_BLOCKS_FREE].fUnitY,
					  L"~f1~C%ls~w0~a%lc~s%.2f%d", 
					  WprDataTypes_pwszGrayTextColor,
					  L'L',
					  pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_BLOCKS_FREE].fScale, 
					  0 );
		
		// print the number of saved profiles
		ftext_Printf( 0.55f, pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_PROFILES_SAVED].fUnitY,
					  L"~f1~C%ls~w0~a%lc~s%.2f%d", 
					  WprDataTypes_pwszGrayTextColor,
					  L'L',
					  pScreen->pText[_MENU_ITEMS_MU_START_OFFSET + _MENU_ITEMS_MU_PROFILES_SAVED].fScale, 
					  0 );
	}
}

static void _SelectMU_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT];
	Wpr_DataTypes_MUSelectEntry_t *pEntry = &_MenuState.paMUEntries[_MenuState.nCurItemIndex];
	f32 fUnitLen;

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	fUnitLen = (f32)fclib_wcslen( pEntry->pwszDisplayName );
	if( fUnitLen <= 6.0f ) {
		fUnitLen *= 16.5f;
	} else if( fUnitLen <= 11.0f ) {
		fUnitLen *= 14.5f;
	} else {
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
		fUnitLen *= 13.15f;
#else
		fUnitLen *= (12.5f * 1.1f);
#endif
	}
	fUnitLen *= _MU_SELECT_DISPLAY_NAME_SCALE;
	fUnitLen *= (1.0f/640.0f);// 0 - 1 length (in 640x480)
		
	wpr_drawutils_DrawLeftRightArrow( ((_MU_SELECT_DISPLAY_NAME_X - (fUnitLen * 0.5f)) - 0.5f) * 2.0f,
						 ((_MU_SELECT_DISPLAY_NAME_X + (fUnitLen * 0.5f)) - 0.5f) * 2.0f,
						 ( 0.5f - ( _MU_SELECT_DISPLAY_NAME_Y * (640.0f/480.0f) ) ) * 2.0f,
						 25.0f * _MU_SELECT_DISPLAY_NAME_SCALE,
						 fScaleMultiplier, fHalfXRes, fHalfYRes );

}

static void _SelectMU_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	BOOL bToggleCamera = TRUE;
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		if( _MenuState.nCurItemIndex == 0 ) {
			// warn the user about not being able to save
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NO_SAVE_WARNING;			
			// make sure that the save info is updated
			pSaveInfo->nStorageDeviceID = FSTORAGE_DEVICE_ID_NONE;
			pSaveInfo->wszCardName[0] = 0;
			pSaveInfo->wszProfileName[0] = 0;
			pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
		} else {
			// setup the profile select screen
            Wpr_DataTypes_MUSelectEntry_t *pEntry = &_MenuState.paMUEntries[_MenuState.nCurItemIndex];

			if( !_IsSpaceAvailForProfile( pEntry->pDeviceInfo ) && 
				pEntry->pDeviceInfo->uNumProfiles == 0 ) {
				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
				bToggleCamera = FALSE;
			} else {
				// copy the selected memory unit info
				pSaveInfo->nStorageDeviceID = pEntry->pDeviceInfo->oeID;
				_SafeCopyFStorageName( pSaveInfo->wszCardName, pEntry->pDeviceInfo->wszName );
				pSaveInfo->wszProfileName[0] = 0;
				pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
				
				_MenuState.pPPDevInfo = pEntry->pDeviceInfo;
				_MenuState.bPPRoomForNewProfiles = _IsSpaceAvailForProfile( _MenuState.pPPDevInfo ); 
				_MenuState.nPPNumSelections = _MenuState.pPPDevInfo->uNumProfiles;
				if( _MenuState.bPPRoomForNewProfiles ) {
					_MenuState.nPPNumSelections++;
				}
				_MenuState.nPPTopSelectionIndex = 0;
				_MenuState.nPPBottomSelectionIndex = (_MENU_ITEMS_PP_ON_SCREEN_LIMIT-1);
				_MenuState.bPPNeedToCacheProfiles = TRUE;
							
				_MenuState.nCurItemIndex = 0;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SELECT;
			}
		}
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// back to the main menu
		_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;
		_MenuState.fModeTimer = 0.0f;
		_MenuState.nCurItemIndex = _MENU_ITEMS_MM_SINGLE;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MAIN_MENU;
		_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
		break;

	case WPR_DATATYPES_NAV_CODE_ALTERNATE:
		gameloop_SetExitDestination( GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM );
		gameloop_ScheduleExit();
		break;

	case WPR_DATATYPES_NAV_CODE_CONFIRM: // This is used to trigger the warning format card menu screen

		if( _MenuState.paMUEntries[ _MenuState.nFCEntryArrayID ].bFormatOffered ) {
			// The user has already had the chance to format this card and declined...
			// Go right to the confirm format screen.
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_CONFIRM_FORMAT;			
		} else {
			// The user will be informed for the first time that this card needs to be formatted.
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_WARNING_FORMATCARD;			
		}
		break;
		
	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;	
}

static Wpr_DataTypes_MUSelectEntry_t *_InitMUSelectVars( u16 &rnNumElements ) {
	Wpr_DataTypes_MUSelectEntry_t *paSelectEntries;
	u32 i, nIndex;
	
	// set the number of elements that we are going to allocate
	rnNumElements = FSTORAGE_MAX_DEVICES + 1;// the extra 1 is for "None"

	// allocate rnNumElements structures
	paSelectEntries = (Wpr_DataTypes_MUSelectEntry_t *)fres_Alloc( sizeof( Wpr_DataTypes_MUSelectEntry_t ) * rnNumElements );
	if( !paSelectEntries ) {
		return NULL;
	}

	// fill in the "None" slot
	nIndex = 0;
	paSelectEntries[nIndex].bOfferDevice = TRUE;
	paSelectEntries[nIndex].bDeviceUnusable = FALSE;
	paSelectEntries[nIndex].bFormatOffered = FALSE;
	paSelectEntries[nIndex].pDeviceInfo = NULL;
	paSelectEntries[nIndex].pwszDisplayName = _apwszPhrases[WPR_DATATYPES_PHRASES_NONE];
	nIndex++;

	// Set up the character pointers for the Y button phrases
	_MenuState.pwszMUManageMemory = _apwszPhrases[WPR_DATATYPES_PHRASES_MANAGEMEMORY];
	_MenuState.pwszMUFormat		= _apwszPhrases[WPR_DATATYPES_PHRASES_FORMAT];
	
	for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
		// create a display name for the slots
		_FillInStorageDeviceName( i, Wpr_DataTypes_wszTempString );
		paSelectEntries[nIndex].pwszDisplayName = _pStringTable->AddString( Wpr_DataTypes_wszTempString );

		// don't offer this device yet, this will get updated later
        paSelectEntries[nIndex].bOfferDevice = FALSE;
        paSelectEntries[nIndex].bFormatOffered = FALSE;
		paSelectEntries[nIndex].bDeviceUnusable = FALSE;
		paSelectEntries[nIndex].pDeviceInfo = NULL;

		nIndex++;
	}

	return paSelectEntries;
}

static void _MUSelectUpdate( Wpr_DataTypes_MUSelectEntry_t *paSelectEntries, BOOL bForceUpdate ) {
	u32 i;
	const FStorage_DeviceInfo_t *pDeviceInfo;

	// update the current device infos
	u32 nConnected, nInserted, nRemoved;
	fstorage_UpdateDeviceInfos( &nConnected, &nInserted, &nRemoved );

	if( !bForceUpdate &&
		nInserted == FSTORAGE_DEVICE_STATUS_NONE &&
		nRemoved == FSTORAGE_DEVICE_STATUS_NONE ) {
		// no change, go ahead and quit now
		return;
	}
	
	for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
		pDeviceInfo = fstorage_GetDeviceInfo( i );

		if( (pDeviceInfo->uStatus & FSTORAGE_DEVICE_READY_FOR_USE) == FSTORAGE_DEVICE_READY_FOR_USE ) {
			// device is ready to go
			paSelectEntries[i+1].bOfferDevice = TRUE;
			paSelectEntries[i+1].bDeviceUnusable = FALSE;
			paSelectEntries[i+1].bFormatOffered = FALSE;
			paSelectEntries[i+1].pDeviceInfo = pDeviceInfo;

		} else if( pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_CONNECTED ) {
			// this is an unmountable device that is connected
			paSelectEntries[i+1].bOfferDevice = TRUE;
			paSelectEntries[i+1].bDeviceUnusable = TRUE;
			paSelectEntries[i+1].pDeviceInfo = pDeviceInfo;

		} else {
			// device is not plugged in
			paSelectEntries[i+1].bOfferDevice = FALSE;
			paSelectEntries[i+1].bDeviceUnusable = FALSE;
			paSelectEntries[i+1].bFormatOffered = FALSE;
			paSelectEntries[i+1].pDeviceInfo = NULL;
		}
	}
}

static u32 _MUSelectPickDefault( Wpr_DataTypes_MUSelectEntry_t *paSelectEntries ) {
	u32 i;

	for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
		if( paSelectEntries[i+1].bOfferDevice && !paSelectEntries[i+1].bDeviceUnusable ) {
			return i+1;
		}
	}

	// guess we have to pick the "none" slot
	return 0;
}

///////////////////
// 'select profile'
///////////////////

static Wpr_DataTypes_NavCode_e _SelectProfile_Work( void ) {

	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}
	// check for the Y button
	if( _CheckYButton( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_CONFIRM;
	}
	s32 nCache = _MenuState.nCurItemIndex;

	// allow the configuration to be changed
	_UpDown_e nUpDown = _CheckUpDownAxis( _MenuState.nControllerIndex );
	_MenuState.nCurItemIndex += nUpDown;
	FMATH_CLAMP( _MenuState.nCurItemIndex, 0, _MenuState.nPPNumSelections-1 );
#if defined(MA_PC_INPUT)
	s32 nHover = _MouseHoverSelect( _MenuState.nControllerIndex, _MenuState.nPPNumSelections-1 );
	if( nHover >= 0 ) {
		_MenuState.nCurItemIndex = (s8)nHover;
	}
#endif

	// figure out the scroll vars
	if( _MenuState.nCurItemIndex < _MenuState.nPPTopSelectionIndex ) {
		// scroll up
		_MenuState.nPPTopSelectionIndex--;
		_MenuState.nPPBottomSelectionIndex--;

		_MenuState.bPPNeedToCacheProfiles = TRUE;
	} else if( _MenuState.nCurItemIndex > _MenuState.nPPBottomSelectionIndex ) {
		// scroll down
		_MenuState.nPPTopSelectionIndex++;
		_MenuState.nPPBottomSelectionIndex++;

		_MenuState.bPPNeedToCacheProfiles = TRUE;
	}
	
	// gather up the profile infos
	if( nCache != _MenuState.nCurItemIndex ) {
		// play the cursor change sound
		ftext_ResetBlinkTimers();
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );			

	} else if( nUpDown != _NOT_UP_OR_DOWN ) {
		// there was movement on the stick, but no change
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
	}		

	if( _MenuState.bPPRoomForNewProfiles && (_MenuState.nCurItemIndex == 0) ) {
        _MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
	} else {
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABY_BUTTONS;
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _SelectProfile_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_PROFILE_SELECT];

	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );
	
	// print the memory unit 
	Wpr_DataTypes_TextLayout_t *pText = &pScreen->pText[_MENU_ITEMS_PP_MEM_UNIT + _MENU_ITEMS_PP_START_OFFSET];

	_SafeCopyFStorageName( Wpr_DataTypes_wszTempString, _MenuState.pPPDevInfo->wszName, _MAX_MU_NAME_DISPLAY_LEN );
	ftext_Printf( pText->fUnitX + (0.055f * (fclib_wcslen( pText->pwszText )/3)), pText->fUnitY,
				L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
				WprDataTypes_pwszWhiteTextColor,
				L'L',
				pText->fScale, 
				Wpr_DataTypes_wszTempString );

	// we print the rest later, so we can also use fdraw	
}

static void _SelectProfile_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_PROFILE_SELECT];
	u32 i, nNumReturned;
	
	// setup the fdraw modes
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
			
	wpr_drawutils_DrawUpDownArrow( 0.0f,
		0.20f,
		-0.35f,
		25.0f,
		90.0f,
		fScaleMultiplier, fHalfXRes, fHalfYRes );

	// fill in a temp text layout struct
	Wpr_DataTypes_TextLayout_t TempTextLayout;
	
	TempTextLayout.fScale = 1.0f;
	TempTextLayout.fUnitX = 0.5f;
	TempTextLayout.fUnitY = 0.31f;
	TempTextLayout.nAlignment = WPR_DATATYPES_ALIGN_CENTERED;
	TempTextLayout.nType = WPR_DATATYPES_REGULAR;
	TempTextLayout.pwszText = Wpr_DataTypes_wszTempString;
	TempTextLayout.fTickSpaceFactor = 16.0f;

	BOOL bSelected;
	u32 nStartOffset, nLastEntry, nLen;

	if( _MenuState.bPPRoomForNewProfiles ) {
		nStartOffset = (_MenuState.nPPTopSelectionIndex) ? _MenuState.nPPTopSelectionIndex-1 : 0;
		nLastEntry = _MenuState.nPPBottomSelectionIndex;
		if( nLastEntry > _MenuState.pPPDevInfo->uNumProfiles ) {
			nLastEntry = _MenuState.pPPDevInfo->uNumProfiles;
		}

		if( _MenuState.nPPTopSelectionIndex == 0 ) {
			// draw the "create new profile"
			TempTextLayout.pwszText = _apwszPhrases[WPR_DATATYPES_PHRASES_CREATE_NEW_PROFILE];

			bSelected = (_MenuState.nCurItemIndex == 0);
			_DrawText( &TempTextLayout, bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes );
#if defined(MA_PC_INPUT)
			_MouseAddItem( 0 );
#endif

			if( bSelected ) {
				wpr_drawutils_DrawSelectionArrows( &TempTextLayout,
					&_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
					fScaleMultiplier, fHalfXRes, fHalfYRes );
			}
			TempTextLayout.pwszText = Wpr_DataTypes_wszTempString;
			TempTextLayout.fUnitY += 0.04f;
		}
	} else {
		// not offering a "create new" option
		nStartOffset = _MenuState.nPPTopSelectionIndex;
		nLastEntry = _MenuState.nPPBottomSelectionIndex + 1;
		if( nLastEntry > _MenuState.pPPDevInfo->uNumProfiles ) {
			nLastEntry = _MenuState.pPPDevInfo->uNumProfiles;
		}
	}
	
	if( _MenuState.bPPNeedToCacheProfiles ) {
		// only cache if there are profiles to cache
		if( _MenuState.pPPDevInfo->uNumProfiles ) {
			if( fstorage_GetProfileInfos( _MenuState.pPPDevInfo->oeID,
				&_MenuState.aPPProfileInfos[0],
				nLastEntry - nStartOffset,
				&nNumReturned,
				nStartOffset ) != FSTORAGE_ERROR_NONE ) {
				// there was an error
				return;
			}
			if( nNumReturned != nLastEntry - nStartOffset ) {
				return;
			}
		}
		_MenuState.bPPNeedToCacheProfiles = FALSE;
	}

	// draw the profiles on this card
	u32 nIndex = 0;
	for( i=nStartOffset; i < nLastEntry; i++ ) {
		_SafeCopyFStorageName( Wpr_DataTypes_wszTempString, _MenuState.aPPProfileInfos[nIndex].wszName );
		bSelected = ( _MenuState.bPPRoomForNewProfiles ? (_MenuState.nCurItemIndex == (i+1)) : (_MenuState.nCurItemIndex == i) );
		_DrawText( &TempTextLayout, 
			bSelected,
			fScaleMultiplier, fHalfXRes, fHalfYRes );
#if defined(MA_PC_INPUT)
		_MouseAddItem( _MenuState.bPPRoomForNewProfiles ? (s32)i+1 : (s32)i );
#endif

		if( bSelected ) {
			nLen = fclib_wcslen( Wpr_DataTypes_wszTempString );

			if( nLen == 1 ) {
				TempTextLayout.fTickSpaceFactor = 40.0f;
			} else if( nLen < 4 ) {
				TempTextLayout.fTickSpaceFactor = 30.0f;
			} else if( nLen < 8 ) {
				TempTextLayout.fTickSpaceFactor = 28.0f;
			} else {
				TempTextLayout.fTickSpaceFactor = 24.0f;
			}
                			
			wpr_drawutils_DrawSelectionArrows( &TempTextLayout,
				&_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
				fScaleMultiplier, fHalfXRes, fHalfYRes );
		}
		TempTextLayout.fUnitY += 0.04f;

		nIndex++;
	}
}

static void _SelectProfile_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	u32 i;
	BOOL bSpinCamera = TRUE;
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {
			
	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// determine if the user has selected an existing profile or to create a new one
		if( _MenuState.bPPRoomForNewProfiles && (_MenuState.nCurItemIndex == 0) ) {
			// create a new profile name and take the user to the profile name screen
			_MenuState.bLMNewProfile = TRUE;

			// pick a new default name
			_CreateUniqueName( pSaveInfo->nStorageDeviceID,
							  _MenuState.wszPNCurProfileName );

			// move to the name profile screen
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;
			_MenuState.bPNLower = TRUE;
			// pick the "done" button as default
			_MenuState.nPNCurCol = 0;
			_MenuState.nPNCurRow = (_MenuState.nPNNumRows-1);
			_MenuState.nCurItemIndex = 0;
			for( i=0; i < (u32)_MenuState.nPNCurRow; i++ ) {
				_MenuState.nCurItemIndex += _MenuState.anPNColumnsPerRow[i];
			}
			_MenuState.nCurItemIndex += _MenuState.nPNCurCol;

		} else {
            // the user selected a profile, load it

			// grab the selected profile
			FStorage_ProfileInfo_t ProfileInfo;
			u32 nNumReturned;
			if( fstorage_GetProfileInfos( _MenuState.pPPDevInfo->oeID,
				&ProfileInfo,
				1,
				&nNumReturned,
				_MenuState.bPPRoomForNewProfiles ? (_MenuState.nCurItemIndex-1) : _MenuState.nCurItemIndex ) != FSTORAGE_ERROR_NONE ) {
				// there was an error
				FASSERT_NOW;
			}
			FASSERT( nNumReturned == 1 );

			// copy the profile name
			_SafeCopyFStorageName( pSaveInfo->wszProfileName, ProfileInfo.wszName );

			// read the desired profile
			_TryToReadProfileAndSetupProperResponseState( FALSE );
		}
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the MU select screen
		_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		
		// select the already selected memory card by default
		_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
		for( i=0; i < _MenuState.nMUNumEntries; i++ ) {
			if( _MenuState.paMUEntries[i].pDeviceInfo == _MenuState.pPPDevInfo ) {
				_MenuState.nCurItemIndex = i;
				break;
			}
		}
		break;

	case WPR_DATATYPES_NAV_CODE_CONFIRM:
		// the user wants to delete the current profile, confirm
		if( (_MenuState.bPPRoomForNewProfiles && (_MenuState.nCurItemIndex != 0)) ||
			!_MenuState.bPPRoomForNewProfiles ) {

			// grab the selected profile
			FStorage_ProfileInfo_t ProfileInfo;
			u32 nNumReturned;
			if( fstorage_GetProfileInfos( _MenuState.pPPDevInfo->oeID,
				&ProfileInfo,
				1,
				&nNumReturned,
				_MenuState.bPPRoomForNewProfiles ? (_MenuState.nCurItemIndex-1) : _MenuState.nCurItemIndex ) != FSTORAGE_ERROR_NONE ) {
				// there was an error
				FASSERT_NOW;
			}
			FASSERT( nNumReturned == 1 );

			// copy the profile name
			_SafeCopyFStorageName( pSaveInfo->wszProfileName, ProfileInfo.wszName );

			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_DELETE_PROFILE;
		} else {
			bSpinCamera = FALSE;
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
		}
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_PROFILE_SELECT;
}

static void _SelectProfile_SetupReturnVisit( const FStorage_DeviceInfo_t *pDevInfo, wchar *pwszProfileNameToMakeActive ) {

	_MenuState.nCurItemIndex = _FindMemCardProfileIndexFromName( pDevInfo, pwszProfileNameToMakeActive );
	FASSERT( _MenuState.nCurItemIndex >= 0 );
	if( _MenuState.bPPRoomForNewProfiles ) {
		_MenuState.nCurItemIndex++;
	}
	if( _MenuState.nCurItemIndex < _MENU_ITEMS_PP_ON_SCREEN_LIMIT ) {
		_MenuState.nPPTopSelectionIndex = 0;
		_MenuState.nPPBottomSelectionIndex = (_MENU_ITEMS_PP_ON_SCREEN_LIMIT-1);
	} else {
		_MenuState.nPPTopSelectionIndex = _MenuState.nCurItemIndex - (_MENU_ITEMS_PP_ON_SCREEN_LIMIT-1);
		_MenuState.nPPBottomSelectionIndex = _MenuState.nCurItemIndex;
	}
	_MenuState.bPPNeedToCacheProfiles = TRUE;

	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SELECT;
}

////////////////////////////
// 'pick color' functions
////////////////////////////

static Wpr_DataTypes_NavCode_e _PickColor_Work( void ) {
	
	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}
	
	// check for up/down changes
	_HandleAxisSelections( FALSE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex, (GAMESAVE_MP_COLORS_COUNT-1), 
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _PickColor_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_PICK_COLOR];
	
    wpr_drawutils_DrawTextureToScreen( TRUE, 
		&_paTexInsts[WPR_DATATYPES_TEXTURES_YELLOW_GLITCH + _MenuState.nCurItemIndex],
		0.25f, 0.0f,
		0.70f,
		fScaleMultiplier, fHalfXRes, fHalfYRes );	
}

static void _PickColor_ProfileToWorkingVars() {

	_MenuState.nCurItemIndex = _paProfiles[0].m_Data.nColorIndex;
	FMATH_CLAMP( _MenuState.nCurItemIndex, 0, (GAMESAVE_MP_COLORS_COUNT-1) );
}

static void _PickColor_WorkingVarsToProfile() {
	
	_paProfiles[0].m_Data.nColorIndex = (u8)_MenuState.nCurItemIndex;
}

static void _PickColor_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the profile settings screen, lose changes
		_PickColor_ProfileToWorkingVars();
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_COLOR;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// save changes and go back to the profile settings screen
		_PickColor_WorkingVarsToProfile();
		_MenuState.nCurItemIndex = _MENU_ITEMS_PS_COLOR;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SETTINGS;
		
		// flag the profile for saving
		_paProfiles[0].m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_PICK_COLOR;
}

////////////////////
// MJ - 'multi join'
////////////////////
static Wpr_DataTypes_NavCode_e _MultiJoin_Work( void ) {
	u32 i, nFirstPlayerToExit;
	BOOL bBack, bForward;

	// first update the join info
    _MJ_UpdateJoinInfo( &_MenuState.MJJoinInfo, FALSE );

	// clear all of the work flags
	for( i=0; i < MAX_PLAYERS; i++ ) {
		_MenuState.aMJSections[i].bWorkDoneThisFrame = FALSE;
	}

		// do all the ERROR work first
	for( i=0; i< MAX_PLAYERS; i++ ) {
		if( _MenuState.aMJSections[i].nState == _MULTI_JOIN_STATE_LOAD_ERROR ) {
			_MenuState.aMJSections[i].bWorkDoneThisFrame = TRUE;
			if( _CheckAcceptButtons( i ) ) {
				_MenuState.aMJSections[i].nState = _MULTI_JOIN_STATE_SELECT;
			}
		}
	}
	
	// do all of the ENTER state work first
	bBack = FALSE;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		if( _MJ_SectionWork_EnterState( &_MenuState.MJJoinInfo, &_MenuState.aMJSections[i] ) == WPR_DATATYPES_NAV_CODE_BACK ) {
			bBack = TRUE;
		}
	}

	// do all of the SELECT state work second
	for( i=0; i < MAX_PLAYERS; i++ ) {
		_MJ_SectionWork_SelectState( &_MenuState.MJJoinInfo, &_MenuState.aMJSections[i] );
	}

	// do all of the WAIT state work next
	bForward = FALSE;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		if( _MJ_SectionWork_WaitState( &_MenuState.MJJoinInfo, &_MenuState.aMJSections[i] ) == WPR_DATATYPES_NAV_CODE_FORWARD ) {
			if( !bForward ) {
				nFirstPlayerToExit = i;
			}
			bForward = TRUE;
		}
	}

	// update our waiting for other players var
	_MenuState.MJJoinInfo.bWaitingForOtherPlayers = FALSE;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		if( !( _MenuState.aMJSections[i].nState == _MULTI_JOIN_STATE_ENTER ||
			_MenuState.aMJSections[i].nState == _MULTI_JOIN_STATE_WAIT) ) {
			_MenuState.MJJoinInfo.bWaitingForOtherPlayers = TRUE;
			break;
		}
	}
	
	if( bForward ) {
		if( !_MenuState.MJJoinInfo.bWaitingForOtherPlayers ) {
			// everyone has decided, record who is in this round and then move on
			_MenuState.nMPPlayerMask = 0;
			_MenuState.nMPNumPlayers = 0;
			_MenuState.nMLNumUnlockedLevels = 0;
			for( i=0; i < MAX_PLAYERS; i++ ) {
				if( _MenuState.aMJSections[i].nState == _MULTI_JOIN_STATE_WAIT ) {
					_MenuState.nMPPlayerMask |= (1 << i);
					_MenuState.nMPNumPlayers++;
					// record how many levels have been unlocked
					_MenuState.nMLNumUnlockedLevels = FMATH_MAX( _MenuState.nMLNumUnlockedLevels, _MenuState.aMJSections[i].pProfile->m_Data.nNumMPLevelsUnlocked );					
				}
			}
			_MenuState.nControllerIndex = nFirstPlayerToExit;
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			return WPR_DATATYPES_NAV_CODE_FORWARD;
		}
	} else if( bBack ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MultiJoin_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i, nDeviceIndex;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MULTI_JOIN];
	Wpr_DataTypes_TextLayout_t *pTextLayout;
	
	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	// draw the state specific text
	for( i=0; i < MAX_PLAYERS; i++ ) {
		pTextLayout = &pScreen->pText[i + _MENU_ITEMS_MJ_PLAYER_HEADING_OFFSET];

		switch( _MenuState.aMJSections[i].nState ) {

		case _MULTI_JOIN_STATE_ENTER:
			// if the controller is plugged in, draw the join message
			// otherwise, draw the insert controller message
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.07f, 
				L"~f8~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszInstructionTextColor,
				pTextLayout->fScale + 0.10f,
				( Gamepad_nPortOnlineMask & (1<<i) ) ? _apwszPhrases[WPR_DATATYPES_PHRASES_PRESS_A_TO_JOIN] : Game_apwszPhrases[GAMEPHRASE_INSERT_CONTROLLER] );		
			break;

		case _MULTI_JOIN_STATE_SELECT:
			// print the profile heading
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.04f, 
				L"~f1~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszBlueTextColor,
				pTextLayout->fScale * 0.98f,
				_apwszPhrases[WPR_DATATYPES_PHRASES_CHOOSE_PROFILE] );			

			// print the profile name
			_SafeCopyFStorageName( Wpr_DataTypes_wszTempString, _MenuState.aMJSections[i].pProfile->m_SaveInfo.wszProfileName, _MAX_MU_NAME_DISPLAY_LEN );
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.07f, 
				L"~f1~w0~ac~s%.2f~C%ls{ ~C%ls~B7%ls~B0 ~C%ls}",
				( fclib_wcslen( Wpr_DataTypes_wszTempString ) < 10 ) ? pTextLayout->fScale : pTextLayout->fScale * 0.94f,
				WprDataTypes_pwszBlueTextColor,
				WprDataTypes_pwszWhiteTextColor,
				Wpr_DataTypes_wszTempString, 
				WprDataTypes_pwszBlueTextColor );

			// print the location heading and name
			nDeviceIndex = _GET_JOIN_DEVICE_INDEX( _MenuState.aMJSections[i].nProfileKey );
			if( nDeviceIndex == _VIRTUAL_DEVICE_INDEX ) {
				_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls", _apwszPhrases[WPR_DATATYPES_PHRASES_NONE] );	
			} else {
				_FillInStorageDeviceName( nDeviceIndex, Wpr_DataTypes_wszTempString );
			}
			// print the location heading & location
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.10f, 
				L"~f1~C%ls~w0~ac~s%.2f%ls : ~f8~s%.2f~C%ls%ls",
				WprDataTypes_pwszBlueTextColor,
				pTextLayout->fScale * 0.80f,
				_apwszPhrases[WPR_DATATYPES_PHRASES_LOCATION],
				pTextLayout->fScale * 0.75f,
				WprDataTypes_pwszProfileLocationColor,
				Wpr_DataTypes_wszTempString );

			// print the accept instructions
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.13f, 
				L"~f8~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszInstructionTextColor,
				pTextLayout->fScale,
				_apwszPhrases[WPR_DATATYPES_PHRASES_PRESS_A_TO_ACCEPT] );
			break;

		case _MULTI_JOIN_STATE_WAIT:
			// print the profile name
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.06f, 
				L"~f1~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszBlueTextColor,
				pTextLayout->fScale + 0.02f,
				_MenuState.aMJSections[i].pProfile->m_SaveInfo.wszProfileName );

			// print the instructions to move on
			if( !_MenuState.MJJoinInfo.bWaitingForOtherPlayers ) {
				ftext_Printf( pTextLayout->fUnitX,
					pTextLayout->fUnitY + 0.13f, 
					L"~f8~C%ls~w0~ac~s%.2f%ls",
					WprDataTypes_pwszInstructionTextColor,
					pTextLayout->fScale,
					_apwszPhrases[WPR_DATATYPES_PHRASES_PRESS_A_TO_PROCEED] );
			} else {
				ftext_Printf( pTextLayout->fUnitX,
					pTextLayout->fUnitY + 0.13f, 
					L"~f8~C%ls~w0~ac~s%.2f%ls",
					WprDataTypes_pwszInstructionTextColor,
					pTextLayout->fScale * 0.81f,
					_apwszPhrases[WPR_DATATYPES_PHRASES_WAITING_FOR_OTHER_PLAYERS] );				
			}
			break;

		case _MULTI_JOIN_STATE_LOAD_ERROR:
            // draw the error message
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.07f, 
				L"~f8~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszInstructionTextColor,
				pTextLayout->fScale,
				_apwszPhrases[WPR_DATATYPES_PHRASES_UNABLE_LOAD_PROFILE] );		

			// draw the continue message
			ftext_Printf( pTextLayout->fUnitX,
				pTextLayout->fUnitY + 0.13f, 
				L"~f8~C%ls~w0~ac~s%.2f%ls",
				WprDataTypes_pwszInstructionTextColor,
				pTextLayout->fScale,
				_apwszPhrases[WPR_DATATYPES_PHRASES_PRESS_A_TO_CONTINUE] );
			break;

		default:
			FASSERT_NOW;
			break;
		}
	}
}

static void _MultiJoin_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the Main Menu screen
		_MenuState.nMode = WPR_DATATYPES_MODES_MAIN_MENU;
		_MenuState.fModeTimer = 0.0f;
		_MenuState.nCurItemIndex = _MENU_ITEMS_MM_MULTI;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MAIN_MENU;
		_MenuState.nControllerIndex = WPR_SYSTEM_CONTROLLER_PORT_UNKNOWN;
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// go to the Game Type screen
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_TYPE;
		_MenuState.nMTMode = 0;
		_MenuState.nMTType = 0;
		_MenuState.nMRTmpRuleType = -1;

		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_JOIN;
}

static void _MJ_InitJoinInfoAndSections( _JoinInfo_t *pJoinInfo, 
										_MultiJoin_SectionData_t *paSections,
										u32 nNumPlayers ) {
	u32 i;

	FASSERT( nNumPlayers <= MAX_PLAYERS );

	// update the Join Info
	_MJ_UpdateJoinInfo( pJoinInfo, TRUE );

	// init the section data
	_MultiJoin_SectionData_t *pSection;
	for( i=0; i < nNumPlayers; i++ ) {
		pSection = &paSections[i];

		pSection->nState = _MULTI_JOIN_STATE_ENTER;
		pSection->nProfileKey = pJoinInfo->nDefaultKey;
		pSection->pProfile = &_paProfiles[i];
		pSection->pProfile->m_nControllerIndex = i;
		pSection->bWorkDoneThisFrame = FALSE;
        pSection->nTeamID = 0;
	}

	pJoinInfo->bWaitingForOtherPlayers = TRUE;
}

static void _MJ_UpdateJoinInfo( _JoinInfo_t *pJoinInfo, BOOL bForceUpdate ) {
	u32 i;
	const FStorage_DeviceInfo_t *pDeviceInfo;
	BOOL bDefaultSet = FALSE;

	// update the current device infos
	u32 nConnected, nInserted, nRemoved;
	fstorage_UpdateDeviceInfos( &nConnected, &nInserted, &nRemoved );

	if( !bForceUpdate &&
		nInserted == FSTORAGE_DEVICE_STATUS_NONE &&
		nRemoved == FSTORAGE_DEVICE_STATUS_NONE ) {
		// no change, go ahead and quit now
		return;
	}

	// start with no know profiles
	pJoinInfo->nTotalProfiles = 0;
	
	// walk the real devices
	for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
		pDeviceInfo = fstorage_GetDeviceInfo( i );

		if( (pDeviceInfo->uStatus & FSTORAGE_DEVICE_READY_FOR_USE) == FSTORAGE_DEVICE_READY_FOR_USE ) {
			if( !bDefaultSet && pDeviceInfo->uNumProfiles ) {
				pJoinInfo->nDefaultKey = _CREATE_JOIN_KEY( i, 0 );
				bDefaultSet = TRUE;
			}
			pJoinInfo->anNumProfiles[i] = pDeviceInfo->uNumProfiles;
			pJoinInfo->nTotalProfiles += pDeviceInfo->uNumProfiles;			
		} else {
			pJoinInfo->anNumProfiles[i] = 0;
		}
	}
	// add the virtual profiles to the virtual device
	pJoinInfo->anNumProfiles[_VIRTUAL_DEVICE_INDEX] = _NUM_VIRTUAL_PROFILES;
	pJoinInfo->nTotalProfiles += _NUM_VIRTUAL_PROFILES;

	// since there must not be any memory card plugged in, make the 1st virtual profile the default
	if( !bDefaultSet ) {
		pJoinInfo->nDefaultKey = _CREATE_JOIN_KEY( _VIRTUAL_DEVICE_INDEX, 0 );
		bDefaultSet = TRUE;
	}
}

// Assumes that _MJ_UpdateJoinInfo has been called this frame.
static Wpr_DataTypes_NavCode_e _MJ_SectionWork_EnterState( const _JoinInfo_t *pJoinInfo,
														  _MultiJoin_SectionData_t *pSection ) {
	
	if( pSection->bWorkDoneThisFrame ||
		pSection->nState != _MULTI_JOIN_STATE_ENTER ) {
		// not the correct state work function or we've already done work on this section
		return WPR_DATATYPES_NAV_CODE_NOTHING;
	}
	
	// mark that we have already done work on this section so that we don't do it again till next frame
	pSection->bWorkDoneThisFrame = TRUE;

	// waiting for user to click the button to move to the next state
	if( _CheckAcceptButtons( pSection->pProfile->m_nControllerIndex ) ) {
		// the user finally pressed the accept button, move to the enter state
		pSection->nState = _MULTI_JOIN_STATE_SELECT;
		pSection->nProfileKey = pJoinInfo->nDefaultKey;
		
		// fill in the save info based on our profile key
		_MJ_UpdateSaveInfo( pJoinInfo, pSection, FALSE );

		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SELECT_ITEM] );

	} else if( _CheckBackButtons( pSection->pProfile->m_nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;	
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;	
}

// Assumes that _MJ_UpdateJoinInfo has been called this frame.
static Wpr_DataTypes_NavCode_e _MJ_SectionWork_SelectState( const _JoinInfo_t *pJoinInfo,
														   _MultiJoin_SectionData_t *pSection ) {

	if( pSection->bWorkDoneThisFrame ||
		pSection->nState != _MULTI_JOIN_STATE_SELECT ) {
		// not the correct state work function or we've already done work on this section
		return WPR_DATATYPES_NAV_CODE_NOTHING;
	}
	
	// mark that we have already done work on this section so that we don't do it again till next frame
	pSection->bWorkDoneThisFrame = TRUE;

	// waiting for user to click the button to move to the next state
	if( _CheckAcceptButtons( pSection->pProfile->m_nControllerIndex ) ) {
		// the user finally pressed the accept button
		
		// read the desired profile
		if( _GET_JOIN_DEVICE_INDEX( pSection->nProfileKey ) == _VIRTUAL_DEVICE_INDEX ) {
			// fill in the profile with either the default or invert default
			pSection->pProfile->InitNewProfile( TRUE );
			
			u32 nProfileIndex = _GET_JOIN_PROFILE_INDEX( pSection->nProfileKey );
			if( nProfileIndex == 0 ) {
				
			} else if( nProfileIndex == 1 ) {
				pSection->pProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG;
			} else {
				FASSERT_NOW;
			}
			pSection->nState = _MULTI_JOIN_STATE_WAIT;	

		} else {
			// read the profile off the mem card
			if( !pSection->pProfile->LoadFromCard() ) {
				// move to the error state
				pSection->nState = _MULTI_JOIN_STATE_LOAD_ERROR;				
			} else {
				// move to the wait state
				pSection->nState = _MULTI_JOIN_STATE_WAIT;		
			}
		}		

		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SELECT_ITEM] );

	} else if( _CheckBackButtons( pSection->pProfile->m_nControllerIndex ) ) {
		// back up a state
		pSection->nState = _MULTI_JOIN_STATE_ENTER;		
	} else {
		_MJ_UpdateSaveInfo( pJoinInfo, pSection, TRUE );
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static Wpr_DataTypes_NavCode_e _MJ_SectionWork_WaitState( const _JoinInfo_t *pJoinInfo,
														 _MultiJoin_SectionData_t *pSection ) {
	if( pSection->bWorkDoneThisFrame ||
		pSection->nState != _MULTI_JOIN_STATE_WAIT ) {
		// not the correct state work function or we've already done work on this section
		return WPR_DATATYPES_NAV_CODE_NOTHING;
	}
	
	// mark that we have already done work on this section so that we don't do it again till next frame
	pSection->bWorkDoneThisFrame = TRUE;

	// waiting for user to click the button to move to the next state
	if( _CheckAcceptButtons( pSection->pProfile->m_nControllerIndex ) ) {
		// the user finally pressed the accept button
		
		return WPR_DATATYPES_NAV_CODE_FORWARD;

	} else if( _CheckBackButtons( pSection->pProfile->m_nControllerIndex ) ) {
		// back up a state
		pSection->nState = _MULTI_JOIN_STATE_SELECT;

		// fill in the save info based on our profile key
		_MJ_UpdateSaveInfo( pJoinInfo, pSection, FALSE );
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

// assumes that _MJ_UpdateJoinInfo has been called this frame
static void _MJ_UpdateSaveInfo( const _JoinInfo_t *pJoinInfo, 
							   _MultiJoin_SectionData_t *pSection,
							   BOOL bAllowChanges ) {
	s32 nDeviceIndex, nProfileIndex;
	_LeftRight_e nLR;
	FStorage_ProfileInfo_t ProfileInfo;
	const FStorage_DeviceInfo_t *pDeviceInfo;
	BOOL bKeyChanged = FALSE;

	// grab the device and profile indices
	nDeviceIndex = _GET_JOIN_DEVICE_INDEX( pSection->nProfileKey );
	nProfileIndex = _GET_JOIN_PROFILE_INDEX( pSection->nProfileKey );

	FASSERT( nDeviceIndex < _TOTAL_DEVICES );

	// make sure that nDeviceIndex is still plugged in
	if( (s32)pJoinInfo->anNumProfiles[nDeviceIndex] <= nProfileIndex ) {
		// something happened and the selected profile isn't available anymore, we must reset this section to default selection
		pSection->nProfileKey = pJoinInfo->nDefaultKey;

		// since we just set the key, re get the device & profile indices
		nDeviceIndex = _GET_JOIN_DEVICE_INDEX( pSection->nProfileKey );
		nProfileIndex = _GET_JOIN_PROFILE_INDEX( pSection->nProfileKey );

		bKeyChanged = TRUE;

	} else {
		// check the controller to see if we should change profiles
		if( bAllowChanges ) {
			nLR = _CheckLeftRightAxis( pSection->pProfile->m_nControllerIndex );
			if( nLR ) {
				do {
					nProfileIndex += nLR;
					if( nProfileIndex >= (s32)pJoinInfo->anNumProfiles[nDeviceIndex] ) {
						// we moved past the greatest allowed profile index on this device, move to the next device
						nProfileIndex = 0;
						nDeviceIndex++;
						if( nDeviceIndex >= _TOTAL_DEVICES ) {
							nDeviceIndex = 0;
						}
					} else if( nProfileIndex < 0 ) {
						// moved past the lowest allowed profile index on this device, move to the next device
						nDeviceIndex--;
						if( nDeviceIndex < 0 ) {
							nDeviceIndex = _VIRTUAL_DEVICE_INDEX;
						}
						nProfileIndex = (pJoinInfo->anNumProfiles[nDeviceIndex]) ? pJoinInfo->anNumProfiles[nDeviceIndex] - 1 : 0;
					}
				} while( pJoinInfo->anNumProfiles[nDeviceIndex] == 0 );

				pSection->nProfileKey = _CREATE_JOIN_KEY( nDeviceIndex, nProfileIndex );

				bKeyChanged = TRUE;

				fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );
			}
		}
	}

	GameSave_SaveInfo_t *pSaveInfo = &pSection->pProfile->m_SaveInfo;

	if( nDeviceIndex == _VIRTUAL_DEVICE_INDEX ) {
		// this device is always available
		pSaveInfo->nStorageDeviceID  = FSTORAGE_DEVICE_ID_NONE;
		pSaveInfo->wszCardName[0] = 0;
		_SafeCopyFStorageName( pSaveInfo->wszProfileName, _apwszPhrases[WPR_DATATYPES_PHRASES_DEFAULT + nProfileIndex] );
		pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
	} else {
		// this is from a memory card (only update if the user changed selections)
		if( !bAllowChanges || bKeyChanged ) {
			if( !_MJ_ConvertProfileIndexToProfileInfo( pJoinInfo, nDeviceIndex, nProfileIndex, &ProfileInfo, &pDeviceInfo ) ) {
				FASSERT_NOW;

				nProfileIndex = 0;
				nDeviceIndex = _VIRTUAL_DEVICE_INDEX;
				pSection->nProfileKey = _CREATE_JOIN_KEY( nDeviceIndex, nProfileIndex );

				pSaveInfo->nStorageDeviceID = FSTORAGE_DEVICE_ID_NONE;
				pSaveInfo->wszCardName[0] = 0;
				_SafeCopyFStorageName( pSaveInfo->wszProfileName, _apwszPhrases[WPR_DATATYPES_PHRASES_DEFAULT + nProfileIndex] );
				pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
			} else {
				pSaveInfo->nStorageDeviceID = pDeviceInfo->oeID;
				_SafeCopyFStorageName( pSaveInfo->wszCardName, pDeviceInfo->wszName );
				_SafeCopyFStorageName( pSaveInfo->wszProfileName, ProfileInfo.wszName );
				pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
			}
		}
	}
}

static BOOL _MJ_ConvertProfileIndexToProfileInfo( const _JoinInfo_t *pJoinInfo,
												 u32 nDeviceIndex, u32 nProfileIndex,
												 FStorage_ProfileInfo_t *pProfileInfo,
												 const FStorage_DeviceInfo_t **ppDeviceInfo ) {
	u32 nCount;
	
	if( nDeviceIndex >= _VIRTUAL_DEVICE_INDEX ) {
		return FALSE;
	}

	*ppDeviceInfo = fstorage_GetDeviceInfo( nDeviceIndex );
	if( !*ppDeviceInfo ||
		(((*ppDeviceInfo)->uStatus & FSTORAGE_DEVICE_READY_FOR_USE) != FSTORAGE_DEVICE_READY_FOR_USE ) ) {
		// either the device was pulled or something bad happened
		return FALSE;
	}
	
	// get the profile Info pointer
	if( fstorage_GetProfileInfos( (*ppDeviceInfo)->oeID,
        pProfileInfo,
		1,
		&nCount,
		nProfileIndex ) != FSTORAGE_ERROR_NONE ) {
		// trouble getting the profile info
		return FALSE;
	}

	return TRUE;
}

/////////////////////////
// MT - 'multi game type'
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiType_Work( void ) {
	
	_MP_FillUniqueProfiles();
	s32 nTypeCount = _MT_CalcTotalTypes();

	// check for the A button, Play Game
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button, Back
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}
	// check for the Y button, Edit Types
	if( _CheckYButton( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_CONFIRM;
	}

	// Only show the X button if there are profiles to edit
	BOOL bAllowMore = _MenuState.apMEUniqueProfiles[0] != NULL;

	// Check for the X button, More options
	if( bAllowMore && _CheckXButton( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_ALTERNATE;
	}
	
	if( !_HandleAxisSelections( FALSE, _MenuState.nControllerIndex, 
							_MenuState.nCurItemIndex, (_MENU_ITEMS_MT_COUNT-1),
							FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] ) ) {

		// check for left/right changes
		_LeftRight_e nLeftRight = _CheckLeftRightAxis( _MenuState.nControllerIndex );
		if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
			s32 nCache2;

			// the left right axis has changed
			switch( _MenuState.nCurItemIndex ) {

			case _MENU_ITEMS_MT_TYPE:
				nCache2 = _MenuState.nMTType;
				_MenuState.nMTType += nLeftRight;
				FMATH_CLAMP( _MenuState.nMTType, 0, nTypeCount-1);

				if( nCache2 != _MenuState.nMTType ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );	
				}
				break;

			case _MENU_ITEMS_MT_MODE:
				nCache2 = _MenuState.nMTMode;
				_MenuState.nMTMode += nLeftRight;
				FMATH_CLAMP( _MenuState.nMTMode, 0, _MENU_ITEMS_MT_TYPE_COUNT-1 );

				if( nCache2 != _MenuState.nMTMode ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );	
				}
				break;

			default:
				FASSERT_NOW;
				break;
			}
		}
	}
	
	if( bAllowMore )
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABYX_BUTTONS;
	else
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABY_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MultiType_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MULTI_TYPE];
		
	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {		
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}
	
	// draw the text
	BOOL bSelected;
	u32 nItem;
	for( i=0; i < _MENU_ITEMS_MT_DESCRIPTION_START_OFFSET; i++ ) {
		bSelected = (i == (_MenuState.nCurItemIndex + _MENU_ITEMS_MT_START_OFFSET));
		_DrawText( &pScreen->pText[i], bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes, FALSE, TRUE );
#if defined(MA_PC_INPUT)
		if( i >= _MENU_ITEMS_MT_START_OFFSET ) {
			_MouseAddItem( (s32)i - _MENU_ITEMS_MT_START_OFFSET, _MOUSE_ITEM_ACCEPT | _MOUSE_ITEM_ADJUST );
		}
#endif

		if( i >= _MENU_ITEMS_MT_START_OFFSET && i < (_MENU_ITEMS_MT_DESCRIPTION_START_OFFSET-1) ) {
			nItem = i - _MENU_ITEMS_MT_START_OFFSET;

			if( nItem == _MENU_ITEMS_MT_TYPE ) {
				ftext_Printf( _MULTIPLAYER_TYPES_X,
							pScreen->pText[i].fUnitY,
							L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
							bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
							L'C',
							pScreen->pText[i].fScale * 0.8f,
							_MT_GetTypeName( _MenuState.nMTType ) );

				// If this type comes from a user profile, print the location info
				s32 nRuleIndex = 0;
				CPlayerProfile* pProfile = _MT_GetProfForType( _MenuState.nMTType, &nRuleIndex );

				if ( pProfile ) {
					wchar wszBuf1[20], wszBuf2[20];

					// Print the game type
					ftext_Printf( 0.5f,
						pScreen->pText[i].fUnitY + 0.04f,
						L"~f8~C%ls~w0~ac~s%.2f%ls",
						WprDataTypes_pwszInstructionTextColor,
						pScreen->pText[i].fScale * 0.80f,
						_apwszPhrases[WPR_DATATYPES_PHRASES_BASE_DEATHMATCH + pProfile->m_Data.aMultiPlayerRules[nRuleIndex].nGameType] );

					// Print the location
					_SafeCopyString( wszBuf1, pProfile->m_SaveInfo.wszCardName, 20, 13 );
					_SafeCopyString( wszBuf2, pProfile->m_SaveInfo.wszProfileName, 20, 12 );
					ftext_Printf( 0.5f,
						pScreen->pText[i].fUnitY + 0.08f,
						L"~f8~C%ls~w0~ac~s%.2f%ls : ~f8~s%.2f~C%ls%ls",
						WprDataTypes_pwszInstructionTextColor,
						pScreen->pText[i].fScale * 0.6f,
						wszBuf1,
						pScreen->pText[i].fScale * 0.6f,
						WprDataTypes_pwszProfileLocationColor,
						wszBuf2 );
				}
				else {
					ftext_Printf( 0.5f,
						pScreen->pText[i].fUnitY + 0.04f, 
						L"~f8~C%ls~w0~ac~s%.2f%ls",
						WprDataTypes_pwszInstructionTextColor,
						pScreen->pText[i].fScale * 0.80f,
						_apwszPhrases[WPR_DATATYPES_PHRASES_BUILT_IN_TYPE] );
				}
			} else {
				// Play Mode
				ftext_Printf( _MULTIPLAYER_TYPES_X,
							pScreen->pText[i].fUnitY,
							L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
							bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
							L'C',
							pScreen->pText[i].fScale * 0.8f, 
							_MenuState.nMTMode ? _apwszPhrases[WPR_DATATYPES_PHRASES_TEAM] : _apwszPhrases[WPR_DATATYPES_PHRASES_INDIVIDUAL] );
			}
		}
	}

	// draw the description for the current selected item
	i = _MENU_ITEMS_MT_DESCRIPTION_START_OFFSET;
	if( _MenuState.nMRTmpRuleType == _MenuState.nMTType )
		i += _MENU_ITEMS_MT_DESCRIPTIONS_PER_TYPE * (_MENU_ITEMS_ME_PRECOOKED_COUNT + 1);
	else if ( _MenuState.nMTType > _MENU_ITEMS_ME_PRECOOKED_COUNT )
		i += _MENU_ITEMS_MT_DESCRIPTIONS_PER_TYPE * _MENU_ITEMS_ME_PRECOOKED_COUNT;
	else
		i += _MENU_ITEMS_MT_DESCRIPTIONS_PER_TYPE * _MenuState.nMTType;

	_DrawText( &pScreen->pText[i], FALSE, fScaleMultiplier, fHalfXRes, fHalfYRes );
	_DrawText( &pScreen->pText[i+1], FALSE, fScaleMultiplier, fHalfXRes, fHalfYRes );

	// draw which player is in control of the selection
	_DrawSelectingPlayerMsg();
}

static s32 _MT_CalcTotalTypes( void ) {
	// We always count our precooked types
	s32 nTypes = _MENU_ITEMS_ME_PRECOOKED_COUNT;

	s32 j;
	for( j=0; j < _MenuState.nMEUniqueProfileCount; j++ ) {
		nTypes += _MenuState.apMEUniqueProfiles[j]->m_Data.nMPRulesCount;
	}

	return nTypes;
}

static void _MT_InitRulesFromType( GameSave_MPRules_t* pRules, s32 nType, BOOL bPreserveName, BOOL bPreserveBaseType ) {
	FASSERT( (WPR_DATATYPES_PHRASES_TAG - WPR_DATATYPES_PHRASES_DEATHMATCH + 1) == _MENU_ITEMS_ME_PRECOOKED_COUNT);

	// If we are creating a new type, preserve the base type and just fill in
	// the default rules.
	if( bPreserveBaseType ) {
		switch (pRules->nGameType) {
			case GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH:
				nType = _MENU_ITEMS_ME_PRECOOKED_DEATHMATCH;
				break;
			case GAME_MULTIPLAYER_BASE_TYPES_KING_OF_THE_HILL:
				nType = _MENU_ITEMS_ME_PRECOOKED_KOTH_FIXED;
				break;
			case GAME_MULTIPLAYER_BASE_TYPES_TAG:
				nType = _MENU_ITEMS_ME_PRECOOKED_TAG;
				break;
			case GAME_MULTIPLAYER_BASE_TYPES_REVERSE_TAG:
				nType = _MENU_ITEMS_ME_PRECOOKED_REVERSE_TAG;
				break;
		}
	}

	// If we are being asked for a pre-cooked type, fill it in 
	if ( nType < _MENU_ITEMS_ME_PRECOOKED_COUNT ) {
		// Set to the default set of rules, which is a timed death match
		if( bPreserveName )
			CPlayerProfile::SetMultiPlayerRulesToDefault( pRules, pRules->wszGameName );
		else
			CPlayerProfile::SetMultiPlayerRulesToDefault( pRules, _apwszPhrases[WPR_DATATYPES_PHRASES_DEATHMATCH + nType] );

		// Only change what is necessary here; get the rest from SetMultiPlayerRulesToDefault.
		switch (nType) {
			case _MENU_ITEMS_ME_PRECOOKED_DEATHMATCH_TIMED:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;
				pRules->nDMKillsPerRound = 0;
				pRules->nTimeLimit = 15;
				break;
			case _MENU_ITEMS_ME_PRECOOKED_POSSESSION_MELEE:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;
				pRules->nDMKillsPerRound = 10;
				pRules->nTimeLimit = 0;
				pRules->nLimitPrimaryWeapon = GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_WEAPONS;
				pRules->nLimitSecondaryWeapon = GAMESAVE_SECONDARY_WEAPON_LIMIT_RECRUITER_ONLY;
				pRules->nLimitVehicles = GAMESAVE_VEHICLE_LIMIT_NO_VEHICLES;
				break;
			case _MENU_ITEMS_ME_PRECOOKED_KOTH_FIXED:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_KING_OF_THE_HILL;
				pRules->nTimeLimit = 2;
				pRules->nTimeBetweenHillSwaps = 0;
				break;
			case _MENU_ITEMS_ME_PRECOOKED_KOTH_MOVING:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_KING_OF_THE_HILL;
				pRules->nTimeLimit = 2;
				pRules->nTimeBetweenHillSwaps = pRules->nTimeLimit;
				break;
			case _MENU_ITEMS_ME_PRECOOKED_REVERSE_TAG:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_REVERSE_TAG;
				pRules->nTimeLimit = 2;
				break;
			case _MENU_ITEMS_ME_PRECOOKED_TAG:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_TAG;
				pRules->nTimeLimit = 2;
				break;
			case _MENU_ITEMS_ME_PRECOOKED_DEATHMATCH:
			default:
				pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;
				pRules->nDMKillsPerRound = 10;
				pRules->nTimeLimit = 0;
				break;
		}
		return;
	}

	// Not a pre-cooked type, so look it up and return a pointer to it.
	nType -= _MENU_ITEMS_ME_PRECOOKED_COUNT;

	s32 j;
	for( j = 0; j < _MenuState.nMEUniqueProfileCount; j++ ) {
		CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[j];
		if ( nType < pProfile->m_Data.nMPRulesCount ) {
			*pRules = pProfile->m_Data.aMultiPlayerRules[nType];
			return;
		}
		nType -= pProfile->m_Data.nMPRulesCount;
	}

	// Should never get here
	FASSERT_NOW;
	CPlayerProfile::SetMultiPlayerRulesToDefault( pRules, _apwszPhrases[WPR_DATATYPES_PHRASES_DEFAULT_GAMETYPE_NAME] );
}

static cwchar* _MT_GetTypeName( s32 nType ) {
	FASSERT( (WPR_DATATYPES_PHRASES_TAG - WPR_DATATYPES_PHRASES_DEATHMATCH + 1) == _MENU_ITEMS_ME_PRECOOKED_COUNT);

	// If it is one of our precooked types, just return the name
	if ( nType < _MENU_ITEMS_ME_PRECOOKED_COUNT )
		return _apwszPhrases[WPR_DATATYPES_PHRASES_DEATHMATCH + nType];

	// User defined type. Search through the profiles and find the correct match
	nType -= _MENU_ITEMS_ME_PRECOOKED_COUNT;

	s32 j;
	for( j=0; j < _MenuState.nMEUniqueProfileCount; j++ ) {
		CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[j];
		if ( nType < pProfile->m_Data.nMPRulesCount ) {
			return pProfile->m_Data.aMultiPlayerRules[nType].wszGameName;
		}
		else {
			nType -= pProfile->m_Data.nMPRulesCount;
		}
	}

	// Should never get here...
	FASSERT_NOW;
	return _apwszPhrases[WPR_DATATYPES_PHRASES_DEATHMATCH];
}

static CPlayerProfile* _MT_GetProfForType( s32 nType, s32* pnRulesIndex ) {
	s32 nIndex = _MT_GetProfIndexForType(nType, pnRulesIndex);
	return ( nIndex >= 0 ) ? _MenuState.apMEUniqueProfiles[ nIndex ] : NULL;
}

static s32 _MT_GetProfIndexForType( s32 nType, s32* pnRulesIndex ) {
	FASSERT( (WPR_DATATYPES_PHRASES_TAG - WPR_DATATYPES_PHRASES_DEATHMATCH + 1) == _MENU_ITEMS_ME_PRECOOKED_COUNT);

	// If it is one of our precooked types, let them know
	if ( nType < _MENU_ITEMS_ME_PRECOOKED_COUNT )
		return -1;

	// User defined type. Search through the profiles and find the correct match
	nType -= _MENU_ITEMS_ME_PRECOOKED_COUNT;

	s32 j;
	for( j=0; j < _MenuState.nMEUniqueProfileCount; j++ ) {
		CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[j];
		if ( nType < pProfile->m_Data.nMPRulesCount ) {
			if ( pnRulesIndex )
				*pnRulesIndex = nType;
			return j;
		}
		else {
			nType -= pProfile->m_Data.nMPRulesCount;
		}
	}

	// Should never get here...
	FASSERT_NOW;
	return -1;
}

static void _MultiType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the Multi Join screen
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_JOIN;

		_MJ_InitJoinInfoAndSections( &_MenuState.MJJoinInfo, _MenuState.aMJSections, MAX_PLAYERS );
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		if( _MenuState.nMTMode ) {
            // go to the Pick Team screen
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_TEAM;

			// setup the initial team configuration
			u32 nCount, i, nMask;
			nCount = 0;
			_MenuState.nMPTeamAHumanMask = 0;
			for( i=0; i < MAX_PLAYERS; i++ ) {
				nMask = (1 << i);
				if( nMask & _MenuState.nMPPlayerMask ) {
					nCount++;
					if( nCount & 0x1 ){
						// put this player on Team A
						_MenuState.nMPTeamAHumanMask |= nMask;
					}
					if( nCount == _MenuState.nMPNumPlayers ) {
						break;
					}
				}
			}
			_MenuState.nMPBalanceDirection = +1;
		} else {
			// we don't need to team up, go to the pick level screen
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_LEVEL;
		}
		break;

	case WPR_DATATYPES_NAV_CODE_CONFIRM:
		// Edit the current rules
		_MT_PrepareEditRulesFromType();

		if ( _MenuState.nMTType < _MENU_ITEMS_ME_PRECOOKED_COUNT ) {
			// Editing a hardwired type; give them a warning first
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_NO_SAVE;
		}
		else {
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_RULES;
		}
		break;

	case WPR_DATATYPES_NAV_CODE_ALTERNATE:
		// More options: choose a profile, new, delete, etc

		// If we don't have any unique profiles, go to our warning screen
		if ( _MenuState.apMEUniqueProfiles[0] == NULL ) {
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_NO_SAVE;
		}
		else {
			// We have a profile, so go let them select a set of custom rules
			_MT_PrepareEditFromType();
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES;
		}
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_TYPE;
}

static void _MT_PrepareEditRulesFromType( void ) {
	s32 nType = _MenuState.nMTType;
	
	if ( nType < _MENU_ITEMS_ME_PRECOOKED_COUNT ) {
		_MT_InitRulesFromType( &_MenuState.MR_WorkingRules, _MenuState.nMTType, FALSE, FALSE );
		_MenuState.nMRProfileIndex = -1;
	}
	else {
		// User defined type. Search through the profiles and find the correct match
		nType -= _MENU_ITEMS_ME_PRECOOKED_COUNT;

		s32 j;
		for( j=0; j < _MenuState.nMEUniqueProfileCount; j++ ) {
			CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[j];
			if ( nType < pProfile->m_Data.nMPRulesCount ) {
				_MenuState.MR_WorkingRules = pProfile->m_Data.aMultiPlayerRules[nType];
				_MenuState.nMRProfileIndex = j;
				_MenuState.nMRRuleIndex = nType;
				break;
			}
			else {
				nType -= pProfile->m_Data.nMPRulesCount;
			}
		}
	}
	_MenuState.bMRMadeChanges = FALSE;
	_MenuState.bMEAppend = FALSE;
}

static void _MT_PrepareEditFromType( void ) {
	// Prepare to go to the edit types screen.

	if ( _MenuState.nMTType < _MENU_ITEMS_ME_PRECOOKED_COUNT ) {
		_MenuState.nMEType = 0;
		_MenuState.nMEPlayerIndexToGetRulesFrom = 0;
		return;
	}

	s32 nType = _MenuState.nMTType - _MENU_ITEMS_ME_PRECOOKED_COUNT;

	s32 j;
	for( j=0; j < _MenuState.nMEUniqueProfileCount; j++ ) {
		CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[j];
		if ( nType < pProfile->m_Data.nMPRulesCount ) {
			_MenuState.nMEType = nType;
			_MenuState.nMEPlayerIndexToGetRulesFrom = j;
			break;
		}
		else {
			nType -= pProfile->m_Data.nMPRulesCount;
		}
	}
}

//////////////////////////
// ME - 'multi game edit types'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiEditType_Work( void ) {
	// First find the correct player number
	s32 nPlayer = _MenuState.nMEPlayerIndexToGetRulesFrom;
	CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[nPlayer];

	// We do things differently if we have no room for a new one
	BOOL bFull = ( pProfile->m_Data.nMPRulesCount == MP_RULE_SET_MAX );

	// check for the A button, Edit rules
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button, Back
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}
	// check for the Y button, New
	if( !bFull && _CheckYButton( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_CONFIRM;
	}
	// check for the X button, Delete
	if ( _MenuState.nMEType < pProfile->m_Data.nMPRulesCount ) {
		if( _CheckXButton( _MenuState.nControllerIndex ) ) {
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			return WPR_DATATYPES_NAV_CODE_ALTERNATE;
		}
	}

	if( !_HandleAxisSelections( FALSE, _MenuState.nControllerIndex, 
		_MenuState.nCurItemIndex, (_MENU_ITEMS_ME_COUNT-1),
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] ) ) {

			// check for left/right changes
			_LeftRight_e nLeftRight = _CheckLeftRightAxis( _MenuState.nControllerIndex );
			if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
				s32 nCache2;

				// the left right axis has changed
				switch( _MenuState.nCurItemIndex ) {

					case _MENU_ITEMS_ME_PROFILE:
						nCache2 = _MenuState.nMEPlayerIndexToGetRulesFrom;
						_MenuState.nMEPlayerIndexToGetRulesFrom += nLeftRight;
						FMATH_CLAMP( _MenuState.nMEPlayerIndexToGetRulesFrom, 0, _MenuState.nMEUniqueProfileCount - 1 );

						if( nCache2 != _MenuState.nMEPlayerIndexToGetRulesFrom ) {
							fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );

							// Since our profile changed, better reset our type
							_MenuState.nMEType = 0;
						}
						break;

					case _MENU_ITEMS_ME_GAME_TYPE:
						nCache2 = _MenuState.nMEType;
						if (pProfile->m_Data.nMPRulesCount > 0) {
							_MenuState.nMEType += nLeftRight;
							FMATH_CLAMP( _MenuState.nMEType, 0, pProfile->m_Data.nMPRulesCount - 1 );
						}
						else {
							_MenuState.nMEType = 0;
						}

						if( nCache2 != _MenuState.nMEType ) {
							fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );	
						}
						break;

					default:
						FASSERT_NOW;
						break;
				}
			}
		}

		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;

		// Only draw "New" button if array is not full
		if ( !bFull )
			_MenuState.nButtonDrawMask |= WPR_DATATYPES_DRAW_Y_BUTTON_ONLY;

		// Only draw "Delete" button if valid profile selected
		if ( _MenuState.nMEType < pProfile->m_Data.nMPRulesCount )
			_MenuState.nButtonDrawMask |= WPR_DATATYPES_DRAW_X_BUTTON_ONLY;

		return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MultiEditType_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES];

	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {		
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}

	// draw the text
	BOOL bSelected;
	u32 nItem;
	for( i=0; i < pScreen->nNumTextElements; i++ ) {
		bSelected = (i == (_MenuState.nCurItemIndex + _MENU_ITEMS_ME_START_OFFSET));
		_DrawText( &pScreen->pText[i], bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes, FALSE, TRUE );
#if defined(MA_PC_INPUT)
		if( i >= _MENU_ITEMS_ME_START_OFFSET ) {
			_MouseAddItem( (s32)i - _MENU_ITEMS_ME_START_OFFSET, _MOUSE_ITEM_ACCEPT | _MOUSE_ITEM_ADJUST );
		}
#endif

		if( i >= _MENU_ITEMS_ME_START_OFFSET ) {
			nItem = i - _MENU_ITEMS_ME_START_OFFSET;

			// find the _MenuState.nMEPlayerIndexToGetRulesFrom'th player and print their id
			s32 nPlayer = _MenuState.nMEPlayerIndexToGetRulesFrom;
			CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[nPlayer];

			// Buffer for truncating strings
			wchar wszBuf[24];

			if( nItem == _MENU_ITEMS_ME_PROFILE ) {
				_SafeCopyString( wszBuf, pProfile->m_SaveInfo.wszProfileName, 24, 11 );
				ftext_Printf( _MULTIPLAYER_TYPES_X,
					pScreen->pText[i].fUnitY,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
					bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
					L'C',
					pScreen->pText[i].fScale * 0.8f,
					wszBuf );

				// print the location heading & location
				_SafeCopyString( wszBuf, pProfile->m_SaveInfo.wszCardName, 24, 15 );
				ftext_Printf( 0.5f,
					pScreen->pText[i].fUnitY + 0.04f, 
					L"~f1~C%ls~w0~ac~s%.2f%ls : ~f8~s%.2f~C%ls%ls",
					WprDataTypes_pwszBlueTextColor,
					pScreen->pText[i].fScale * 0.80f,
					_apwszPhrases[WPR_DATATYPES_PHRASES_LOCATION],
					pScreen->pText[i].fScale * 0.75f,
					WprDataTypes_pwszProfileLocationColor,
					wszBuf );

			} 
			else if( nItem == _MENU_ITEMS_ME_GAME_TYPE ) {
				cwchar* pwszName;
				if ( pProfile->m_Data.nMPRulesCount == 0 )
					pwszName = _apwszPhrases[WPR_DATATYPES_PHRASES_CREATE_NEW_GAMETYPE];
				else
					pwszName = pProfile->m_Data.aMultiPlayerRules[_MenuState.nMEType].wszGameName;

				ftext_Printf( _MULTIPLAYER_TYPES_X,
					pScreen->pText[i].fUnitY,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
					bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
					L'C',
					pScreen->pText[i].fScale * 0.8f, 
					pwszName );

				// Print the game type
				if ( pProfile->m_Data.nMPRulesCount > 0 ) {
					ftext_Printf( 0.5f,
						pScreen->pText[i].fUnitY + 0.04f,
						L"~f8~C%ls~w0~ac~s%.2f%ls",
						WprDataTypes_pwszInstructionTextColor,
						pScreen->pText[i].fScale * 0.80f,
						_apwszPhrases[WPR_DATATYPES_PHRASES_BASE_DEATHMATCH + pProfile->m_Data.aMultiPlayerRules[_MenuState.nMEType].nGameType] );
				}
			}
		}
	}

	// draw which player is in control of the selection
	_DrawSelectingPlayerMsg();
}

static void _MultiEditType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	s32 i;
	s32 nPlayer	= _MenuState.nMEPlayerIndexToGetRulesFrom;
	CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[nPlayer];

	switch( nNavCode ) {

		case WPR_DATATYPES_NAV_CODE_BACK:
			// go back to the Choose Type screen

			// In case the number of types has changed, let's make sure to match
			// up the currently selected type with what we are currently showing.
			if( pProfile->m_Data.nMPRulesCount > 0 ) {
				_MenuState.nMTType = _MENU_ITEMS_ME_PRECOOKED_COUNT;
				for( i = 0; i < _MenuState.nMEPlayerIndexToGetRulesFrom; i++ ) {
					_MenuState.nMTType += _MenuState.apMEUniqueProfiles[i]->m_Data.nMPRulesCount;
				}
				_MenuState.nMTType += _MenuState.nMEType;
			}
			else if( _MenuState.nMTType >= _MENU_ITEMS_ME_PRECOOKED_COUNT ) {
				// No rules defined in this profile, and the previously selected
				// type is now off the scale, so just reset it.
				_MenuState.nMTType = _MENU_ITEMS_ME_PRECOOKED_DEATHMATCH;
			}

			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_TYPE;
			break;

		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// Edit the current set of rules

			// If the current player has no rules defined, create a new set
			if ( pProfile->m_Data.nMPRulesCount == 0 ) {
				_MenuState.bMRMadeChanges = FALSE;
				_ME_TransitionToNew( 1 );
			}
			else {
				// We have a valid rule set, so edit it
				_MenuState.nCurItemIndex = 0;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_RULES;

				// Copy this rule set into our working copy
				_MenuState.nMRProfileIndex = _MenuState.nMEPlayerIndexToGetRulesFrom;
				_MenuState.nMRRuleIndex = _MenuState.nMEType;
				_MenuState.bMRMadeChanges = FALSE;
				_MenuState.bMEAppend = FALSE;
				_MenuState.MR_WorkingRules = pProfile->m_Data.aMultiPlayerRules[ _MenuState.nMEType ];
			}
			break;

		case WPR_DATATYPES_NAV_CODE_CONFIRM:
			// Create a new set of rules

			// Should never be able to get here is we are full...
			FASSERT( pProfile->m_Data.nMPRulesCount < MP_RULE_SET_MAX );

			_MenuState.bMRMadeChanges = FALSE;
			_ME_TransitionToNew( pProfile->m_Data.nMPRulesCount + 1 );

			break;

		case WPR_DATATYPES_NAV_CODE_ALTERNATE:
			// Delete the currently selected rule set

			// Make sure we have a valid type to delete
			if ( _MenuState.nMEType < pProfile->m_Data.nMPRulesCount ) {
				// First confirm
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_CONFIRM_DEL;
			}
			break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES;
}

static void _ME_TransitionToNew( s32 nIndex ) {

	// Set the name into the holder for the keyboard
	_snwprintf( _MenuState.wszPNCurProfileName, FSTORAGE_MAX_NAME_LEN - 1, L"%s%d",
		_apwszPhrases[WPR_DATATYPES_PHRASES_DEFAULT_GAMETYPE_NAME], nIndex);

	// Start with a clean set of rules
	CPlayerProfile::SetMultiPlayerRulesToDefault( &_MenuState.MR_WorkingRules, _MenuState.wszPNCurProfileName );

	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_NEW_TYPE;
	_MenuState.bPNLower = TRUE;
	_ME_ResetKeyboard();
}

static void _ME_ResetKeyboard( void ) {
	// pick the "done" button as default
	_MenuState.nPNCurCol = 0;
	_MenuState.nPNCurRow = (_MenuState.nPNNumRows-1);
	_MenuState.nCurItemIndex = 0;
	s32 i;
	for( i=0; i < _MenuState.nPNCurRow; i++ ) {
		_MenuState.nCurItemIndex += _MenuState.anPNColumnsPerRow[i];
	}
	_MenuState.nCurItemIndex += _MenuState.nPNCurCol;
}

static void _MP_FillUniqueProfiles( void ) {
	s32 i;

	// Initialize
	_MenuState.nMEUniqueProfileCount = 0;
	for (i = 0; i < MAX_PLAYERS; i++) {
		_MenuState.apMEUniqueProfiles[i] = NULL;
	}

	// Go through the player's profiles, and put unique ones into our array
	for (i = 0; i < MAX_PLAYERS; i++) {
		if ( _MenuState.nMPPlayerMask & (1<<i) ) {
			if (!_paProfiles[i].IsVirtual()) {
				int j;
				for (j = 0; j < _MenuState.nMEUniqueProfileCount; j++) {
					if (_MP_IsEqual( _MenuState.apMEUniqueProfiles[j], &_paProfiles[i]) )
						break;
				}
				if ( j == _MenuState.nMEUniqueProfileCount ) {
					_MenuState.apMEUniqueProfiles[j] = &_paProfiles[i];
					_MenuState.nMEUniqueProfileCount++;
				}
			}
		}
	}
}

// We consider two profiles equal if they come from the same medium and have
// the same name.
static BOOL _MP_IsEqual( CPlayerProfile* p1, CPlayerProfile* p2 ) {
	if (p1->m_SaveInfo.nStorageDeviceID != p2->m_SaveInfo.nStorageDeviceID)
		return FALSE;

	if (fclib_wcsicmp(p1->m_SaveInfo.wszProfileName, p2->m_SaveInfo.wszProfileName))
		return FALSE;

	return TRUE;
}

/////////////////////////
// New multiplayer game type
/////////////////////////

static void _MultiNewType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	s32 i = _MenuState.nMEPlayerIndexToGetRulesFrom;
	CPlayerProfile* pProfile = _MenuState.apMEUniqueProfiles[i];

	switch( nNavCode ) {

		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// Copy out the name and edit the rules
			fclib_wcsncpy( _MenuState.MR_WorkingRules.wszGameName, _MenuState.wszPNCurProfileName, META_GAME_NAME_LEN - 1);

			// First, check to see if the name is unique within the profile
			for( i = 0; i < pProfile->m_Data.nMPRulesCount; i++) {
				if( fclib_wcsicmp( pProfile->m_Data.aMultiPlayerRules[i].wszGameName, _MenuState.MR_WorkingRules.wszGameName ) == 0 )
					break;
			}
			if( i < pProfile->m_Data.nMPRulesCount ) {
				// The name is a duplicate; transition to the warning screen
				_MenuState.nCurItemIndex = 0;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_DUPLICATE;
			}
			else {
				// Everything is OK; go ahead and edit the rules
				_MenuState.nCurItemIndex = 0;
				_MenuState.bMRMadeChanges = TRUE;
				_MenuState.bMEAppend = TRUE;
				_MenuState.nMRProfileIndex = _MenuState.nMEPlayerIndexToGetRulesFrom;
				_MenuState.nMRRuleIndex = _MenuState.nMEType;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_RULES;
			}
			break;

		case WPR_DATATYPES_NAV_CODE_BACK:
			// Don't save anything, just go back
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES;
			break;

		default:
			FASSERT_NOW;
			break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_NEW_TYPE;
}

/////////////////////////
// Rules are full
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiNosaveType_Work( void ) {

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );	
}

static void _MultiNosaveType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	switch( nNavCode ) {

		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// Edit the current rules without saving
			
			// Set the working rules to match the last selected game type
			_MT_InitRulesFromType( &_MenuState.MR_WorkingRules, _MenuState.nMTType, FALSE, FALSE );
			_MenuState.bMRMadeChanges = FALSE;
			_MenuState.bMEAppend = FALSE;
			_MenuState.nMRProfileIndex = -1;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_RULES;

			break;

		case WPR_DATATYPES_NAV_CODE_BACK:
			// Cancel; just go back
			_MenuState.nCurrentScreen = _MenuState.nLastScreen;
			break;

		default:
			FASSERT_NOW;
			break;
	}

	// Note: we intentionally don't set the last screen to this, so that
	// going back will go to OUR previous screen.
}

/////////////////////////
// Confirm delete of rules
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiDelType_Work( void ) {

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );	
}

static void _MultiDelType_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	_GenericDrawOrtho_NoSelectionsWithString( &Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ],
											0.36f,
											_MenuState.apMEUniqueProfiles[ _MenuState.nMEPlayerIndexToGetRulesFrom ]->m_Data.aMultiPlayerRules[_MenuState.nMEType].wszGameName, 
											fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _MultiDelType_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	s32 i;
	s32 nPlayer = _MenuState.nMEPlayerIndexToGetRulesFrom;
	CPlayerProfile *pProfile = _MenuState.apMEUniqueProfiles[nPlayer];
	s32 nCount = pProfile->m_Data.nMPRulesCount;
	FASSERT( (nCount == 0) || (_MenuState.nMEType < nCount) );

	switch( nNavCode ) {

		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// delete the game type

			// Fill in the hole
			for ( i = _MenuState.nMEType; i < nCount - 1; i++ ) {
				pProfile->m_Data.aMultiPlayerRules[i] = pProfile->m_Data.aMultiPlayerRules[i+1];
			}
			pProfile->m_Data.nMPRulesCount--;

			// Make sure that the current type index is valid
			if ( _MenuState.nMEType >= pProfile->m_Data.nMPRulesCount )
				_MenuState.nMEType = 0;

			// Now save the profile

			// make sure that this player's profile isn't virtual
			if( !pProfile->IsVirtual() ) {
				// instead of heading straight back to the type screen, save the profile
				_SetupMemCardSave( WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES, pProfile );
			}

			break;

		case WPR_DATATYPES_NAV_CODE_BACK:
			// don't delete this game type, return to the edit screen
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES;
			break;

		default:
			FASSERT_NOW;
			break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_CONFIRM_DEL;
}

/////////////////////////
// Confirm delete of rules
/////////////////////////
static Wpr_DataTypes_NavCode_e _MultiDupWarn_Work( void ) {

	return _GenericButtonWork( TRUE, FALSE, WPR_DATATYPES_DRAW_A_BUTTON_ONLY );
}

static void _MultiDupWarn_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	_GenericDrawOrtho_NoSelectionsWithString( &Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ],
										0.32f,
										_MenuState.MR_WorkingRules.wszGameName, 
										fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _MultiDupWarn_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// Just go back
			_MenuState.nCurrentScreen = _MenuState.nLastScreen;
			break;

		default:
			FASSERT_NOW;
			break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_DUPLICATE;
}

//////////////////////////
// MR - 'multi game rules'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiRules_Work( void ) {
	GameSave_MPRules_t *pRules = &_MenuState.MR_WorkingRules;

	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}
	// check for the Y button
	if( _CheckYButton( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_CONFIRM;
	}

	// check for up/down changes
	if( !_HandleRuleAxisSelections( FALSE, _MenuState.nControllerIndex, 
		_MenuState.nCurItemIndex, (_MENU_ITEMS_MR_COUNT-1),
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] ) ) {

		// check for left/right changes
		_LeftRight_e nLeftRight = _CheckLeftRightAxis( _MenuState.nControllerIndex );
		if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
			s32 nCache2;

			// the left right axis has changed
			switch( _MenuState.nCurItemIndex ) {

			case _MENU_ITEMS_MR_BASE_TYPE:
				nCache2 = pRules->nGameType;
				pRules->nGameType += nLeftRight;
				FMATH_CLAMP( pRules->nGameType, GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH, GAME_MULTIPLAYER_BASE_TYPES_COUNT-1 );
				if( nCache2 != pRules->nGameType ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;

					// Deathmatch allows a time of zero, but others do not
					if( pRules->nGameType != GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH ) {
						if( pRules->nTimeLimit == 0) {
							pRules->nTimeLimit = 2;
						}
					}
				}
				break;

			case _MENU_ITEMS_MR_MARKERS:
				nCache2 = pRules->nFlags;
				if( nLeftRight == _LEFT ) {
					pRules->nFlags &= ~GAMESAVE_RULE_FLAGS_MARKERS_ON;
				} else {
					pRules->nFlags |= GAMESAVE_RULE_FLAGS_MARKERS_ON;
				}
				if( nCache2 != pRules->nFlags ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_RADAR:
				nCache2 = pRules->nFlags;
				if( nLeftRight == _LEFT ) {
					pRules->nFlags &= ~GAMESAVE_RULE_FLAGS_RADAR_ON;
				} else {
					pRules->nFlags |= GAMESAVE_RULE_FLAGS_RADAR_ON;
				}
				if( nCache2 != pRules->nFlags ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_ONLY_PRIMARY:
				nCache2 = pRules->nLimitPrimaryWeapon;
				pRules->nLimitPrimaryWeapon += nLeftRight;
				FMATH_CLAMP( pRules->nLimitPrimaryWeapon, GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT, GAMESAVE_PRIMARY_WEAPON_LIMIT_COUNT - 1 );
				if( nCache2 != pRules->nLimitPrimaryWeapon ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_ONLY_SECONDARY:
				nCache2 = pRules->nLimitSecondaryWeapon;
				pRules->nLimitSecondaryWeapon += nLeftRight;
				FMATH_CLAMP( pRules->nLimitSecondaryWeapon, GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT, GAMESAVE_SECONDARY_WEAPON_LIMIT_COUNT-1 );
				if( nCache2 != pRules->nLimitSecondaryWeapon ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_ONLY_VEHICLE:
				nCache2 = pRules->nLimitVehicles;
				pRules->nLimitVehicles += nLeftRight;
				FMATH_CLAMP( pRules->nLimitVehicles, GAMESAVE_VEHICLE_LIMIT_ALL_VEHICLES, GAMESAVE_VEHICLE_LIMIT_COUNT-1 );
				if( nCache2 != pRules->nLimitVehicles ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_TIME_LIMIT:
				nCache2 = pRules->nTimeLimit;
				pRules->nTimeLimit = (s8)_BumpNumber( nCache2, nLeftRight, 10, 5 * nLeftRight );

				if( pRules->nGameType == GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH ) {
					FMATH_CLAMP( pRules->nTimeLimit, 0, _MAX_MINUTES_PER_ROUND );
				}
				else {
					FMATH_CLAMP( pRules->nTimeLimit, 1, _MAX_MINUTES_PER_ROUND );
				}

				if( nCache2 != pRules->nTimeLimit ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}					
				break;

			case _MENU_ITEMS_MR_DM_POINTS_TO_WIN:
				nCache2 = pRules->nDMKillsPerRound;
				pRules->nDMKillsPerRound = (s8)_BumpNumber( nCache2, nLeftRight, 10, 5 * nLeftRight );
				FMATH_CLAMP( pRules->nDMKillsPerRound, 0, _MAX_POINTS_PER_ROUND );
				if( nCache2 != pRules->nDMKillsPerRound ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}					
				break;

			case _MENU_ITEMS_MR_KOTH_SWAP_TIME:
				nCache2 = pRules->nTimeBetweenHillSwaps;
				pRules->nTimeBetweenHillSwaps = (s8)_BumpNumber( nCache2, nLeftRight, 10, 5 * nLeftRight );
				FMATH_CLAMP( pRules->nTimeBetweenHillSwaps, 0, _MAX_MINUTES_PER_ROUND );
				if( nCache2 != pRules->nTimeBetweenHillSwaps ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_POSSESSABLE_BOTS:
				nCache2 = pRules->nFlags;
				if( nLeftRight == _LEFT ) {
					pRules->nFlags &= ~GAMESAVE_RULE_FLAGS_POSSESSABLE_BOTS;
				} else {
					pRules->nFlags |= GAMESAVE_RULE_FLAGS_POSSESSABLE_BOTS;
				}
				if( nCache2 != pRules->nFlags ) {
					fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			case _MENU_ITEMS_MR_POSSESSION_BOT_STATE:
				nCache2 = pRules->nFlags;
				if( nLeftRight == _LEFT ) {
					pRules->nFlags &= ~GAMESAVE_RULE_FLAGS_BOTS_ON_WHEN_NOT_TETHERED;
				} else {
					pRules->nFlags |= GAMESAVE_RULE_FLAGS_BOTS_ON_WHEN_NOT_TETHERED;
				}
				if( nCache2 != pRules->nFlags ) {
                    fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
					_MenuState.bMRMadeChanges = TRUE;
				}
				break;

			}
		}
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABY_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MultiRules_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i, nItem;
	cwchar *pwszEndString;
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MULTI_RULES];
	
	// draw the meshes
	for( i=0; i < pScreen->nNumMeshElements; i++ ) {		
		wpr_drawutils_DrawMesh_XlatOnly( &pScreen->pMesh[i], fScaleMultiplier, fHalfXRes, fHalfYRes );
	}

	// draw which player is in control of the selection
	_DrawSelectingPlayerMsg( 0.085f, 0.17f );

	// Draw the name of the current rule set
	ftext_Printf( 0.5f, 0.21f,
		L"~f1~C%ls~w0~aC~s1.50%ls", 
		WprDataTypes_pwszSubtitleTextColor,
		_MenuState.MR_WorkingRules.wszGameName );

	// draw the text
	BOOL bSelected;
	BOOL bDisabled;
	for( i=0; i < pScreen->nNumTextElements; i++ ) {
		bSelected = (i == (_MenuState.nCurItemIndex + _MENU_ITEMS_MR_START_OFFSET));
		bDisabled = FALSE;
		if( i >= _MENU_ITEMS_MR_START_OFFSET )
			bDisabled = _RuleDisabled( i - _MENU_ITEMS_MR_START_OFFSET );

		_DrawText( &pScreen->pText[i], bSelected, fScaleMultiplier, fHalfXRes, fHalfYRes, bDisabled, TRUE );
#if defined(MA_PC_INPUT)
		// A saves the rules and leaves; the rows are settings
		if( i >= _MENU_ITEMS_MR_START_OFFSET && !bDisabled ) {
			_MouseAddItem( (s32)i - _MENU_ITEMS_MR_START_OFFSET, _MOUSE_ITEM_ADJUST );
		}
#endif

		// draw the options
		if( i >= _MENU_ITEMS_MR_START_OFFSET ) {
			nItem = i - _MENU_ITEMS_MR_START_OFFSET;

			switch( nItem ) {

			case _MENU_ITEMS_MR_BASE_TYPE:
				ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
					pScreen->pText[i].fUnitY,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
					bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
					L'C',
					pScreen->pText[i].fScale, 
					_apwszPhrases[WPR_DATATYPES_PHRASES_BASE_DEATHMATCH + _MenuState.MR_WorkingRules.nGameType] );
				break;	

			case _MENU_ITEMS_MR_MARKERS:
				_DrawOnOffSelection( _MULTIPLAYER_RULES_OPTIONS_X,
									pScreen->pText[i].fUnitY,
									bSelected,
									pScreen->pText[i].fScale,
									_MenuState.MR_WorkingRules.nFlags & GAMESAVE_RULE_FLAGS_MARKERS_ON );
				break;

			case _MENU_ITEMS_MR_RADAR:
				_DrawOnOffSelection( _MULTIPLAYER_RULES_OPTIONS_X,
									 pScreen->pText[i].fUnitY,
									 bSelected,
									 pScreen->pText[i].fScale,
									 _MenuState.MR_WorkingRules.nFlags & GAMESAVE_RULE_FLAGS_RADAR_ON );
				break;

			case _MENU_ITEMS_MR_ONLY_PRIMARY:
				ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
							pScreen->pText[i].fUnitY,
							L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
							bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
							L'C',
							pScreen->pText[i].fScale, 
							_apwszPhrases[WPR_DATATYPES_PHRASES_PRIMARY_LIMIT1 + _MenuState.MR_WorkingRules.nLimitPrimaryWeapon] );
				break;	

			case _MENU_ITEMS_MR_ONLY_SECONDARY:
				ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
							pScreen->pText[i].fUnitY,
							L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
							bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
							L'C',
							pScreen->pText[i].fScale, 
							_apwszPhrases[WPR_DATATYPES_PHRASES_SECONDARY_LIMIT1 + _MenuState.MR_WorkingRules.nLimitSecondaryWeapon] );
				break;

			case _MENU_ITEMS_MR_ONLY_VEHICLE:
				ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
					pScreen->pText[i].fUnitY,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
					bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
					L'C',
					pScreen->pText[i].fScale, 
					_apwszPhrases[WPR_DATATYPES_PHRASES_VEHICLE_LIMIT1 + _MenuState.MR_WorkingRules.nLimitVehicles] );
				break;

			case _MENU_ITEMS_MR_TIME_LIMIT:
				// If this is death match, a time of zero means no limit
				if( (_MenuState.MR_WorkingRules.nGameType == GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH) && (_MenuState.MR_WorkingRules.nTimeLimit == 0) ) {
					// no limit
					ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
						pScreen->pText[i].fUnitY,
						L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
						bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
						L'C',
						pScreen->pText[i].fScale, 
						_apwszPhrases[WPR_DATATYPES_PHRASES_NO_LIMIT] );
				}
				else {
					pwszEndString = ( _MenuState.MR_WorkingRules.nTimeLimit == 1 ) ? _apwszPhrases[WPR_DATATYPES_PHRASES_MINUTE] : _apwszPhrases[WPR_DATATYPES_PHRASES_MINUTES];
					
					ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
								pScreen->pText[i].fUnitY,
								L"~f1~C%ls~w0~a%lc~s%.2f%2d %ls", 
								bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
								L'C',
								pScreen->pText[i].fScale, 
								_MenuState.MR_WorkingRules.nTimeLimit,
								pwszEndString );
				}
				break;

			case _MENU_ITEMS_MR_DM_POINTS_TO_WIN:
				if( _MenuState.MR_WorkingRules.nDMKillsPerRound ) {
					ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
								pScreen->pText[i].fUnitY,
								L"~f1~C%ls~w0~a%lc~s%.2f%2d", 
								bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
								L'C',
								pScreen->pText[i].fScale, 
								_MenuState.MR_WorkingRules.nDMKillsPerRound );
				} else {
					// no limit
					ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
								pScreen->pText[i].fUnitY,
								L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
								bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
								L'C',
								pScreen->pText[i].fScale, 
								_apwszPhrases[WPR_DATATYPES_PHRASES_NO_LIMIT] );
				}
				break;

			case _MENU_ITEMS_MR_KOTH_SWAP_TIME:
				if( _MenuState.MR_WorkingRules.nTimeBetweenHillSwaps ) {
					pwszEndString = ( _MenuState.MR_WorkingRules.nTimeBetweenHillSwaps == 1 ) ? _apwszPhrases[WPR_DATATYPES_PHRASES_MINUTE] : _apwszPhrases[WPR_DATATYPES_PHRASES_MINUTES];

					ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
								pScreen->pText[i].fUnitY,
								L"~f1~C%ls~w0~a%lc~s%.2f%2d %ls", 
								bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
								L'C',
								pScreen->pText[i].fScale, 
								_MenuState.MR_WorkingRules.nTimeBetweenHillSwaps,
								pwszEndString );
				} else {
					// no limit
					ftext_Printf( _MULTIPLAYER_RULES_OPTIONS_X,
								pScreen->pText[i].fUnitY,
								L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
								bSelected ? WprDataTypes_pwszWhiteTextColor : WprDataTypes_pwszGrayTextColor,
								L'C',
								pScreen->pText[i].fScale, 
								_apwszPhrases[WPR_DATATYPES_NO_HILL_SWAPS] );
				}
				break;

			case _MENU_ITEMS_MR_POSSESSABLE_BOTS:
				_DrawOnOffSelection( _MULTIPLAYER_RULES_OPTIONS_X,
									pScreen->pText[i].fUnitY,
									bSelected,
									pScreen->pText[i].fScale,
									_MenuState.MR_WorkingRules.nFlags & GAMESAVE_RULE_FLAGS_POSSESSABLE_BOTS );
				break;

			case _MENU_ITEMS_MR_POSSESSION_BOT_STATE:
				_DrawOnOffSelection( _MULTIPLAYER_RULES_OPTIONS_X,
					pScreen->pText[i].fUnitY,
					bSelected,
					pScreen->pText[i].fScale,
					_MenuState.MR_WorkingRules.nFlags & GAMESAVE_RULE_FLAGS_BOTS_ON_WHEN_NOT_TETHERED );
				break;
			}
		}
	}
}

static void _MultiRules_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	BOOL bToggleSpin = TRUE;

	// By default, we use a profile, not the working rules to init a game
	_MenuState.nMRTmpRuleType = -1;

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// don't save changes then go back to the previous screen
		_MenuState.nCurrentScreen = _MenuState.nLastScreen;
		_MenuState.nCurItemIndex = 0;
		_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_RULES;

		if( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_MULTI_NEW_TYPE )
			_ME_ResetKeyboard();

		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// save changes and go back to the previous screen

		// Little trick here; if the last screen was the NEW screen, then we 
		// skip over it to the edit screen. Otherwise, really go back to the 
		// last screen.
		if( _MenuState.nLastScreen == WPR_DATATYPES_SCREENS_MULTI_NEW_TYPE )
			_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES;

		_MenuState.nCurrentScreen = _MenuState.nLastScreen;
		_MenuState.nCurItemIndex = 0;

		// do saving logic here
		if( _MenuState.bMRMadeChanges ) {

			// find the correct player number
			CPlayerProfile *pProfile = (_MenuState.nMRProfileIndex >= 0) ? _MenuState.apMEUniqueProfiles[_MenuState.nMRProfileIndex] : NULL;
			
			// make sure that this player's profile isn't virtual
			if( pProfile && !pProfile->IsVirtual() ) {
				// If we are appending, move up to the next slot
				if( _MenuState.bMEAppend ) {
					FASSERT( pProfile->m_Data.nMPRulesCount < MP_RULE_SET_MAX );
					_MenuState.nMRRuleIndex = pProfile->m_Data.nMPRulesCount++;
				}

				// copy the rules data to the profile
				pProfile->m_Data.aMultiPlayerRules[_MenuState.nMRRuleIndex] = _MenuState.MR_WorkingRules;

				// instead of heading straight back to the type screen, save the profile
				_SetupMemCardSave( (Wpr_DataTypes_Screens_e)_MenuState.nLastScreen, pProfile );
			}
			else {
				// No profile or a virtual profile, so start the game from our working rules
				_MenuState.nMRTmpRuleType = _MenuState.nMTType;
			}
		}
		_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_RULES;
		_MenuState.bMRMadeChanges = FALSE;
		break;

	case WPR_DATATYPES_NAV_CODE_CONFIRM:
		// reset the rules to the defaults
		_MT_InitRulesFromType( &_MenuState.MR_WorkingRules, _MenuState.nMTType, TRUE, _MenuState.bMEAppend );
		_MenuState.bMRMadeChanges = TRUE;
		bToggleSpin = FALSE;
		break;

	default:
		FASSERT_NOW;
		break;
	}
}

///////////////////////////
// MP - 'multi pick teams'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiTeam_Work( void ) {
	
	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

	u32 nCache = _MenuState.nMPTeamAHumanMask;
	u32 i, nMask;
	BOOL bBalanceTeams = FALSE;
	_LeftRight_e nLeftRight;		

	for( i=0; i < MAX_PLAYERS; i++ ) {
		nMask = (1 << i);
		if( _MenuState.nMPPlayerMask & nMask ) {
			// this player is in the round

			nLeftRight = _CheckLeftRightAxis( _paProfiles[i].m_nControllerIndex );
			if( nLeftRight == _LEFT ) {
				// put the player on Team A
				_MenuState.nMPTeamAHumanMask |= nMask;
			} else if( nLeftRight == _RIGHT ) {
				// remove the player from Team A, putting them on Team B
				_MenuState.nMPTeamAHumanMask &= ~nMask;
			}
			
			// check for the Y button (Rebalance Teams)
			if( !bBalanceTeams && _CheckYButton( _paProfiles[i].m_nControllerIndex ) ) {
				bBalanceTeams = TRUE;
			}
		}
	}
	if( nCache != _MenuState.nMPTeamAHumanMask ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_CHANGE_LETTERS] );
	}

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_ABY_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MultiTeam_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MULTI_TEAM];
	
	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	// draw the team boxes
	Wpr_DataTypes_MeshLayout_t Layout;
	Layout.pMeshInst = wpr_datatypes_FindMeshInst( "gfh_box06", _nNumMeshes, _paMeshInsts );
	if( Layout.pMeshInst ) {
		Layout.fDrawZ = pScreen->pMesh[pScreen->nNumMeshElements-1].fDrawZ - WPR_DATATYPES_LAYER_Z;
		Layout.fScale = 1.30f;

		Layout.fBiPolarUnitX = -0.30f;
		Layout.fBiPolarUnitY = -0.05f;
		
		wpr_drawutils_DrawMesh_WithRot( &Layout, 
			fScaleMultiplier, fHalfXRes, fHalfYRes, 
			0.0f, 0.0f, FMATH_DEG2RAD( 90.0f ) );

		Layout.fBiPolarUnitX = 0.30f;
				
		wpr_drawutils_DrawMesh_WithRot( &Layout, 
			fScaleMultiplier, fHalfXRes, fHalfYRes, 
			0.0f, 0.0f, FMATH_DEG2RAD( 90.0f ) );
	}

	// draw the human players
	f32 fAx, fAy, fBx, fBy;
	u32 i, nMask;
	fAx = pScreen->pText[_MENU_ITEMS_MP_TEAM_A_OFFSET].fUnitX;
	fAy = pScreen->pText[_MENU_ITEMS_MP_TEAM_A_OFFSET].fUnitY + _PICK_TEAMS_TITLE_Y_OFFSET;
	fBx = pScreen->pText[_MENU_ITEMS_MP_TEAM_B_OFFSET].fUnitX;
	fBy = pScreen->pText[_MENU_ITEMS_MP_TEAM_B_OFFSET].fUnitY + _PICK_TEAMS_TITLE_Y_OFFSET;
	for( i=0; i < MAX_PLAYERS; i++ ) {
		nMask = (1 << i);
		if( _MenuState.nMPPlayerMask & nMask ) {
			// this player is in the round, are they on Team A or B
			if( nMask & _MenuState.nMPTeamAHumanMask ) {
				// Team A
				ftext_Printf( fAx, fAy,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls %d", 
					WprDataTypes_pwszWhiteTextColor,
					L'c',
					_PICK_TEAMS_FONT_SCALE, 
					_apwszPhrases[WPR_DATATYPES_PHRASES_PLAYER],
					i+1 );
				fAy += _PICK_TEAMS_ENTRY_Y_OFFSET;
			} else {
				// Team B
				ftext_Printf( fBx, fBy,
					L"~f1~C%ls~w0~a%lc~s%.2f%ls %d", 
					WprDataTypes_pwszWhiteTextColor,
					L'c',
					_PICK_TEAMS_FONT_SCALE, 
					_apwszPhrases[WPR_DATATYPES_PHRASES_PLAYER],
					i+1 );
				fBy += _PICK_TEAMS_ENTRY_Y_OFFSET;
			}
		}
	}
}

static void _MultiTeam_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	// draw left right arrows
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	wpr_drawutils_DrawLeftRightArrow( -0.50f,
						 0.51f,
						 0.12f,
						 65.0f,
						 fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _MultiTeam_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	BOOL bToggleSpin = TRUE;

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the Game Type screen
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_TYPE;
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// go to the Multi Level screen
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_MULTI_LEVEL;
		break;

	case WPR_DATATYPES_NAV_CODE_CONFIRM:
		// toggle up the ai, but don't switch screens
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		bToggleSpin = FALSE;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_TEAM;
}

///////////////////////////
// ML - 'multi pick level'
//////////////////////////
static Wpr_DataTypes_NavCode_e _MultiLevel_Work( void ) {
	
	// check for the A button
	if( _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		if( _MenuState.nCurItemIndex >= (_MenuState.MLUnLockInfo.nNumFreebies + _MenuState.nMLNumUnlockedLevels) ) {
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_NO_CAN_DO] );
		} else {
			fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
			return WPR_DATATYPES_NAV_CODE_FORWARD;
		}		
	}
	// check for the B button
	if( _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

	// change the current level (L/R changes)
	u32 nNumLevels = _MenuState.MLLevelSelectInfo.nNumLevels - 1;

	_HandleAxisSelections( TRUE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex,
		nNumLevels,
		TRUE,
		_ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

	_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _MultiLevel_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_MULTI_LEVEL];
	_LevelSelectInfo_t *pLevelSelect;

	pLevelSelect = &_MenuState.MLLevelSelectInfo;
	
	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );

	// print "DeathMatch Level 36"
	ftext_Printf( 0.5f, 0.25f,
				  L"~f1~C%ls~w0~a%lc~s%.2f%ls %ls %d", 
				  WprDataTypes_pwszBlueTextColor,
				  L'c',
				  1.0f, 
				  _MT_GetTypeName( _MenuState.nMTType ),
				  _apwszPhrases[WPR_DATATYPES_PHRASES_LEVEL],
				  _MenuState.nCurItemIndex+1 );
	// print the level name
	ftext_Printf( 0.5f, 0.555f,
				  L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
				  WprDataTypes_pwszBlueTextColor,
				  L'C',
				  0.85f, 
				  pLevelSelect->papwzNames[_MenuState.nCurItemIndex] );
	// print the work "locked" over the main screen shot
	if( _MenuState.nCurItemIndex >= (_MenuState.MLUnLockInfo.nNumFreebies + _MenuState.nMLNumUnlockedLevels) ) {
		// draw the lock warning
		ftext_Printf( 0.5f, 0.375f,
				  L"~f1~C%ls~w0%ls~a%lc~s%.2f%ls", 
				  L"88300685",
				  WprDataTypes_pwszHiLightedBlink,
				  L'C',
				  1.30f, 
				  _apwszPhrases[WPR_DATATYPES_PHRASES_LOCKED] );			
	}	

	// draw which player is in control of the selection
	_DrawSelectingPlayerMsg();
}

static void _MultiLevel_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	_LevelSelectInfo_t *pLevelSelect;

	pLevelSelect = &_MenuState.MLLevelSelectInfo;

	// draw the current screenshot
	wpr_drawutils_DrawTextureToScreen( (_MenuState.nCurItemIndex < (_MenuState.MLUnLockInfo.nNumFreebies + _MenuState.nMLNumUnlockedLevels)),
					 &pLevelSelect->paTexInsts[_MenuState.nCurItemIndex],
					 -0.0f, -0.12f,
					 0.70f,
                     fScaleMultiplier, fHalfXRes, fHalfYRes );	

	// draw the inactive screenshots
	if( pLevelSelect->nNumLevels > 1 ) {
		s32 nLeftIndex = _MenuState.nCurItemIndex - 1;
		if( nLeftIndex < 0 ) {
			// draw the last item
			nLeftIndex = (pLevelSelect->nNumLevels - 1);
		}
		wpr_drawutils_DrawTextureToScreen( FALSE,
						 &pLevelSelect->paTexInsts[nLeftIndex],
						 -0.45f, -0.12f,
						 0.40f,
						 fScaleMultiplier, fHalfXRes, fHalfYRes );

		s32 nRightIndex = _MenuState.nCurItemIndex + 1;
		if( nRightIndex >= (s32)pLevelSelect->nNumLevels ) {
			nRightIndex = 0;
		}
		if( nRightIndex != nLeftIndex ) {
			wpr_drawutils_DrawTextureToScreen( FALSE,
							 &pLevelSelect->paTexInsts[nRightIndex],
							 0.45f, -0.12f,
							 0.40f,
							 fScaleMultiplier, fHalfXRes, fHalfYRes );
		}		
	}

	// draw left right arrows
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	wpr_drawutils_DrawLeftRightArrow( -0.615f,
						 0.615f,
						 0.0f,
						 45.0f,
						 fScaleMultiplier, fHalfXRes, fHalfYRes );

}

static void _MultiLevel_MP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back to the last screen, either the pick team or game type
		if( _MenuState.nMTType ) {
			// going back to the pick teams screen
            _MenuState.nCurItemIndex = 0;
		} else {
			// going back to the pick type screen
			_MenuState.nCurItemIndex = 0;
		}
		_MenuState.nCurrentScreen = _MenuState.nLastScreen;
		break;

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// play the game
		_MenuState.nLevelToPlay = (u8)_MenuState.nCurItemIndex;
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NONE;
		_MenuState.fModeTimer = 0.0f;// reset the timer
		_MenuState.nStartGameMethod = _START_METHOD_MULTIPLAYER;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_MULTI_LEVEL;
}

static Wpr_DataTypes_NavCode_e _PrepareToLoad_Work( void ) {
	u32 i, nMask;

	_MenuState.fModeTimer += FLoop_fPreviousLoopSecs;
	if( _MenuState.fModeTimer >= (_TOTAL_FADE_OUT_TIME*1.1f) ) {// hold the black screen just a little longer
		// fill in the game init structure
		FASSERT( _MenuState.nMode == WPR_DATATYPES_MODES_SINGLE_PLAYER || _MenuState.nMode == WPR_DATATYPES_MODES_MULTIPLAYER );

		fang_MemZero( &_GameInitInfo, sizeof( GameInitInfo_t ) );

		if( _MenuState.nMode == WPR_DATATYPES_MODES_SINGLE_PLAYER ) {
			// setup a single palyer game
			_GameInitInfo.apProfile[0] = &_paProfiles[0];
			_paProfiles[0].m_nControllerIndex = _MenuState.nControllerIndex;
			_GameInitInfo.nNumPlayers = 1;
			_GameInitInfo.bSinglePlayer = TRUE;
			_GameInitInfo.nDifficultyLevel = _paProfiles[0].m_Data.nDifficulty;
			if( _MenuState.nStartGameMethod == _START_METHOD_NEW ) {
				_GameInitInfo.bNewGame = TRUE;
				_GameInitInfo.nLevelToPlay = _MenuState.PLSelectInfo.panLevelNumbers[0] - 1;
			} else if( _MenuState.nStartGameMethod == _START_METHOD_CONTINUE ) {
				_GameInitInfo.nLevelToPlay = _MenuState.PLSelectInfo.panLevelNumbers[ _paProfiles[0].m_Data.nCurrentLevel ] - 1;				
			} else if( _MenuState.nStartGameMethod == _START_METHOD_REPLAY ) {
				_GameInitInfo.bReplayingALevel = TRUE;
				_GameInitInfo.nLevelToPlay = _MenuState.PLSelectInfo.panLevelNumbers[_MenuState.nLevelToPlay] - 1;
			}

			FASSERT( _GameInitInfo.nLevelToPlay < LEVEL_SINGLE_PLAYER_COUNT );
		} else {
			_GameInitInfo.bSinglePlayer = FALSE;
			_GameInitInfo.nLevelToPlay = _MenuState.MLLevelSelectInfo.panLevelNumbers[_MenuState.nLevelToPlay] - 1;
			
			if( _MenuState.nMRTmpRuleType == _MenuState.nMTType )
				_MenuState.MPRules = _MenuState.MR_WorkingRules;
			else
				_MT_InitRulesFromType( &_MenuState.MPRules, _MenuState.nMTType, TRUE, FALSE );

			_GameInitInfo.pMultiplayerRules = &_MenuState.MPRules;
			_GameInitInfo.bTeamPlay = (_MenuState.nMTMode == 1);
			
			_GameInitInfo.nNumPlayers = 0;
			for( i=0; i < MAX_PLAYERS; i++ ) {
				nMask = 1 << i;
				if( _MenuState.nMPPlayerMask & nMask ) {
					// player i is in the game
					_GameInitInfo.apProfile[_GameInitInfo.nNumPlayers] = &_paProfiles[i];
					if( _MenuState.nMPTeamAHumanMask & nMask ) {
						_GameInitInfo.nTeamAHumanMask |= (1 << _GameInitInfo.nNumPlayers);
					}

					// increase the number of players
					_GameInitInfo.nNumPlayers++;
				}
			}
		}

		// call the launcher to tell them to start the game
		launcher_StartGame( LAUNCHER_FROM_WRAPPERS, FALSE );
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _PrepareToLoad_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
		
	f32 fUnitPercent = _MenuState.fModeTimer * (1.0f/_TOTAL_FADE_OUT_TIME);
	if( fUnitPercent >= 1.0f ) {
		fUnitPercent = 1.0f;
	}
	fUnitPercent = fmath_UnitLinearToSCurve( fUnitPercent );

	game_DrawSolidFullScreenOverlay( (1.0f - fUnitPercent), 0.0f );
}

///////////////////////////////
// 'error loading' warning functions
///////////////////////////////

static Wpr_DataTypes_NavCode_e _ErrorLoading_Work( void ) {
		
	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}	
	
	// check for up/down changes
	_HandleAxisSelections( FALSE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex, (_MENU_ITEMS_EL_COUNT-1), 
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _ErrorLoading_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_ERROR_LOAD];
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;
	
	// print the profile name
	ftext_Printf( 0.5f, 0.215f, L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
		WprDataTypes_pwszInstructionTextColor,
		L'C',
		1.00f,  
		pSaveInfo->wszProfileName );
		
	// draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_EL_START_OFFSET],
		&_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
		fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _ErrorLoading_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
	
		switch( _MenuState.nCurItemIndex ) {
		
		default:
		case _MENU_ITEMS_EL_SELECT_DIFF_PROFILE:
			_SelectProfile_SetupReturnVisit( _MenuState.pPPDevInfo, pSaveInfo->wszProfileName );
			break;
			
		case _MENU_ITMES_EL_RESET_PROFILE:
			// default the settings
			_paProfiles[0].InitNewProfile( FALSE );
			// flag this profile for saving (actually rename the file to the same name)
			_paProfiles[0].m_SaveInfo.nFlags |= (GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING | GAMESAVE_SAVE_INFO_FLAGS_RENAME);

			// set up state for next screen
			_SetupMemCardSave( WPR_DATATYPES_SCREENS_LAUNCH, &_paProfiles[0] );
#if 0
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;
			_MenuState.bLMNewProfile = TRUE;
			_MenuState.nLMCodeState = _LAUNCH_CODE_NOT_STARTED;
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_START_NEW;
#endif
			break;
			
		case _MENU_ITMES_EL_NO_SAVE:
			// warn the user about not being able to save
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NO_SAVE_WARNING;			
			// make sure that the save info is updated
			pSaveInfo->nStorageDeviceID = FSTORAGE_DEVICE_ID_NONE;
			pSaveInfo->wszCardName[0] = 0;
			pSaveInfo->wszProfileName[0] = 0;
			pSaveInfo->nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
			break;
			
		case _MENU_ITEMS_EL_DELETE:
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_DELETE_PROFILE;
			break;
			
		case _MENU_ITEMS_EL_RETRY:
			_TryToReadProfileAndSetupProperResponseState( TRUE );
			break;			
		}
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		_SelectProfile_SetupReturnVisit( _MenuState.pPPDevInfo, pSaveInfo->wszProfileName );
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_ERROR_LOAD;
}


///////////////////////////////
// 'hdspace' warning functions
///////////////////////////////

static Wpr_DataTypes_NavCode_e _WarningHDSpace_Work( void ) {

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );		
}

static void _WarningHDSpace_DrawOrtho( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_WARNING_HDSPACE];

	wpr_system_DrawBasicScreen( pScreen, 
		_MenuState.nCurItemIndex + _MENU_ITEMS_PS_START_OFFSET,
		FALSE,
        fScaleMultiplier, fHalfXRes, fHalfYRes );	

	if( _MenuState.uHDNeededBlocks > 1 ) {
		ftext_Printf( 0.5f, 0.35f, L"~f1~C%ls~w0~a%lc~s%.2f%ls %d %ls", WprDataTypes_pwszMessageTextColor, L'C', 0.9f,  
						_apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_0], _MenuState.uHDNeededBlocks, _apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_1] );
	} else {
		ftext_Printf( 0.5f, 0.35f, L"~f1~C%ls~w0~a%lc~s%.2f%ls %d %ls", WprDataTypes_pwszMessageTextColor, L'C', 0.9f,  
						_apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_0], _MenuState.uHDNeededBlocks, _apwszPhrases[WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_2] );
	}
}


static void _WarningHDSpace_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// want to continue, set up state for next screen
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// get out & go to the dashboard
		gameloop_SetExitDestination( GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM );
		gameloop_ScheduleExit();
		//_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	//_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_WARNING_HDSPACE;
}


///////////////////////////////
// 'format Memory card' functions
///////////////////////////////

static Wpr_DataTypes_NavCode_e _WarningFormatCard_Work( void ) {

	// Make sure this card is still in the device...
	if( !_IsCardStillAttached( _MenuState.eFCMemCardID, _MenuState.pwszFCDeviceName, FALSE ) ) {
		// go back to the memory unit select screen and start over
		_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
		_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;						
	}
	
	_MenuState.paMUEntries[ _MenuState.nFCEntryArrayID ].bFormatOffered = TRUE;
	
	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}	
	
	// check for up/down changes
	_HandleAxisSelections( FALSE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex, (_MENU_ITEMS_FC_COUNT-1), 
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}


static void _WarningFormatCard_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_WARNING_FORMATCARD];
	
	// we need to draw the name of the device thats unformatted :
	f32 fUnitY = ( pScreen->pText[ 1 ].fUnitY + pScreen->pText[ 2 ].fUnitY ) *0.5f;
	ftext_Printf( 0.5f, fUnitY, L"~f1~C%ls~w0~aC~s%.2f%ls", WprDataTypes_pwszBlueTextColor, pScreen->pText[1].fScale,
			_MenuState.pwszFCDeviceName );
	
	
	// draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_FC_START_OFFSET],
						  &_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
						  fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _WarningFormatCard_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
	
		// go to the various options screens
		switch( _MenuState.nCurItemIndex ) {

			case _MENU_ITEMS_FC_CONTINUEWITHOUTFORMAT:
				// the user wants to back out of here...
				// update the current device infos
				_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
				_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
				break;
			case _MENU_ITEMS_FC_FORMAT:
				// Should we go to a format confirm messagebox here?
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_CONFIRM_FORMAT;
				break;

			case _MENU_ITEMS_FC_RETRY:
				// We should go back to the single player mode, which
				// Will in turn re-evaluate the card and possibly come back here...
				_MenuState.paMUEntries[ _MenuState.nFCEntryArrayID ].bFormatOffered = FALSE;
				_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
				_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
				
				break;
			default:
				FASSERT_NOW;
				break;
		}
		break;
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// the user wants to back out of here...
		_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
		_MenuState.nCurItemIndex = 0;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_WARNING_FORMATCARD;
}


/////////////////////////////////////
// WPR_DATATYPES_SCREENS_ERROR_CREATE
/////////////////////////////////////

static Wpr_DataTypes_NavCode_e _Error_Create_Work( void ) {

	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_ABY_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}
		
	// check for the Y button
	if( _CheckYButton( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_ALTERNATE;
	}

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _Error_Create_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// want to continue, set up state for next screen
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_PROFILE_SELECT;
		_MenuState.nCurItemIndex = 0;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// go back
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;
		break;

	case WPR_DATATYPES_NAV_CODE_ALTERNATE:
		// get out & go to the dashboard
		gameloop_SetExitDestination( GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM );
		gameloop_ScheduleExit();
		break;

	default:
		FASSERT_NOW;
		break;
	}

	//_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_ERROR_CREATE;
}


///////////////////////////////
// WPR_DATATYPES_SCREENS_ERROR_RENAME
///////////////////////////////

static Wpr_DataTypes_NavCode_e _Error_Rename_Work( void ) {

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );	
}

static void _Error_Rename_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// want to continue, set up state for next screen
		if( _MenuState.bLMNewProfile ) {
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_START_NEW;
		} else {
			if( _paProfiles[0].m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER ) {
				// the player is finished with the game, default to replay
				_MenuState.nCurItemIndex = _MENU_ITEMS_LM_REPLAY;
			} else {
				_MenuState.nCurItemIndex = _MENU_ITEMS_LM_CONTINUE;
			}
		}
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;			
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		_MenuState.nCurItemIndex = _MENU_ITEMS_LM_CONTINUE;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME;
		break;

	default:
		FASSERT_NOW;
		break;
	}
}


///////////////////////////////
// WPR_DATATYPES_SCREENS_ERROR_RENAME_SPACE
///////////////////////////////

static Wpr_DataTypes_NavCode_e _Error_RenameSpace_Work( void ) {
	
	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );		
}

static void _Error_RenameSpace_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {

	switch( nNavCode ) {

	case WPR_DATATYPES_NAV_CODE_FORWARD:
		// want to continue, set up state for next screen
		_MenuState.nButtonDrawMask = WPR_DATATYPES_DRAW_AB_BUTTONS;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;

	case WPR_DATATYPES_NAV_CODE_BACK:
		// get out & go to the dashboard
		gameloop_SetExitDestination( GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM );
		gameloop_ScheduleExit();
		//_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;

	default:
		FASSERT_NOW;
		break;
	}
}

static s32 _BumpNumber( s32 nNumberIn, s32 nSmallIncr, s32 nThreshold, s32 nBigIncr ) {
	s32 nBump;

	if( nNumberIn < nThreshold )
		nBump = nSmallIncr;
	else if( nNumberIn > nThreshold )
		nBump = nBigIncr;
	else {
		// nNumberIn == nThreshold
		if( nSmallIncr <= 0 )
			nBump = nSmallIncr;
		else
			nBump = nBigIncr;
	}
	return nNumberIn + nBump;
}

/////////////////////////////////////
// the difficulty level functions

static Wpr_DataTypes_NavCode_e _DifficultyLevel_Work( void ) {

	Wpr_DataTypes_NavCode_e nRet = _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );
	if( nRet != WPR_DATATYPES_NAV_CODE_NOTHING ) {
		return nRet;
	}
	
	// check for up/down changes
	_HandleAxisSelections( FALSE, _MenuState.nControllerIndex,
		_MenuState.nCurItemIndex, (_MENU_ITEMS_DL_COUNT-1), 
		FALSE, _ahSounds[WPR_DATATYPES_SOUNDS_CURSOR_MOVED] );

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _DifficultyLevel_DrawFDraw( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_ScreenData_t *pScreen = &Wpr_DataTypes_paScreenData[WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL];
	
	// draw the arrows next to the selected text
	wpr_drawutils_DrawSelectionArrows( &pScreen->pText[_MenuState.nCurItemIndex + _MENU_ITEMS_DL_START_OFFSET],
						  &_paTexInsts[WPR_DATATYPES_TEXTURES_ARROW],	
						  fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _DifficultyLevel_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	GameSave_SaveInfo_t *pSaveInfo = &_paProfiles[0].m_SaveInfo;
	GameSave_ProfileData_t *pProfile = &_paProfiles[0].m_Data;

	if( _paProfiles[0].IsVirtual() ) {
		// the profile is virtual...

		switch( nNavCode ) {
			
		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// move forward and start the game...
			pProfile->nDifficulty = (GameSave_Difficulty_e)_MenuState.nCurItemIndex;
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NONE;
			_MenuState.fModeTimer = 0.0f;// reset the timer
			break;

		case WPR_DATATYPES_NAV_CODE_BACK:
			// the user wants to pick a memory card...
			_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
			_MenuState.nCurItemIndex = 0;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
			break;

		default:
			FASSERT_NOW;
			break;
		}

	} else {
		// the profile is not virtual...

		switch( nNavCode ) {
			
		case WPR_DATATYPES_NAV_CODE_FORWARD:
			// move forward...
			if( pProfile->nDifficulty != _MenuState.nCurItemIndex ) {
				pProfile->nDifficulty = (GameSave_Difficulty_e)_MenuState.nCurItemIndex;
				pSaveInfo->nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
			}

			// move to the saving memcard screen
			if( pSaveInfo->nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING ) {
				_SetupMemCardSave( WPR_DATATYPES_SCREENS_NONE, &_paProfiles[0] ); 
			} else {
				// start the game
				_MenuState.nCurItemIndex = 0;
				_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_NONE;
				_MenuState.fModeTimer = 0.0f;// reset the timer
			}
			break;

		case WPR_DATATYPES_NAV_CODE_BACK:
			// return to the launch screen
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_START_NEW;
			_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;			
			break;

		default:
			FASSERT_NOW;
			break;
		}
	}

	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL;
}

///////////////////////////////////////////
// 'confirm format' confirmation functions
///////////////////////////////////////////

static Wpr_DataTypes_NavCode_e _ConfirmFormat_Work( void ) {

	// Make sure this card is still in the device...
	if( !_IsCardStillAttached( _MenuState.eFCMemCardID, _MenuState.pwszFCDeviceName, FALSE ) ) {
		// go back to the memory unit select screen and start over
		_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
		_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;						
	}

	return _GenericButtonWork( TRUE, TRUE, WPR_DATATYPES_DRAW_AB_BUTTONS );		
}

static void _ConfirmFormat_SP_ExitDecisions( Wpr_DataTypes_NavCode_e nNavCode ) {
	u32 nConnected, nInserted, nRemoved;

	switch( nNavCode ) {
		
	case WPR_DATATYPES_NAV_CODE_FORWARD:
	
		// Format card
		// RAFHACK -- Should probably institute some error conditions here, I just don't know what they are yet!
		fstorage_FormatDevice( _MenuState.eFCMemCardID ); // go back to the main menu
		fstorage_UpdateDeviceInfos( &nConnected, &nInserted, &nRemoved, TRUE ); // FORCE AN UPDATE!
		_MUSelectUpdate( _MenuState.paMUEntries, TRUE ); // Now update our wrapper variables based on the update forced above...
		_MenuState.nCurItemIndex = _MUSelectPickDefault( _MenuState.paMUEntries );
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT;
		break;
		
	case WPR_DATATYPES_NAV_CODE_BACK:
		// the user declined to format... go back to where we were...
		_MenuState.nCurrentScreen = _MenuState.nLastScreen;
		break;

	default:
		FASSERT_NOW;
		break;
	}
	_MenuState.nLastScreen = WPR_DATATYPES_SCREENS_CONFIRM_FORMAT;
}


//////////////////////////////////////////////////////////////
// IN GAME SAVE LOGIC

#if !FANG_PLATFORM_GC
#define _MAX_PROFILE_LEN	( 15 )
	wchar _wszTmpProf[_MAX_PROFILE_LEN];
#endif

static void _ShortenString( s32 uMaxLen, wchar *pwszDest, cwchar *pwszSource ) {
	if( fclib_wcslen( pwszSource ) < uMaxLen - 4) {
		fclib_wcscpy( pwszDest, pwszSource );
	} else {
		fclib_wcsncpy( pwszDest, pwszSource, uMaxLen - 4 );
		pwszDest[uMaxLen - 4] = '.';
		pwszDest[uMaxLen - 3] = '.';
		pwszDest[uMaxLen - 2] = '.';
		pwszDest[uMaxLen - 1] = 0x0;
	}
}

static void _SafeCopyString( wchar *pwszDst, cwchar *pwszSrc, s32 nDstSize, s32 nLimitAtThisLen/*=0*/ ) {
	fclib_wcsncpy( pwszDst, pwszSrc, nDstSize );
	pwszDst[nDstSize-1] = 0;
	
	if( nLimitAtThisLen > 0 ) {
		FASSERT( nLimitAtThisLen <= (nDstSize-4) );
		
		// Check against the limit + 1 so we don't blank out just one letter
		if( fclib_wcslen( pwszDst ) > (nLimitAtThisLen + 1) ) {
			pwszDst[nLimitAtThisLen] = L'.';
			pwszDst[nLimitAtThisLen+1] = L'.';
			pwszDst[nLimitAtThisLen+2] = L'.';
			pwszDst[nLimitAtThisLen+3] = 0;			
		}
	}
}

static void _TryToReadProfileAndSetupProperResponseState( BOOL bMakeRetryTheDefaultOptionIfError ) {

	if( _paProfiles[0].LoadFromCard() ) {
		// move to the launch screen
		_SetupLaunchMenu();		
	} else {
		// trouble reading the file
		_MenuState.nCurItemIndex = (bMakeRetryTheDefaultOptionIfError) ? _MENU_ITEMS_EL_RETRY : _MENU_ITEMS_EL_SELECT_DIFF_PROFILE;
		_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_ERROR_LOAD;
	}
}

static void _SetupLaunchMenu( void ) {

	_MenuState.bLMNewProfile = (_paProfiles[0].m_Data.nCurrentLevel == 0);
	_MenuState.nLMCodeState = _LAUNCH_CODE_NOT_STARTED;

	if( _MenuState.bLMNewProfile ) {
		_MenuState.nCurItemIndex = _MENU_ITEMS_LM_START_NEW;
	} else {
		if( _paProfiles[0].m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER ) {
			// the player is finished with the game, default to replay
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_REPLAY;
		} else {
			_MenuState.nCurItemIndex = _MENU_ITEMS_LM_CONTINUE;
		}
	}
	_MenuState.nCurrentScreen = WPR_DATATYPES_SCREENS_LAUNCH;	
}

static BOOL _IG_SaveGame_Work( void ) {
	s32 i;
	s32 nDevID;
	u32 nConnected, nInserted, nRemoved;
	BOOL bGoBack;
	
	switch( _IGSaveState ) {
	case _IGSAVE_SAVE:
		if( FLoop_nRealTotalLoopTicks - _uSaveTimer < (u64)(_IGSAVE_START_DELAY_SECONDS*FLoop_nTicksPerSec) ) {
			break;
		}

		_MUSelectUpdate( _MenuState.paMUEntries, TRUE );
		_IGSaveErrorState = _IGSAVE_NONE;


		for( i=0; i<MAX_PLAYERS; i++ ) {
			if( _auIGPlayerSaveStatus[i] & _IGSAVESTATUS_SAVE ) {

				// make sure the profile is there if we're not roaming
				if( !_apIGSaveProfiles[i]->IsOnCard() && !(_auIGPlayerSaveStatus[i] & _IGSAVESTATUS_ROAMING) ) {
					if( !_IGSaveErrorState ) {
						_IGSaveErrorState = _IGSAVE_INSERT_CARD;
						_uIGErrorPlayer = i;
					}

				// otherwise, if we are roaming, make sure the profile isn't there
				} else if( _apIGSaveProfiles[i]->IsOnCard() && (_auIGPlayerSaveStatus[i] & _IGSAVESTATUS_ROAMING) ) {
					if( !_IGSaveErrorState ) {
						_IGSaveErrorState = _IGSAVE_WARN_OVERWRITE;
						_uIGErrorPlayer = i;
					}
				} else {
					// everything seems to be ok, try to save
					if( !_apIGSaveProfiles[i]->SaveToCard() ) {
						_IGSaveErrorState = _IGSAVE_ERROR;
						_uIGErrorPlayer = i;
					} else {
						_auIGPlayerSaveStatus[i] = _IGSAVESTATUS_SAVED;
						_IGSaveState = _IGSAVE_DELAY;
					}
				}
			}
		}

		// if at least one profile was saved successfully, _IGSaveState will be _IGSAVE_DELAY.  If so, go there, otherwise go to error state
		if( _IGSaveErrorState == _IGSAVE_INSERT_CARD ) {
			CMsgBox::Clear();
			FASSERT( Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile );
			nDevID = fStorage_DeviceIDToIndex( Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_SaveInfo.nStorageDeviceID );
			nDevID++;

#if FANG_PLATFORM_GC
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_MU_REMOVED], 
						Player_aPlayer[_uIGErrorPlayer].m_pwszPlayerName, _MenuState.paMUEntries[nDevID].pwszDisplayName );
#else 
			_ShortenString( _MAX_PROFILE_LEN, _wszTmpProf, Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_SaveInfo.wszCardName );
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_MU_REMOVED], 
						_wszTmpProf, _MenuState.paMUEntries[nDevID].pwszDisplayName );
#endif
			CMsgBox::Display( "IGSaveMURemoved", NULL, Wpr_DataTypes_wszTempString, Game_apwszPhrases[GAMEPHRASE_ACCEPT], NULL, Game_apwszPhrases[GAMEPHRASE_SELECTNEWLOC], 0, TRUE, _IG_SaveGame_Work );
			_IGSaveState = _IGSAVE_INSERT_CARD;

		} else if( _IGSaveErrorState == _IGSAVE_ERROR ) {
			CMsgBox::Clear();
			
			// Try and figure out what the error was...
			BOOL bNoFreeSpace = FALSE;
#if 0
			if( !Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->IsOnCard() ) {
				// See if there was any free space on the card
				u32 nRetStatus;
				if( fstorage_CanSaveNewProfile( Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_SaveInfo.nStorageDeviceID, sizeof( GameSave_ProfileData_t ), &nRetStatus ) == FSTORAGE_ERROR_NONE ) {
					bNoFreeSpace = (nRetStatus & (FSTORAGE_DEVICE_STATUS_NO_FREE_MEM | FSTORAGE_DEVICE_STATUS_NO_FREE_FILE_ENTRIES));
				}
			}
#endif
			if( bNoFreeSpace ) {
#if 0
				fstorage_CalcSaveGameSize( pEntry->pDeviceInfo->oeID, sizeof( GameSave_ProfileData_t ), &nBytesNeededToSaveProfile );
				
				_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls\n\n%ls\n\n%ls %ls\n%d %ls",
							Game_apwszPhrases[GAMEPHRASE_ERRORSAVING], 
							_apwszPhrases[WPR_DATATYPES_PHRASES_NO_ROOM_FOR_NEW_PROFILE],
							_apwszPhrases[WPR_DATATYPES_PHRASES_MA_NEEDS_0],
							_apwszPhrases[WPR_DATATYPES_PHRASES_MA_NEEDS_1],
							nBlocksNeededToSaveProfile,					
							_apwszPhrases[WPR_DATATYPES_PHRASES_MA_NEEDS_2] );				
#endif
			} else {
				_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_ERRORSAVING] );
			}
			CMsgBox::Display( "IGSaveMURemoved", NULL, Wpr_DataTypes_wszTempString, Game_apwszPhrases[GAMEPHRASE_SELECTSAVELOCATION], NULL, NULL, 0, TRUE, _IG_SaveGame_Work );
			_IGSaveState = _IGSAVE_ERROR;

		} else if( _IGSaveErrorState == _IGSAVE_WARN_OVERWRITE ) {
			CMsgBox::Clear();
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_PROFILEFOUND], '\n' );
			CMsgBox::Display( "IGSaveWarn", NULL, Wpr_DataTypes_wszTempString, Game_apwszPhrases[GAMEPHRASE_OVERWRITE], Game_apwszPhrases[GAMEPHRASE_BACK], NULL, 0, TRUE, _IG_SaveGame_Work );
			_IGSaveState = _IGSAVE_WARN_OVERWRITE;
		}
		break;

	case _IGSAVE_DELAY:
		if( FLoop_nRealTotalLoopTicks - _uSaveTimer > (_IGSAVE_DELAY_SECONDS*FLoop_nTicksPerSec) ) {
			CMsgBox::Clear();
			_IGSaveState = _IGSAVE_NONE;
		}

		break;


	case _IGSAVE_ERROR:
		if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
			CMsgBox::ClearButtonPress();
			_IGSaveDisplaySelectMUMsgBox( TRUE );
		}
		break;

	case _IGSAVE_INSERT_CARD:
		if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
			CMsgBox::ClearButtonPress();
			CMsgBox::Clear();

			_IG_StartSave();

		} else if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ALTERNATE ) {
			// Switching to another memory unit
			CMsgBox::ClearButtonPress();
			_IGSaveDisplaySelectMUMsgBox( TRUE );
		}
		break;


	case _IGSAVE_SELECTMU: {
	
		// Check to see if an unformatted card has been inserted...
		u32 i;
		// Check to see if we have a memory card inserted thats corrupted or incorrectly formated
		for( i=0; i < FSTORAGE_MAX_DEVICES; i++ ) {
			// Search for unusable offered devices that have a status of corrupted
			if( _MenuState.paMUEntries[i+1].bOfferDevice && _MenuState.paMUEntries[i+1].bDeviceUnusable &&
				( _MenuState.paMUEntries[i+1].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_ENCODING ) &&
				!_MenuState.paMUEntries[i+1].bFormatOffered ) {
					// We need to inform this guy immediately that he needs a format...
					CMsgBox::Clear();
					_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_CARDNEEDSFORMATTING], _MenuState.paMUEntries[ i+1 ].pwszDisplayName );
					CMsgBox::Display( "IGSaveNeedFormat", NULL, Wpr_DataTypes_wszTempString, Game_apwszPhrases[GAMEPHRASE_YES], Game_apwszPhrases[GAMEPHRASE_NO], NULL, 0, TRUE, _IG_SaveGame_Work );
					_MenuState.nCurItemIndex = i+1;
					_IGSaveState = _IGSAVE_CARDNEEDSFORMATTING;
					break;
			}
		}
				
		if( _IGSaveState != _IGSAVE_CARDNEEDSFORMATTING ) {
			_LeftRight_e nLeftRight = _CheckLeftRightAxis( Player_aPlayer[_uIGErrorPlayer].m_nControllerIndex );
			_MUSelectUpdate( _MenuState.paMUEntries, FALSE );

			if( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice ) {
				nLeftRight = _RIGHT;
			}

			if( nLeftRight != _NOT_LEFT_OR_RIGHT ) {
				do {
					_MenuState.nCurItemIndex += nLeftRight;

					if( _MenuState.nCurItemIndex >= (s32)_MenuState.nMUNumEntries ) {
						_MenuState.nCurItemIndex = 0;
					} else if( _MenuState.nCurItemIndex < 0 ) {
						_MenuState.nCurItemIndex = _MenuState.nMUNumEntries - 1;
					}

				} while( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice );			
				_IGSaveDisplaySelectMUMsgBox( FALSE );
			}

			if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
				CMsgBox::ClearButtonPress();
				CMsgBox::Clear();

				if( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo ) {
					// warn the user

					_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_NOPROGRESS], '\n' );
					CMsgBox::Display( "IGSave", Game_apwszPhrases[GAMEPHRASE_WARNING], Wpr_DataTypes_wszTempString, Game_apwszPhrases[GAMEPHRASE_ACCEPT], 
												Game_apwszPhrases[GAMEPHRASE_BACK], NULL, 0, TRUE, _IG_SaveGame_Work );
					_IGSaveState = _IGSAVE_WARNNOSAVE;

				} else {
					// this is a valid device.  Set up the player's profile & try to save again
					FASSERT( Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile );
		
					_auIGPlayerSaveStatus[_uIGErrorPlayer] |= _IGSAVESTATUS_ROAMING;
					Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_SaveInfo.nStorageDeviceID = _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->oeID;
					Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW;
					_IG_StartSave();
				}
			} else if ( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ALTERNATE ) {
				if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo && 
					( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_ENCODING ) ) {
					// The user wants to format this guy...
					CMsgBox::ClearButtonPress();
					CMsgBox::Clear();

					// Go to the format warning screen.
					CMsgBox::Display( "IGSaveNeedFormat", NULL, Game_apwszPhrases[GAMEPHRASE_CONFIRMFORMAT], Game_apwszPhrases[GAMEPHRASE_YES], Game_apwszPhrases[GAMEPHRASE_NO], NULL, 0, TRUE, _IG_SaveGame_Work );
					_IGSaveState = _IGSAVE_WARN_FORMAT;
				}
			}
		}
		break;
	}

	case _IGSAVE_WARNNOSAVE:

		if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
			CMsgBox::ClearButtonPress();
			CMsgBox::Clear();

			if( Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile ) {
				Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE;
			}
			_IGSaveState = _IGSAVE_NONE;

		} else if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_CANCEL ) {
			// go back to select mu
			CMsgBox::ClearButtonPress();
			_IGSaveDisplaySelectMUMsgBox( TRUE );	
		}
		break;

	case _IGSAVE_WARN_OVERWRITE:

		bGoBack = FALSE;

		// Check to make sure the card is still in there...
		_MUSelectUpdate( _MenuState.paMUEntries, FALSE );
		if( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice ) {
			bGoBack = TRUE;
		}

		if( !bGoBack && CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
			CMsgBox::ClearButtonPress();
			CMsgBox::Clear();

			_auIGPlayerSaveStatus[_uIGErrorPlayer] &= ~_IGSAVESTATUS_ROAMING;
			Player_aPlayer[_uIGErrorPlayer].m_pPlayerProfile->m_SaveInfo.nFlags &= ~GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW;
			_IG_StartSave();

		} else if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_CANCEL ) {
			// abort the overwrite
			CMsgBox::ClearButtonPress();
			bGoBack = TRUE;
		}
		
		if( bGoBack ) {
			_IGSaveDisplaySelectMUMsgBox( TRUE );	
		}
			
		break;

	case _IGSAVE_CARDNEEDSFORMATTING:
		bGoBack = FALSE;
		_MenuState.paMUEntries[_MenuState.nCurItemIndex].bFormatOffered = TRUE;
		
		// Check to make sure the card is still in there...
		_MUSelectUpdate( _MenuState.paMUEntries, FALSE );

		if( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice ) {
			bGoBack = TRUE;
		}
		
		if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
			CMsgBox::ClearButtonPress();
			CMsgBox::Clear();

			// Go to the format warning screen.
			CMsgBox::Display( "IGSaveNeedFormat", NULL, Game_apwszPhrases[GAMEPHRASE_CONFIRMFORMAT], Game_apwszPhrases[GAMEPHRASE_YES], Game_apwszPhrases[GAMEPHRASE_NO], NULL, 0, TRUE, _IG_SaveGame_Work );
			_IGSaveState = _IGSAVE_WARN_FORMAT;
			

		} else if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_CANCEL ) {

			CMsgBox::ClearButtonPress();
			bGoBack = TRUE;
		}

		if( bGoBack ) {
			_IGSaveDisplaySelectMUMsgBox( TRUE );		
		}
		break;
		
	case _IGSAVE_WARN_FORMAT:
		bGoBack = FALSE;
		
		// Check to make sure the card is still in there...
		_MUSelectUpdate( _MenuState.paMUEntries, FALSE );

		if( !_MenuState.paMUEntries[_MenuState.nCurItemIndex].bOfferDevice ) {
			bGoBack = TRUE;
		} else if( ( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) || ( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_CANCEL ) ) {
			if( CMsgBox::CheckForButtonPress() == CMsgBox::BUTTON_ACCEPT ) {
				// Format the card here....
				fstorage_FormatDevice( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->oeID ); // go back to the main menu
				fstorage_UpdateDeviceInfos( &nConnected, &nInserted, &nRemoved, TRUE ); // FORCE AN UPDATE!
			}
			CMsgBox::ClearButtonPress();
			bGoBack = TRUE;
		}

		if( bGoBack ) {
			// User is declining to format, or the card has been removed...
			_IGSaveDisplaySelectMUMsgBox( TRUE );		
		}
		
		break;

	default:
		FASSERT_NOW;
	}

	return TRUE;
}


static void _IG_StartSave( void ) {
	_uSaveTimer = FLoop_nRealTotalLoopTicks;
	_IGSaveState = _IGSAVE_SAVE;

#if FANG_PLATFORM_GC
	_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, Game_apwszPhrases[GAMEPHRASE_DONTTOUCHMU], '\n', '\n' );
	CMsgBox::Display( "IGSave", Game_apwszPhrases[GAMEPHRASE_SAVINGPROFILE], Wpr_DataTypes_wszTempString, NULL, NULL, NULL, 0, TRUE, _IG_SaveGame_Work );

#else
	CMsgBox::Display( "IGSave", NULL, Game_apwszPhrases[GAMEPHRASE_SAVINGPROFILE], NULL, NULL, NULL, 0, TRUE, _IG_SaveGame_Work );
#endif
}


// This function will clear the current messagebox and then set up the SelectMU IG wrapper
static void _IGSaveDisplaySelectMUMsgBox( BOOL bForceStatusUpdate ) {

	cwchar *pwszAText = NULL;
	cwchar *pwszBText = NULL;
	cwchar *pwszYText = NULL;
	
	wchar wszTempPhrase[ WPR_DATATYPES_TEMPSTRING_LENGTH ]; //Yikes!
	CMsgBox::Clear();

	_MUSelectUpdate( _MenuState.paMUEntries, bForceStatusUpdate );

	// Create the name of the memory unit
#if FANG_PLATFORM_GC
	_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, _MenuState.paMUEntries[_MenuState.nCurItemIndex].pwszDisplayName );
#else
	if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo && (_MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->oeID != FSTORAGE_DEVICE_ID_XB_PC_HD) ) {
		_ShortenString( _MAX_PROFILE_LEN, _wszTmpProf, _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->wszName );
		_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls\n%ls",
					_MenuState.paMUEntries[_MenuState.nCurItemIndex].pwszDisplayName, _wszTmpProf );
	} else {
		_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls",
					 _MenuState.paMUEntries[_MenuState.nCurItemIndex].pwszDisplayName );
	}
#endif

	// First, copy the old string to the temp string...		
	fclib_wcscpy( wszTempPhrase, Wpr_DataTypes_wszTempString );

	// Now, add an explanation (if necessary)
	if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].bDeviceUnusable ) {
		// We should give an explanation of WHY it's unusable...
		
		if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo && 
			( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_ENCODING ) ) {
			// This device NEEDS TO BE FORMATTED
			pwszYText = _apwszPhrases[WPR_DATATYPES_PHRASES_FORMAT];
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"~C%ls%ls\n\n~C%ls%ls\n%ls",
						WprDataTypes_pwszBlackTextColor, wszTempPhrase, 
						WprDataTypes_pwszInstructionTextColor, _apwszPhrases[WPR_DATATYPES_PHRASES_MU_UNUSABLE],
						Game_apwszPhrases[GAMEPHRASE_MUSELECT_CARDNEEDSFORMATTING] );
		} else if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo && 
			( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_WRONG_DEVICE ) ) {
			// This device is NOT A MEMORY CARD
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"~C%ls%ls\n\n~C%ls%ls\n%ls",
						WprDataTypes_pwszBlackTextColor, wszTempPhrase, 
						WprDataTypes_pwszInstructionTextColor, _apwszPhrases[WPR_DATATYPES_PHRASES_WRONG_DEVICE],
						_apwszPhrases[WPR_DATATYPES_PHRASES_INSERT_MEMORY_CARD] );
		} else if( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo && 
			( _MenuState.paMUEntries[_MenuState.nCurItemIndex].pDeviceInfo->uStatus & FSTORAGE_DEVICE_STATUS_DAMAGED ) ) {
			// This device is DAMAGED
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"~C%ls%ls\n\n~C%ls%ls",
						WprDataTypes_pwszBlackTextColor, wszTempPhrase, 
						WprDataTypes_pwszInstructionTextColor, _apwszPhrases[WPR_DATATYPES_PHRASES_DAMAGED_MEMORY_CARD_WARNING] );
		} else {
			// This is the generic YOUR SCREWED message
			_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"~C%ls%ls\n\n~C%ls%ls",
						WprDataTypes_pwszBlackTextColor, wszTempPhrase, 
						WprDataTypes_pwszInstructionTextColor, _apwszPhrases[WPR_DATATYPES_PHRASES_MU_UNUSABLE] );
		}
	} else {
		// It's all good baby, use this device...
		pwszAText = Game_apwszPhrases[GAMEPHRASE_ACCEPT];	
		// Copy the string with a valid color
		_snwprintf( Wpr_DataTypes_wszTempString, WPR_DATATYPES_TEMPSTRING_LENGTH, L"~C%ls%ls",
					WprDataTypes_pwszSolidWhiteTextColor, wszTempPhrase );
		
	}
	CMsgBox::Display( "IGSelectMU", Game_apwszPhrases[GAMEPHRASE_SELECTMU], Wpr_DataTypes_wszTempString, pwszAText, pwszBText, pwszYText, 0, TRUE, _IG_SaveGame_Work );

	_IGSaveState = _IGSAVE_SELECTMU;
}


// draws the csv specified ortho draw with nothing currently selected (nothing is flashing, all text appears as is it in the csv file)
static void _GenericDrawOrtho_NoSelections( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	wpr_system_DrawBasicScreen( &Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ], 
		-1,
		FALSE,
		fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _GenericDrawOrtho_Selections( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 nStartingSelectionOffset;

	if( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_WARNING_FORMATCARD ) {
		nStartingSelectionOffset = 4;		
	} else if( _MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_SOUND_OPTIONS ||
		_MenuState.nCurrentScreen == WPR_DATATYPES_SCREENS_PICK_COLOR ) {
		nStartingSelectionOffset = 2;
	} else {
		nStartingSelectionOffset = 1;	
	}

	wpr_system_DrawBasicScreen( &Wpr_DataTypes_paScreenData[ _MenuState.nCurrentScreen ], 
		_MenuState.nCurItemIndex + nStartingSelectionOffset,
		FALSE,
		fScaleMultiplier, fHalfXRes, fHalfYRes );
}

static void _GenericDrawOrtho_NoSelectionsWithString( Wpr_DataTypes_ScreenData_t *pScreen, f32 fStringUnitY, cwchar *pwszString, 
													 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	Wpr_DataTypes_TextLayout_t TempTextLayout;

	wpr_system_DrawBasicScreen( pScreen, 
		-1,
		FALSE,
		fScaleMultiplier, fHalfXRes, fHalfYRes );

	// fill in a temp text layout struct
	TempTextLayout.fScale = 1.0f;
	TempTextLayout.fUnitX = 0.5f;
	TempTextLayout.fUnitY = fStringUnitY;
	TempTextLayout.nAlignment = WPR_DATATYPES_ALIGN_CENTERED;
	TempTextLayout.nType = WPR_DATATYPES_REGULAR;
	TempTextLayout.pwszText = pwszString;
	TempTextLayout.fTickSpaceFactor = 17.0f;

	_DrawText( &TempTextLayout, TRUE, fScaleMultiplier, fHalfXRes, fHalfYRes );													  
}

static Wpr_DataTypes_NavCode_e _GenericButtonWork( BOOL bCheckA, BOOL bCheckB, u32 nButtonMask ) {

	// check for the A button
	if( bCheckA && _CheckAcceptButtons( _MenuState.nControllerIndex ) ) {
		fsndfx_Play2D( _ahSounds[WPR_DATATYPES_SOUNDS_SUCCESS] );
		return WPR_DATATYPES_NAV_CODE_FORWARD;
	}
	// check for the B button
	if( bCheckB && _CheckBackButtons( _MenuState.nControllerIndex ) ) {
		return WPR_DATATYPES_NAV_CODE_BACK;
	}

	_MenuState.nButtonDrawMask = nButtonMask;

	return WPR_DATATYPES_NAV_CODE_NOTHING;
}

static void _FillInStorageDeviceName( u32 nDeviceID, wchar *pwszTarget ) {
#if WPR_DATATYPES_XBOX_GRAPHICS_ON
	if( nDeviceID == 0 ) {
		// "hard disk"
		_snwprintf( pwszTarget, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls", _apwszPhrases[WPR_DATATYPES_PHRASES_HARD_DISK] );			
	} else {
		// "controller port 1 - upper"
		_snwprintf( pwszTarget, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls %ls %d - %ls", 
			_apwszPhrases[WPR_DATATYPES_PHRASES_CONTROLLER],
			_apwszPhrases[WPR_DATATYPES_PHRASES_PORT],
			( (nDeviceID-1) >> 1 ) + 1,							
			(nDeviceID & 0x1) ? _apwszPhrases[WPR_DATATYPES_PHRASES_UPPER] : _apwszPhrases[WPR_DATATYPES_PHRASES_LOWER] );
	}
#else
	if( nDeviceID == 0 ) {
		_snwprintf( pwszTarget, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls", _apwszPhrases[WPR_DATATYPES_PHRASES_MEMORY_CARD_A] );
	} else {
		_snwprintf( pwszTarget, WPR_DATATYPES_TEMPSTRING_LENGTH, L"%ls", _apwszPhrases[WPR_DATATYPES_PHRASES_MEMORY_CARD_B] );
	}
#endif	
}












