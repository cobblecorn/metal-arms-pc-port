//////////////////////////////////////////////////////////////////////////////////////
// wpr_datatypes.h - the wrapper data types - only used by the wrapper systems
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
#ifndef _WPR_DATA_TYPES_H_
#define _WPR_DATA_TYPES_H_ 1

#include "fang.h"
#include "fmesh.h"
#include "ftex.h"
#include "fstorage.h"
#include "fgamedata.h"
#include "fstringtable.h"

#if FANG_PLATFORM_XB || FANG_PLATFORM_WIN || FANG_PLATFORM_PS2	//ARG - PS2 use XBox for now
	#define WPR_DATATYPES_XBOX_GRAPHICS_ON		TRUE
#else
	#define WPR_DATATYPES_XBOX_GRAPHICS_ON		FALSE
#endif

// button draw masks
#define WPR_DATATYPES_DRAW_NO_BUTTONS			0x00000000
#define WPR_DATATYPES_DRAW_A_BUTTON_ONLY		( 1 << 0 )
#define WPR_DATATYPES_DRAW_B_BUTTON_ONLY		( 1 << 1 )
#define WPR_DATATYPES_DRAW_Y_BUTTON_ONLY		( 1 << 2 )
#define WPR_DATATYPES_DRAW_X_BUTTON_ONLY		( 1 << 3 )
#define WPR_DATATYPES_DRAW_AB_BUTTONS			( WPR_DATATYPES_DRAW_A_BUTTON_ONLY | WPR_DATATYPES_DRAW_B_BUTTON_ONLY )
#define WPR_DATATYPES_DRAW_AY_BUTTONS			( WPR_DATATYPES_DRAW_A_BUTTON_ONLY | WPR_DATATYPES_DRAW_Y_BUTTON_ONLY )
#define WPR_DATATYPES_DRAW_BY_BUTTONS			( WPR_DATATYPES_DRAW_B_BUTTON_ONLY | WPR_DATATYPES_DRAW_Y_BUTTON_ONLY )
#define WPR_DATATYPES_DRAW_ABY_BUTTONS			( WPR_DATATYPES_DRAW_AB_BUTTONS | WPR_DATATYPES_DRAW_Y_BUTTON_ONLY )
#define WPR_DATATYPES_DRAW_ABX_BUTTONS			( WPR_DATATYPES_DRAW_AB_BUTTONS | WPR_DATATYPES_DRAW_X_BUTTON_ONLY )
#define WPR_DATATYPES_DRAW_ABYX_BUTTONS			( WPR_DATATYPES_DRAW_ABY_BUTTONS | WPR_DATATYPES_DRAW_X_BUTTON_ONLY )

#define WPR_DATATYPES_TEMPSTRING_LENGTH			128
#define WPR_DATATYPES_FURTHEST_LAYER_Z			50.0f	// this layer should be the 1st layer drawn
#define WPR_DATATYPES_LAYER_Z					5.0f	// each layer is this distance from each other

