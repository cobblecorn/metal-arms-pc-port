//////////////////////////////////////////////////////////////////////////////////////
// gamepad.cpp - Gamepad functionality.
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
// 02/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif
#include "gamepad.h"
#include "fpadio.h"
#include "fpad.h"
#include "gameloop.h"
#include "floop.h"
#include "fperf.h"
#include "launcher.h"
#include "game.h"
#if defined(MA_PC_INPUT)
extern BOOL pausescreen_IsActive( void );	// PauseScreen.cpp (its header clashes with names here)
#endif


u32 Gamepad_nPortOnlineMask;
FPad_Sample_t *Gamepad_aapSample[GAMEPAD_MAX_PORT_COUNT][GAMEPAD_MAX_ACTIONS];
u32 Gamepad_nDebugPortIndex;// what controller port should we look at for debug features


typedef enum {
	_PLATFORM_XB,
	_PLATFORM_GC,
	_PLATFORM_PS2,			//CPS 4.7.03

	_PLATFORM_COUNT
} _Platform_e;


typedef struct {
	u32 nMapCount;
	const FPadio_InputID_e *aMap[_PLATFORM_COUNT];
} _MapInfo_t;


static BOOL _bSystemInitialized = FALSE;
static GamepadMap_e _anCurrentMap[GAMEPAD_MAX_PORT_COUNT];


static const FPadio_InputID_e _aaInputMap_Main1[_PLATFORM_COUNT][GAMEPAD_MAIN_COUNT] = {
	// Xbox:
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT,// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_XB_ABUTTON_BLACK,		// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_XB_ABUTTON_WHITE,		// GAMEPAD_MAIN_UP_EUK

	// GameCube:
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z,	// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_UP_EUK

	//CPS 4.7.03 -->
	// PlayStation2:
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_HUD_SWITCH
	FPADIO_INPUT_PS2_RIGHT2,			// GAMEPAD_MAIN_TURBO
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_UP_EUK
	//<-- CPS 4.7.03
};
/*
static const FPadio_InputID_e _aaInputMap_Main2[_PLATFORM_COUNT][GAMEPAD_MAIN_COUNT] = {
	// Xbox:
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT,// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_XB_ABUTTON_BLACK,		// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_XB_ABUTTON_WHITE,		// GAMEPAD_MAIN_UP_EUK

	// GameCube:
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z,	// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_UP_EUK

	//CPS 4.7.03 -->
	// PlayStation2:
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_HUD_SWITCH
	FPADIO_INPUT_PS2_RIGHT2,			// GAMEPAD_MAIN_TURBO
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_UP_EUK
	//<-- CPS 4.7.03
};

static const FPadio_InputID_e _aaInputMap_Main3[_PLATFORM_COUNT][GAMEPAD_MAIN_COUNT] = {
	// Xbox:
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT,// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_XB_ABUTTON_BLACK,		// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_XB_ABUTTON_WHITE,		// GAMEPAD_MAIN_UP_EUK

	// GameCube:
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z,	// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_UP_EUK

	//CPS 4.7.03 -->
	// PlayStation2:
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_HUD_SWITCH
	FPADIO_INPUT_PS2_RIGHT2,			// GAMEPAD_MAIN_TURBO
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_UP_EUK
	//<-- CPS 4.7.03
};


static const FPadio_InputID_e _aaInputMap_Main4[_PLATFORM_COUNT][GAMEPAD_MAIN_COUNT] = {
	// Xbox:
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT,// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_XB_ABUTTON_BLACK,		// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_XB_ABUTTON_WHITE,		// GAMEPAD_MAIN_UP_EUK

	// GameCube:
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z,	// GAMEPAD_MAIN_MELEE
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_MELEE2
	FPADIO_INPUT_NONE,					// GAMEPAD_MAIN_UP_EUK

	//CPS 04.07.03 -->
	// PlayStation2:
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MAIN_JUMP
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_FORWARD_BACKWARD
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_STRAFE_LEFT_RIGHT
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MAIN_LOOK_UP_DOWN
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MAIN_ROTATE_LEFT_RIGHT
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MAIN_FIRE_PRIMARY
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MAIN_FIRE_SECONDARY
	FPADIO_INPUT_START,					// GAMEPAD_MAIN_PAUSE
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MAIN_SELECT_PRIMARY
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MAIN_SELECT_SECONDARY
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MAIN_ACTION
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_HUD_SWITCH
	FPADIO_INPUT_PS2_RIGHT2,			// GAMEPAD_MAIN_TURBO
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MAIN_UP_EUK
	//<-- CPS 04.07.03
};
*/

static const FPadio_InputID_e _aaInputMap_Menu[_PLATFORM_COUNT][GAMEPAD_MENU_COUNT] = {
	// Xbox:
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MENU_DPAD_X
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MENU_DPAD_Y
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MENU_LEFT_ANALOG_X
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MENU_LEFT_ANALOG_Y
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MENU_LEFT_SHOULDER
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MENU_RIGHT_ANALOG_X
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MENU_RIGHT_ANALOG_Y
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MENU_RIGHT_SHOULDER
	FPADIO_INPUT_START,					// GAMEPAD_MENU_START
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MENU_ACCEPT
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MENU_BACK
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MENU_BUTTON_TOP
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MENU_BUTTON_SIDE
	FPADIO_INPUT_XB_DBUTTON_BACK,		// GAMEPAD_MENU_BACK2
	FPADIO_INPUT_XB_ABUTTON_WHITE,		// GAMEPAD_MENU_EXTRA_BUTTON

	// GameCube:
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MENU_DPAD_X
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MENU_DPAD_Y
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MENU_LEFT_ANALOG_X
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MENU_LEFT_ANALOG_Y
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MENU_LEFT_SHOULDER
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MENU_RIGHT_ANALOG_X
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MENU_RIGHT_ANALOG_Y
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MENU_RIGHT_SHOULDER
	FPADIO_INPUT_START,					// GAMEPAD_MENU_START
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MENU_ACCEPT
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MENU_BACK
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MENU_BUTTON_TOP
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MENU_BUTTON_SIDE
	FPADIO_INPUT_NONE,					// GAMEPAD_MENU_BACK2
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z,	// GAMEPAD_MENU_EXTRA_BUTTON

	//CPS 4.7.03 -->
	// PlayStation2:
	FPADIO_INPUT_DPAD_X,				// GAMEPAD_MENU_DPAD_X
	FPADIO_INPUT_DPAD_Y,				// GAMEPAD_MENU_DPAD_Y
	FPADIO_INPUT_STICK_LEFT_X,			// GAMEPAD_MENU_LEFT_ANALOG_X
	FPADIO_INPUT_STICK_LEFT_Y,			// GAMEPAD_MENU_LEFT_ANALOG_Y
	FPADIO_INPUT_TRIGGER_LEFT,			// GAMEPAD_MENU_LEFT_SHOULDER
	FPADIO_INPUT_STICK_RIGHT_X,			// GAMEPAD_MENU_RIGHT_ANALOG_X
	FPADIO_INPUT_STICK_RIGHT_Y,			// GAMEPAD_MENU_RIGHT_ANALOG_Y
	FPADIO_INPUT_TRIGGER_RIGHT,			// GAMEPAD_MENU_RIGHT_SHOULDER
	FPADIO_INPUT_START,					// GAMEPAD_MENU_START
	FPADIO_INPUT_CROSS_BOTTOM,			// GAMEPAD_MENU_ACCEPT
	FPADIO_INPUT_CROSS_TOP,				// GAMEPAD_MENU_BACK
	FPADIO_INPUT_CROSS_RIGHT,			// GAMEPAD_MENU_BUTTON_TOP
	FPADIO_INPUT_CROSS_LEFT,			// GAMEPAD_MENU_BUTTON_SIDE
	FPADIO_INPUT_PS2_SELECT,			// GAMEPAD_MENU_BACK2
	FPADIO_INPUT_PS2_LEFT2,				// GAMEPAD_MENU_EXTRA_BUTTON
	//<-- CPS 4.7.03
};