// a way to ID the different screens that make up the wrappers
typedef enum {
	WPR_DATATYPES_SCREENS_NONE = 0,					// NO SCREEN AT ALL
	WPR_DATATYPES_SCREENS_SOUND_OPTIONS,			// SS - 'sound settings'
	WPR_DATATYPES_SCREENS_PROFILE_SETTINGS,			// PS - 'profile settings'
	WPR_DATATYPES_SCREENS_CONTROLLER_SETTINGS,		// CC - 'controller config'
	WPR_DATATYPES_SCREENS_ADVANCED_SETTINGS,		// AS - 'advanced settings'
	WPR_DATATYPES_SCREENS_MAIN_MENU,				// MM - 'main menu'
	WPR_DATATYPES_SCREENS_SELECT_MEMORY_UNIT,		// MU - 'select mu'
    WPR_DATATYPES_SCREENS_PROFILE_SELECT,			// PP - 'select profile'
	WPR_DATATYPES_SCREENS_DELETE_PROFILE,			//		'delete profile'
	WPR_DATATYPES_SCREENS_NO_SAVE_WARNING,			//		'no save'
	WPR_DATATYPES_SCREENS_ALREADY_EXISTS_WARNING,	//		'already exists'
	WPR_DATATYPES_SCREENS_EDIT_PROFILE_NAME,		// PN - 'profile name'
	WPR_DATATYPES_SCREENS_PICK_LEVEL,				// PL - 'pick level'
	WPR_DATATYPES_SCREENS_DONT_REMOVE_MU_WARNING,	// DR - 'don't remove mu'
	WPR_DATATYPES_SCREENS_LAUNCH,					// LM - 'launch menu'
	WPR_DATATYPES_SCREENS_START_OVER,				//		'start over'
	WPR_DATATYPES_SCREENS_PICK_COLOR,				// PC - 'pick color'
	WPR_DATATYPES_SCREENS_MULTI_JOIN,				// MJ - 'multi join'
	WPR_DATATYPES_SCREENS_MULTI_TYPE,				// MT - 'multi game type'
	WPR_DATATYPES_SCREENS_MULTI_EDIT_TYPES,			// ME - 'multi game edit types'
	WPR_DATATYPES_SCREENS_MULTI_NEW_TYPE,			// MEN - 'multi Edit New type'
	WPR_DATATYPES_SCREENS_MULTI_EDIT_NO_SAVE,		// MNS - 'multi Edit No Save'
	WPR_DATATYPES_SCREENS_MULTI_EDIT_CONFIRM_DEL,	// MD - 'multi Delete'
	WPR_DATATYPES_SCREENS_MULTI_DUPLICATE,			//      'multi duplicate' warning
	WPR_DATATYPES_SCREENS_MULTI_RULES,				// MR - 'multi game rules'
	WPR_DATATYPES_SCREENS_MULTI_TEAM,				// MP - 'multi pick teams'
	WPR_DATATYPES_SCREENS_MULTI_LEVEL,				// ML - 'multi pick level'
	WPR_DATATYPES_SCREENS_ERROR_LOAD,				// EL - 'error loading profile'
	WPR_DATATYPES_SCREENS_WARNING_HDSPACE,			// HD - 'warning hd space'
	WPR_DATATYPES_SCREENS_WARNING_FORMATCARD,		// FC	'warming format card'
	WPR_DATATYPES_SCREENS_ERROR_CREATE,				// ES - 'error saving'
	WPR_DATATYPES_SCREENS_ERROR_RENAME,				// ER - 'error renaming'
	WPR_DATATYPES_SCREENS_ERROR_RENAME_SPACE,		// RS - 'rename space'
	WPR_DATATYPES_SCREENS_DIFFICULTY_LEVEL,			// DL - 'difficulty level'
	WPR_DATATYPES_SCREENS_CONFIRM_FORMAT,			// CF - 'confirm format'

	WPR_DATATYPES_SCREEN_COUNT
} Wpr_DataTypes_Screens_e;

// a way to ID the different modes of the wrapper system
typedef enum {
	WPR_DATATYPES_MODES_SIERRA_LOGO = 0,
	WPR_DATATYPES_MODES_VUG_LOGO,
	WPR_DATATYPES_MODES_SWINGIN_LOGO,
#if FANG_PLATFORM_PS2				//CPS 5.5.03
	WPR_DATATYPES_MODES_MMI_LOGO,	//CPS 5.5.03
#endif								//CPS 5.5.03
	WPR_DATATYPES_MODES_MAIN_MENU,
	WPR_DATATYPES_MODES_SINGLE_PLAYER,
	WPR_DATATYPES_MODES_MULTIPLAYER,
	WPR_DATATYPES_MODES_MA_DEMO,
	WPR_DATATYPES_MODES_SIERRA_DEMOS,

	WPR_DATATYPES_MODES_COUNT
} Wpr_DataTypes_Modes_e;