static const _MapInfo_t _aMapInfo[GAMEPAD_MAP_COUNT] = {
	GAMEPAD_MAIN_COUNT, &_aaInputMap_Main1[_PLATFORM_XB][0], &_aaInputMap_Main1[_PLATFORM_GC][0], /*CPS 4.7.03*/&_aaInputMap_Main1[_PLATFORM_PS2][0],
//	GAMEPAD_MAIN_COUNT, &_aaInputMap_Main2[_PLATFORM_XB][0], &_aaInputMap_Main2[_PLATFORM_GC][0], /*CPS 4.7.03*/&_aaInputMap_Main2[_PLATFORM_PS2][0],
//	GAMEPAD_MAIN_COUNT, &_aaInputMap_Main3[_PLATFORM_XB][0], &_aaInputMap_Main3[_PLATFORM_GC][0], /*CPS 4.7.03*/&_aaInputMap_Main3[_PLATFORM_PS2][0],
//	GAMEPAD_MAIN_COUNT, &_aaInputMap_Main4[_PLATFORM_XB][0], &_aaInputMap_Main4[_PLATFORM_GC][0], /*CPS 4.7.03*/&_aaInputMap_Main4[_PLATFORM_PS2][0],

	GAMEPAD_MENU_COUNT, &_aaInputMap_Menu[_PLATFORM_XB][0], &_aaInputMap_Menu[_PLATFORM_GC][0], /*CPS 4.7.03*/&_aaInputMap_Menu[_PLATFORM_PS2][0]
};

void _HandleCheatCodes( void );


BOOL gamepad_InitSystem( void ) {
	u32 i;

	FASSERT_MSG( !_bSystemInitialized, "gamepad_InitSystem(): Cannot call this function more than once.\n" );

	_bSystemInitialized = TRUE;

	Gamepad_nPortOnlineMask = 0;

	Gamepad_ResetCheatCodes();

	for( i=0; i<GAMEPAD_MAX_PORT_COUNT; i++ ) {
		gamepad_SetMapping( i, GAMEPAD_MAP_MAIN1 );
	}

#if( FANG_PLATFORM_WIN )
	Gamepad_nDebugPortIndex = 0;
#elif( FANG_PLATFORM_GC )
	Gamepad_nDebugPortIndex = 1;
#elif( FANG_PLATFORM_XB )
	Gamepad_nDebugPortIndex = 1;
#elif( FANG_PLATFORM_PS2 )			//CPS 4.7.03
	Gamepad_nDebugPortIndex = 1;	//CPS 4.7.03
#endif	

	return TRUE;
}

void gamepad_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		Gamepad_nPortOnlineMask = 0;
		_bSystemInitialized = FALSE;
	}
}