// a list of the different general phrases that are used by the wrapper screens
typedef enum {
	WPR_DATATYPES_PHRASES_ACCEPT = 0,
	WPR_DATATYPES_PHRASES_BACK,
	WPR_DATATYPES_PHRASES_ON,
	WPR_DATATYPES_PHRASES_OFF,
	WPR_DATATYPES_PHRASES_2WAY,
	WPR_DATATYPES_PHRASES_4WAY,
	WPR_DATATYPES_PHRASES_SPACE,
	WPR_DATATYPES_PHRASES_CAPS,
	WPR_DATATYPES_PHRASES_DONE,
	WPR_DATATYPES_PHRASES_LEVEL,
	WPR_DATATYPES_PHRASES_WASHERS,
	WPR_DATATYPES_PHRASES_SECRET_CHIPS,
	WPR_DATATYPES_PHRASES_ENEMIES,
	WPR_DATATYPES_PHRASES_STATUS,
	WPR_DATATYPES_PHRASES_COMPLETE,
	WPR_DATATYPES_PHRASES_CONFIGURATION,
	WPR_DATATYPES_PHRASES_CONTROLLER,
	WPR_DATATYPES_PHRASES_UPPER,
	WPR_DATATYPES_PHRASES_LOWER,
	WPR_DATATYPES_PHRASES_PORT,
	WPR_DATATYPES_PHRASES_LEFT,
	WPR_DATATYPES_PHRASES_RIGHT,
	WPR_DATATYPES_PHRASES_NONE,
	WPR_DATATYPES_PHRASES_HARD_DISK,
	WPR_DATATYPES_PHRASES_CREATE_NEW_PROFILE,
	WPR_DATATYPES_PHRASES_PRESS_START,
	WPR_DATATYPES_PHRASES_PRESS_A_TO_JOIN,
	WPR_DATATYPES_PHRASES_DEFAULT,
	WPR_DATATYPES_PHRASES_INVERTED,
	WPR_DATATYPES_PHRASES_PRESS_A_TO_PROCEED,
	WPR_DATATYPES_PHRASES_LOCATION,
	WPR_DATATYPES_PHRASES_PRESS_A_TO_ACCEPT,
	WPR_DATATYPES_PHRASES_CHOOSE_PROFILE,
	WPR_DATATYPES_PHRASES_WAITING_FOR_OTHER_PLAYERS,
	WPR_DATATYPES_PHRASES_PLAYER,
	WPR_DATATYPES_PHRASES_SELECTING,
	WPR_DATATYPES_PHRASES_STUPID,
	WPR_DATATYPES_PHRASES_EASY,
	WPR_DATATYPES_PHRASES_REGULAR,
	WPR_DATATYPES_PHRASES_HARD,
	WPR_DATATYPES_PHRASES_INSANE,
	WPR_DATATYPES_PHRASES_RANDOM,
	WPR_DATATYPES_PHRASES_MINUTE,
	WPR_DATATYPES_PHRASES_MINUTES,
	WPR_DATATYPES_PHRASES_NO_LIMIT,
	WPR_DATATYPES_PHRASES_AI_BOT,
	WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_0,
	WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_1,		// 'more blocks.'
	WPR_DATATYPES_PHRASES_YOU_NEED_TO_FREE_2,		// 'more block.'
	WPR_DATATYPES_PHRASES_STATS,
	WPR_DATATYPES_PHRASES_COLLECTED,
	WPR_DATATYPES_PHRASES_FINISHED_IN,
	WPR_DATATYPES_PHRASES_ENEMIES_DESTROYED,
	WPR_DATATYPES_PHRASES_SECRET_AREAS_FOUND,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT1,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT2,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT3,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT4,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT5,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT6,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT7,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT8,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT9,
	WPR_DATATYPES_PHRASES_PRIMARY_LIMIT10,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT1,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT2,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT3,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT4,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT5,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT6,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT7,
	WPR_DATATYPES_PHRASES_SECONDARY_LIMIT8,
	WPR_DATATYPES_PHRASES_DEATHMATCH,			// Multiplayer meta-game types; keep the sequence
	WPR_DATATYPES_PHRASES_TIMED_DEATHMATCH,
	WPR_DATATYPES_PHRASES_POSSESSION_MELEE,
	WPR_DATATYPES_PHRASES_KING_OF_THE_HILL,
	WPR_DATATYPES_PHRASES_MOVING_HILL,
	WPR_DATATYPES_PHRASES_REVERSE_TAG,
	WPR_DATATYPES_PHRASES_TAG,					// End of Multipalyer meta-types
	WPR_DATATYPES_PHRASES_BASE_DEATHMATCH,		// Multiplayer base types, keep the sequence
	WPR_DATATYPES_PHRASES_BASE_KING_OF_HILL,
	WPR_DATATYPES_PHRASES_BASE_TAG,
	WPR_DATATYPES_PHRASES_BASE_REVERSE_TAG,		// End of multiplayer base types
	WPR_DATATYPES_PHRASES_INDIVIDUAL,
	WPR_DATATYPES_PHRASES_TEAM,
	WPR_DATATYPES_NO_HILL_SWAPS,
	WPR_DATATYPES_PHRASES_UNABLE_LOAD_PROFILE,
	WPR_DATATYPES_PHRASES_PRESS_A_TO_CONTINUE,
	WPR_DATATYPES_PHRASES_VEHICLE_LIMIT1,
	WPR_DATATYPES_PHRASES_VEHICLE_LIMIT2,
	WPR_DATATYPES_PHRASES_VEHICLE_LIMIT3,
	WPR_DATATYPES_PHRASES_VEHICLE_LIMIT4,
	WPR_DATATYPES_PHRASES_VEHICLE_LIMIT5,
	WPR_DATATYPES_PHRASES_CREATE_NEW_GAMETYPE,
	WPR_DATATYPES_PHRASES_DEFAULT_GAMETYPE_NAME,
	WPR_DATATYPES_PHRASES_BUILT_IN_TYPE,
	WPR_DATATYPES_PHRASES_TIME_TO_BEAT,
	WPR_DATATYPES_PHRASES_BONUS_SECRET_CHIP_EARNED,
	WPR_DATATYPES_PHRASES_LOCKED,
	WPR_DATATYPES_PHRASES_DELETE,
	WPR_DATATYPES_PHRASES_FORMAT,
	WPR_DATATYPES_PHRASES_MA_NEEDS_0,
	WPR_DATATYPES_PHRASES_MA_NEEDS_1,
	WPR_DATATYPES_PHRASES_MA_NEEDS_2,
	WPR_DATATYPES_PHRASES_WRONG_DEVICE,
	
// ADD NON-PLATFORM SPECIFIC PHRASES HERE...
	
	//////////////////////////////////////////////////////////////
	// THIS MUST ALWAYS BE THE LAST NON-PLATFORM SPECIFIC ENUM
	WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT,
	//////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////
	// platform specific phrases
	WPR_DATATYPES_PHRASES_MANAGEMEMORY = WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT,
	WPR_DATATYPES_PHRASES_MEMORY_CARD_A,
	WPR_DATATYPES_PHRASES_MEMORY_CARD_B,
	WPR_DATATYPES_PHRASES_MU_UNUSABLE,
	WPR_DATATYPES_PHRASES_NO_ROOM_FOR_NEW_PROFILE,
	WPR_DATATYPES_PHRASES_INSERT_MEMORY_CARD,
	WPR_DATATYPES_PHRASES_DAMAGED_MEMORY_CARD_WARNING,
	
// ADD PLATFORM SPECIFIC PHRASES HERE...
	
	////////////////////////////////////////////////////////
	// THIS MUST ALWAYS BE THE LAST PLATFORM SPECIFIC ENUMS
	WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_NEXT_INDEX,
	WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT = (WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_NEXT_INDEX - WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT),
	//////////////////////////////////////////////////////////

	WPR_DATATYPES_PHRASES_COUNT = WPR_DATATYPES_PHRASES_NON_PLATFORM_SPECIFIC_COUNT + WPR_DATATYPES_PHRASES_PLATFORM_SPECIFIC_COUNT,
} Wpr_DataTypes_Phrases_e;

// the textures that get loaded for use by the various screens
typedef enum {
	WPR_DATATYPES_TEXTURES_TICK = 0,
	WPR_DATATYPES_TEXTURES_ARROW,
	WPR_DATATYPES_TEXTURES_CONTROLLER,
	WPR_DATATYPES_TEXTURES_A_BUTTON,
	WPR_DATATYPES_TEXTURES_B_BUTTON,
	WPR_DATATYPES_TEXTURES_Y_BUTTON,
	WPR_DATATYPES_TEXTURES_X_BUTTON,
	WPR_DATATYPES_TEXTURES_YELLOW_GLITCH,
	WPR_DATATYPES_TEXTURES_BLUE_GLITCH,
	WPR_DATATYPES_TEXTURES_PURPLE_GLITCH,
	WPR_DATATYPES_TEXTURES_RED_GLITCH,
	WPR_DATATYPES_TEXTURES_GREEN_GLITCH,
	WPR_DATATYPES_TEXTURES_BLACK_GLITCH,
	
	WPR_DATATYPES_TEXTURES_COUNT
} Wpr_DataTypes_Textures_e;

// In-game version of Wpr_DataTypes_Textures_e:
typedef enum {
	WPR_DATATYPES_IG_TEXTURES_TICK = 0,
	WPR_DATATYPES_IG_TEXTURES_ARROW,
	WPR_DATATYPES_IG_TEXTURES_CONTROLLER,
		
	WPR_DATATYPES_IG_TEXTURES_COUNT
} Wpr_DataTypes_IG_Textures_e;