extern BOOL CBot_bUseOldBotCollision;
void gamepad_Sample( void ) 
{
	FASSERT( _bSystemInitialized );

	Gamepad_nPortOnlineMask = fpad_UpdateSamples();
#if defined(MA_PC_INPUT)
	// The pause menu is a menu for the keyboard and mouse even while the gameplay map is set: it switches
	// to the menu map only for the moment it reads its buttons. So the mouse is free for its pointer and
	// Escape is Back there. Only the pause menu, not any paused game loop: holding Q/R for a weapon list
	// also pauses the loop, and treating that as a menu remapped the held key, which closed the list,
	// which unpaused, which reopened it (the list flickered open and shut).
	pcinput_BeginFrame( _anCurrentMap[pcinput_KeyboardPort()] == GAMEPAD_MAP_MAIN1 && !pausescreen_IsActive() );
	game_PcPromptWork();
#endif

	_HandleCheatCodes();

#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
	if ( Launcher_bDemoLaunched ) {
		// check the controls to see if there has been some movement
		u32 i, j, nActionCount;
		for( i=0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) {
			if( (Gamepad_nPortOnlineMask & (1<<i)) ) {
				// controller is plugged into this port
				if( _anCurrentMap[i] == GAMEPAD_MAP_MENU ) {
					nActionCount = GAMEPAD_MENU_COUNT;
				} else {
					nActionCount = GAMEPAD_MAIN_COUNT;
				}
				// see if any of these actions are on
				for( j=0; j < nActionCount; j++ ) {
					if( Gamepad_aapSample[i][j]->uLatches & FPAD_LATCH_ON ) {

						gameloop_ResetIdleTimer();
						break;
					}
				}
				if( j < nActionCount ) {
					// we reset the idle timer, no need to continue to the other controllers
					break;
				}

			}
		}
	}
#endif

#if !FANG_PRODUCTION_BUILD
	if ( Gamepad_aapSample[Gamepad_nDebugPortIndex][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState
		&& (Gamepad_aapSample[Gamepad_nDebugPortIndex][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & FPAD_LATCH_CHANGED) )
	{
		FPerf_nDisplayPerfType = (FPerf_Display_Type_e)(FPerf_nDisplayPerfType + 1);
		if ( FPerf_nDisplayPerfType == FPERF_TYPE_ALL )
		{
            FPerf_nDisplayPerfType = FPERF_TYPE_NONE;
		}
	}

	#if 0
		if ( Gamepad_aapSample[Gamepad_nDebugPortIndex][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState
			&& (Gamepad_aapSample[Gamepad_nDebugPortIndex][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->uLatches & FPAD_LATCH_CHANGED) )
		{
			static BOOL bPaused = FALSE;
			bPaused = !bPaused;
			floop_PauseGame( bPaused );
//			CBot_bUseOldBotCollision = !CBot_bUseOldBotCollision;
		}
	#endif
#endif
}

// returns the previous mapping
GamepadMap_e gamepad_SetMapping( u32 nPortIndex, GamepadMap_e nMap ) {
	FASSERT( _bSystemInitialized );
	FASSERT( nPortIndex < GAMEPAD_MAX_PORT_COUNT );
	FASSERT( nMap>=0 && nMap<GAMEPAD_MAP_COUNT );

	// save the current map
	GamepadMap_e nOldMapping = _anCurrentMap[nPortIndex];
	// set the new map
	_anCurrentMap[nPortIndex] = nMap;

	#if FANG_PLATFORM_WIN
		#if defined(MA_PC_INPUT)
			const _Platform_e nMapPlatform = ( nMap == GAMEPAD_MAP_MENU ) ? _PLATFORM_XB : (_Platform_e)Gameloop_nPlatform;
			fpad_Map( nPortIndex, _aMapInfo[nMap].nMapCount, _aMapInfo[nMap].aMap[nMapPlatform], Gamepad_aapSample[nPortIndex] );
		#else
			fpad_Map( nPortIndex, _aMapInfo[nMap].nMapCount, _aMapInfo[nMap].aMap[Gameloop_nPlatform], Gamepad_aapSample[nPortIndex] );
		#endif
	#elif FANG_PLATFORM_XB
		fpad_Map( nPortIndex, _aMapInfo[nMap].nMapCount, _aMapInfo[nMap].aMap[_PLATFORM_XB], Gamepad_aapSample[nPortIndex] );
	#elif FANG_PLATFORM_GC
		fpad_Map( nPortIndex, _aMapInfo[nMap].nMapCount, _aMapInfo[nMap].aMap[_PLATFORM_GC], Gamepad_aapSample[nPortIndex] );
	#elif FANG_PLATFORM_PS2			//CPS 4.7.03
		fpad_Map( nPortIndex, _aMapInfo[nMap].nMapCount, _aMapInfo[nMap].aMap[_PLATFORM_PS2], Gamepad_aapSample[nPortIndex] );		//CPS 4.7.03
	#else
		#error <Undefined Platform>
	#endif

	return nOldMapping;
}

GamepadMap_e gamepad_GetMapping( u32 nPortIndex ) {
	FASSERT( _bSystemInitialized );
	FASSERT( nPortIndex < GAMEPAD_MAX_PORT_COUNT );

	return _anCurrentMap[nPortIndex];
}

void gamepad_ZeroControllerValues( void ) {
	FASSERT( _bSystemInitialized );

	fpad_ZeroAllControllerSamples();
}

BOOL gamepad_AxisSlider( f32 &rfNewValue,			// if TRUE is returned, the new value is stored here
						 f32 fLastValue,			// the last value returned (changes will be made from here)
						 f32 fMin/*=0.0f*/,			// the min acceptable value stored in rfNewValue
						 f32 fMax/*=1.0f*/,			// the max acceptable value stored in rfNewValue
						 f32 fStepSize/*=0.001f*/,	// the smallest absolute amount that value should change between calls (MUST BE POSITIVE, > 0)
						 f32 fMaxChangePerSec/*=0.5f*/,// how fast can the value change (MUST BE POSITIVE, > 0)
						 GamePad_Slider_Axis_e nAxis/*=GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X*/,// what axis do we want to sample
						 u32 nPort/*=0*/ ) {		// what controller port do we want to sample

	FASSERT( nPort < GAMEPAD_MAX_PORT_COUNT );
	FASSERT( nAxis < GAMEPAD_SLIDER_AXIS_COUNT );
	FASSERT( fStepSize > 0.0f );
	FASSERT( fMaxChangePerSec > 0.0f );

	if( (Gamepad_nPortOnlineMask & (1<<nPort)) == 0 ) {
		// a controller is not plugged into the requested port
		return FALSE;
	}

	// map the axis to a pad sample
	FPad_Sample_t *pSample;
	switch( gamepad_GetMapping( nPort ) ) {

	case GAMEPAD_MAP_MAIN1:
//	case GAMEPAD_MAP_MAIN2:
		switch( nAxis ) {
		case GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT];
			break;
		case GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_Y:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MAIN_FORWARD_BACKWARD];
			break;
		case GAMEPAD_SLIDER_AXIS_LEFT_DIGITAL_X:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT];
			break;
		case GAMEPAD_SLIDER_AXIS_LEFT_DIGITAL_Y:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN];
			break;
		case GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_X:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MAIN_ROTATE_LEFT_RIGHT];
			break;
		case GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_Y:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MAIN_LOOK_UP_DOWN];
			break;
		}
		break;

	case GAMEPAD_MAP_MENU:
		switch( nAxis ) {
		case GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MENU_LEFT_ANALOG_X];
			break;
		case GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_Y:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MENU_LEFT_ANALOG_Y];
			break;
		case GAMEPAD_SLIDER_AXIS_LEFT_DIGITAL_X:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MENU_DPAD_X];
			break;
		case GAMEPAD_SLIDER_AXIS_LEFT_DIGITAL_Y:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MENU_DPAD_Y];
			break;
		case GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_X:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MENU_RIGHT_ANALOG_X];
			break;
		case GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_Y:
			pSample = Gamepad_aapSample[nPort][GAMEPAD_MENU_RIGHT_ANALOG_Y];
			break;
		}
		break;

	default:
		FASSERT_NOW;
		return FALSE;
	}
	
	if( (pSample->uLatches & FPAD_LATCH_ON) == 0 ) {
		// the controller didn't change
		return FALSE;
	}

	// map the current state to a delta change
	f32 fDelta;
	fDelta = pSample->fCurrentState * fMaxChangePerSec * FLoop_fRealPreviousLoopSecs;
	if( fDelta == 0.0f ) {
		return FALSE;
	}    	
	if( fDelta > 0.0f && 
		fDelta < fStepSize ) {
		fDelta = fStepSize;
	}
	if( fDelta < 0.0f && 
		fDelta > -fStepSize ) {
		fDelta = -fStepSize;
	}

	// fill in the new value and clamp it to the acceptable range
	rfNewValue = fLastValue + fDelta;
	FMATH_CLAMP( rfNewValue, fMin, fMax );

	return TRUE;
}