// the sounds that get loaded for use by the various screens
typedef enum {
	WPR_DATATYPES_SOUNDS_CURSOR_MOVED = 0,
	WPR_DATATYPES_SOUNDS_SELECT_ITEM,
	WPR_DATATYPES_SOUNDS_CHANGE_LETTERS,
	WPR_DATATYPES_SOUNDS_NO_CAN_DO,
	WPR_DATATYPES_SOUNDS_SUCCESS,
	WPR_DATATYPES_SOUNDS_VOLUME_ADJUST,

	WPR_DATATYPES_SOUNDS_COUNT
} Wpr_DataTypes_Sounds_e;

// the different text alignment options
typedef enum {
	WPR_DATATYPES_ALIGN_LEFT = 0,	// L
	WPR_DATATYPES_ALIGN_RIGHT,		// R	
	WPR_DATATYPES_ALIGN_CENTERED,	// C

	WPR_DATATYPES_ALIGN_COUNT
} Wpr_DataTypes_Alignment_e;

// text entries can be flagged as these types, which leads to their display in different colors & properties
typedef enum {
	WPR_DATATYPES_TITLE = 0,// T
	WPR_DATATYPES_SUBTITLE,	// S
	WPR_DATATYPES_REGULAR,	// R
	WPR_DATATYPES_KEYBOARD,	// K
	WPR_DATATYPES_MESSAGE,	// M
	WPR_DATATYPES_HEADING,	// H
	WPR_DATATYPES_INSTRUCTION,	// I
	WPR_DATATYPES_TEAM_A,	// A
	WPR_DATATYPES_TEAM_B,	// B

	WPR_DATATYPES_TEXTTYPE_COUNT
} Wpr_DataTypes_TextType_e;

// (From CSV) a text entry
typedef struct {
	cwchar *pwszText;
	u16 nAlignment; // should be one of Wpr_DataTypes_Alignment_e
	u16 nType;		// should be one of Wpr_DataTypes_TextType_e
	f32 fUnitX;
	f32 fUnitY;
	f32 fScale;
	f32 fTickSpaceFactor;
} Wpr_DataTypes_TextLayout_t;

// (From CSV) a mesh entry
typedef struct {
	CFMeshInst *pMeshInst;
	f32 fBiPolarUnitX;
	f32 fBiPolarUnitY;
    f32 fScale;	
	f32 fDrawZ;
} Wpr_DataTypes_MeshLayout_t;