void gamepad_SetDebugPortIndex( u32 nIndex ) {
	FMATH_CLAMP( nIndex, 0, (GAMEPAD_MAX_PORT_COUNT-1) );

#if( FANG_PLATFORM_WIN )
	Gamepad_nDebugPortIndex = 0;
#else
	Gamepad_nDebugPortIndex = nIndex;
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CHEAT CODES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define _MAX_KEYS_IN_CHEAT_SEQUENCE		8

// Function called when a cheat code is entered
typedef void _CheatFunction_t( u32 nControllerIdx, u32 nTotalTimesEntered );

//
//
struct _CheatCode_t
{
	GamePadAction_e		aCheatKeySequence[_MAX_KEYS_IN_CHEAT_SEQUENCE];
	BOOL				bAllowMultipleEntry;
	_CheatFunction_t	*pCheatFunction;
};

// Forward declarations for cheat code functions
static void _LevelWinCheat( u32 nControllerIdx, u32 nTotalTimesEntered );

// Here we keep track of the cheats for each port
Gamepad_CheatState_t Gamepad_aCheatState[GAMEPAD_MAX_PORT_COUNT][GAMEPAD_CHEAT_CODE_COUNT];

// Game Cheat codes (Place final game cheats outside the #if FANG_ENABLE_DEV_FEATURES block)
_CheatCode_t _aCheatCode[GAMEPAD_CHEAT_CODE_COUNT] = 
{
	#if FANG_ENABLE_DEV_FEATURES
		// GAMEPAD_CHEAT_LEVEL_WIN
		GAMEPAD_MAIN_FIRE_SECONDARY,
		GAMEPAD_MAIN_FIRE_PRIMARY,
		GAMEPAD_MAIN_ACTION,
		GAMEPAD_MAIN_JUMP,
		GAMEPAD_MAIN_SELECT_SECONDARY,
		GAMEPAD_MAIN_SELECT_PRIMARY,
		GAMEPAD_MAIN_COUNT, // MARKS THE END OF THE SEQUENCE
		GAMEPAD_MAIN_COUNT,
		FALSE,
		_LevelWinCheat,
	#endif // FANG_ENABLE_DEV_FEATURES
};


//
//
//
void Gamepad_ResetCheatCodes( void )
{
	fang_MemZero( Gamepad_aCheatState, sizeof( Gamepad_CheatState_t ) * GAMEPAD_MAX_PORT_COUNT * GAMEPAD_CHEAT_CODE_COUNT );
}


//
//
//
void _HandleCheatCodes( void )
{
	u32 i, j, k;

	// Check each port for cheat codes
	for( i = 0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) 
	{
		// Is a controller plugged in?
		if( !(Gamepad_nPortOnlineMask & (1<<i)) ) 
		{
			continue;
		}

		if( _anCurrentMap[i] == GAMEPAD_MAP_MENU ) 
		{
			// No cheats supported in the menus
			continue;
		}

		// Progress through all of the action buttons
		for( j = 0; j < GAMEPAD_MAIN_COUNT; j++ ) 
		{
			// Do we have this button pressed?
			if( Gamepad_aapSample[i][j]->uLatches & FPAD_LATCH_ON ) 
			{
				// Did we just press it?
				if( Gamepad_aapSample[i][j]->uLatches & FPAD_LATCH_CHANGED )
				{
					// Check each of the cheat codes to see if this is a key in their sequence
					for( k = 0; k < GAMEPAD_CHEAT_CODE_COUNT; k++ )
					{
						if ( Gamepad_aCheatState[i][k].nTimesEntered && !_aCheatCode[k].bAllowMultipleEntry )
						{
							// This cheat code is already enabled
							continue;
						}

						// Is it the next in this sequence?
						if( j == _aCheatCode[k].aCheatKeySequence[Gamepad_aCheatState[i][k].nPosInCheatSequence] )
						{
							// This key was the next in the sequence, so we look for the next key from now on
							Gamepad_aCheatState[i][k].nPosInCheatSequence++;
							if( Gamepad_aCheatState[i][k].nPosInCheatSequence == _MAX_KEYS_IN_CHEAT_SEQUENCE 
								|| _aCheatCode[k].aCheatKeySequence[Gamepad_aCheatState[i][k].nPosInCheatSequence] == GAMEPAD_MAIN_COUNT )
							{
								Gamepad_aCheatState[i][k].nTimesEntered++;

								if ( _aCheatCode[k].pCheatFunction )
								{
									_aCheatCode[k].pCheatFunction( i, Gamepad_aCheatState[i][k].nTimesEntered );
								}
							}
						}
						else
						{
							// This key is not next in the sequence, so we reset the position
							Gamepad_aCheatState[i][k].nPosInCheatSequence = 0;
						}
					}
				}
			}
		}
	}
}

//
//
//
void _LevelWinCheat( u32 nControllerIdx, u32 nTotalTimesEntered )
{
	game_GotoLevel( "next" );
}