// (From CSV) a button entry
typedef struct {
	cwchar *pwszInstructions;
	CFTexInst *pTexture;
	f32 fBiPolarUnitX;
	f32 fBiPolarUnitY;
    f32 fPixelSize;// of the texture image
	f32 fFontScale;
} Wpr_DataTypes_ButtonLayout_t;

// (From CSV) the text and mesh entries for a given screen
typedef struct {
	u16 nNumTextElements;
	u8 nNumMeshElements;
	u8 nNumButtons;
	Wpr_DataTypes_TextLayout_t *pText;	
	Wpr_DataTypes_MeshLayout_t *pMesh;	
	Wpr_DataTypes_ButtonLayout_t *paButtons;
} Wpr_DataTypes_ScreenData_t;

enum {
	WPR_DATATYPES_DRAW_LINE_A = 1,
	WPR_DATATYPES_DRAW_LINE_B = 2,
	WPR_DATATYPES_DRAW_LINE_X = 4,
	WPR_DATATYPES_DRAW_LINE_Y = 8,
	WPR_DATATYPES_DRAW_LINE_Z = 16,
	WPR_DATATYPES_DRAW_LINE_LEFTY = 32,
	WPR_DATATYPES_DRAW_LINE_LEFTX = 64,
	WPR_DATATYPES_DRAW_LINE_RIGHTY = 128,
	WPR_DATATYPES_DRAW_LINE_RIGHTX = 256,
	WPR_DATATYPES_DRAW_LINE_LTRIGGER = 512,
	WPR_DATATYPES_DRAW_LINE_RTRIGGER = 1024,
	WPR_DATATYPES_DRAW_LINE_START = 2048,
	WPR_DATATYPES_DRAW_LINE_DPADY = 4096,
	WPR_DATATYPES_DRAW_LINE_DPADX = 8192,

	WPR_DATATYPES_DRAW_LINE_NONE = 0
};

// (From CSV) this struct holds the screen location for a given axis or button element of a platform's controller diagram
typedef struct {
	cchar *pszKeyString;
	f32 fUnitX;
	f32 fUnitY;
	f32 fScale;
	u32 nLineDrawMask;// see WPR_DATATYPES_DRAW_LINE_...
} Wpr_DataTypes_ControllerTextPos_t;

// (From CSV) this struct holds the actual string to display to the user and a pointer to where to display it
typedef struct {
	cwchar *pwszStringToDisplay;
	Wpr_DataTypes_ControllerTextPos_t *pPosInfo;
} Wpr_DataTypes_ControllerText_t;

// (From CSV) this struct holds all of a controller configuration's text and to what button they are related 
typedef struct {
	u32 nNumTextFields;
	Wpr_DataTypes_ControllerText_t *paTextFields;// nNumTextFields of these
	u32 nLineDrawMask;// what lines should be drawn,see WPR_DATATYPES_DRAW_LINE_...
} Wpr_DataTypes_ControllerConfig_t;

// the 'memory unit select' data, this gets updated so that we know what to offer as a valid selection
typedef struct {
	cwchar *pwszDisplayName;
	BOOL bOfferDevice;
	BOOL bDeviceUnusable;
	BOOL bFormatOffered;		// This variable is set to true when the user is notified that their card is corrupted and can be formatted.
	const FStorage_DeviceInfo_t *pDeviceInfo;
} Wpr_DataTypes_MUSelectEntry_t;

typedef enum {
	WPR_DATATYPES_NAV_CODE_BACK = -1,		// move back a screen
	WPR_DATATYPES_NAV_CODE_FORWARD = 1,		// move forward a screen
	WPR_DATATYPES_NAV_CODE_NOTHING = 0,		// don't change screens
	WPR_DATATYPES_NAV_CODE_CONFIRM = 2,		// a button was pressed that requires a confirm message
	WPR_DATATYPES_NAV_CODE_ALTERNATE = 3	// an alternate choice, (Y button)
} Wpr_DataTypes_NavCode_e;

// function ptrs definitions
typedef Wpr_DataTypes_NavCode_e Wpr_DataTypes_WorkFcn( void ); 
typedef void Wpr_DataTypes_DrawOrthoFcn( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
typedef void Wpr_DataTypes_DrawFDrawFcn( f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
typedef void Wpr_DataTypes_DecisionsFcn( Wpr_DataTypes_NavCode_e nNavCode );

// each screen has some of these functions to handle its processing
typedef struct {
	Wpr_DataTypes_WorkFcn *pWork;
	Wpr_DataTypes_DrawOrthoFcn *pDrawOrtho;
	Wpr_DataTypes_DrawFDrawFcn *pDrawFDraw;
	Wpr_DataTypes_DecisionsFcn *pExitDecisions;
} Wpr_DataTypes_ScreenFunctions_t;


// the different color strings that we use, formated for use by the ftext system
extern cwchar *WprDataTypes_pwszTitleTextColor;
extern cwchar *WprDataTypes_pwszBlueTextColor;
extern cwchar *WprDataTypes_pwszHeadingTextColor;
extern cwchar *WprDataTypes_pwszSubtitleTextColor;
extern cwchar *WprDataTypes_pwszWhiteTextColor;
extern cwchar *WprDataTypes_pwszMessageTextColor;
extern cwchar *WprDataTypes_pwszButtonTextColor;
extern cwchar *WprDataTypes_pwszGrayTextColor;
extern cwchar *WprDataTypes_pwszGrayTextColor2;
extern cwchar *WprDataTypes_pwszInstructionTextColor;
extern cwchar *WprDataTypes_pwszProfileLocationColor;
extern cwchar *WprDataTypes_pwszPressStartColor;
extern cwchar *WprDataTypes_pwszHiLightedBlink;
extern cwchar *WprDataTypes_pwszTeamAColor;
extern cwchar *WprDataTypes_pwszTeamBColor;
extern cwchar *WprDataTypes_pwszBlackTextColor;
extern cwchar *WprDataTypes_pwszSolidWhiteTextColor;


// data used to construct each of the screens, filled in from data contained in the csv file
extern Wpr_DataTypes_ScreenData_t *Wpr_DataTypes_paScreenData;
extern wchar Wpr_DataTypes_wszTempString[WPR_DATATYPES_TEMPSTRING_LENGTH];

extern Wpr_DataTypes_Alignment_e wpr_datatypes_GetAlignmentCode( char c );
extern Wpr_DataTypes_TextType_e wpr_datatypes_GetATextType( char c );
extern BOOL wpr_datatypes_InitScreenTextLayoutArray( FGameDataFileHandle_t hFile, 
													cchar *pszTableName, 
													Wpr_DataTypes_ScreenData_t &rScreen,
													CFStringTable &rStringTable );
extern BOOL wpr_datatypes_InitScreenButtonLayoutArray( FGameDataFileHandle_t hFile, 
													cchar *pszTableName, 
													Wpr_DataTypes_ScreenData_t &rScreen,
													CFStringTable &rStringTable,
													CFTexInst *pAButton,
													CFTexInst *pBButton,
													CFTexInst *pCButton,
													CFTexInst *pDButton );	
extern CFMeshInst *wpr_datatypes_FindMeshInst( cchar *pszMeshName, u32 nNumMeshes, CFMeshInst *paMeshInsts );
extern BOOL wpr_datatypes_InitScreenMeshLayoutArray( FGameDataFileHandle_t hFile, 
													cchar *pszTableName, 
													Wpr_DataTypes_ScreenData_t &rScreen,
													u32 nNumMeshes, CFMeshInst *paMeshInsts );
extern CFTexInst *wpr_datatypes_InitTextureArray( FGameDataFileHandle_t hFile, 
												 cchar *pszTableName,
												 u32 nNumExpectedTextures );
extern CFMeshInst *wpr_datatypes_InitMeshArray( FGameDataFileHandle_t hFile, 
												 cchar *pszTableName,
												 u32 &rnNumMeshes );
extern void wpr_datatypes_BreakDownTimeUnits( f32 fSecs, u32 &rnHours, u32 &rnMins, u32 &rnSecs );


#endif