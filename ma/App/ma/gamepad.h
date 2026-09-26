//////////////////////////////////////////////////////////////////////////////////////
// gamepad.h - Gamepad functionality.
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

#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_ 1

#include "fang.h"
#include "fpadio.h"
#include "fpad.h"

#define GAMEPAD_BUTTON_1ST_PRESS_MASK		( FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT | FPAD_LATCH_SPIKED )

#define GAMEPAD_MAX_PORT_COUNT		(FPADIO_MAX_DEVICES)
#define GAMEPAD_MAX_ACTIONS			32



typedef enum {
	GAMEPAD_MAP_MAIN1,	// strafe = left x analog, rotate = right x analog, regular button config
//	GAMEPAD_MAP_MAIN2,	// rotate = left x analog, strafe = right x analog, regular button config
//	GAMEPAD_MAP_MAIN3,	// main1 config, fire and jump button swapped
//	GAMEPAD_MAP_MAIN4,	// main2 config, fire and jump button swapped

	GAMEPAD_MAP_MENU,

	GAMEPAD_MAP_COUNT
} GamepadMap_e;


typedef enum {
	// GAMEPAD_MAP_MAIN*:
	GAMEPAD_MAIN_JUMP = 0,					
	GAMEPAD_MAIN_FORWARD_BACKWARD,			
	GAMEPAD_MAIN_STRAFE_LEFT_RIGHT,			
	GAMEPAD_MAIN_LOOK_UP_DOWN,				
	GAMEPAD_MAIN_ROTATE_LEFT_RIGHT,			
	GAMEPAD_MAIN_FIRE_PRIMARY,				
	GAMEPAD_MAIN_FIRE_SECONDARY,			
	GAMEPAD_MAIN_PAUSE,						
	GAMEPAD_MAIN_SELECT_PRIMARY,			
	GAMEPAD_MAIN_SELECT_SECONDARY,			
	GAMEPAD_MAIN_ACTION,					
	GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN,		
	GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT,	
	GAMEPAD_MAIN_MELEE,				
	GAMEPAD_MAIN_MELEE2,						
	GAMEPAD_MAIN_UP_EUK,					
	GAMEPAD_MAIN_COUNT,

	// GAMEPAD_MAP_MENUS:
	GAMEPAD_MENU_DPAD_X = 0,
	GAMEPAD_MENU_DPAD_Y,
	GAMEPAD_MENU_LEFT_ANALOG_X,
	GAMEPAD_MENU_LEFT_ANALOG_Y,
	GAMEPAD_MENU_LEFT_SHOULDER,
	GAMEPAD_MENU_RIGHT_ANALOG_X,
	GAMEPAD_MENU_RIGHT_ANALOG_Y,
	GAMEPAD_MENU_RIGHT_SHOULDER,
	GAMEPAD_MENU_START,
	GAMEPAD_MENU_ACCEPT,
	GAMEPAD_MENU_BACK,
	GAMEPAD_MENU_BUTTON_TOP,
	GAMEPAD_MENU_BUTTON_SIDE,
	GAMEPAD_MENU_BACK2,
	GAMEPAD_MENU_EXTRA_BUTTON,				// WHITE (XB), TRIGGER (GC)
	GAMEPAD_MENU_COUNT,
} GamePadAction_e;

typedef enum {
	GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X = 0,
	GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_Y,
	GAMEPAD_SLIDER_AXIS_LEFT_DIGITAL_X,
	GAMEPAD_SLIDER_AXIS_LEFT_DIGITAL_Y,
	GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_X,
	GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_Y,

	GAMEPAD_SLIDER_AXIS_COUNT
} GamePad_Slider_Axis_e;


extern u32 Gamepad_nPortOnlineMask;		// Set bits indicate ports that have controllers installed in them
extern FPad_Sample_t *Gamepad_aapSample[GAMEPAD_MAX_PORT_COUNT][GAMEPAD_MAX_ACTIONS];
extern u32 Gamepad_nDebugPortIndex;// what controller port should we look at for debug features

extern BOOL gamepad_InitSystem( void );
extern void gamepad_UninitSystem( void );

extern void gamepad_Sample( void );

extern GamepadMap_e gamepad_SetMapping( u32 nPortIndex, GamepadMap_e nMap );
extern GamepadMap_e gamepad_GetMapping( u32 nPortIndex );
extern void gamepad_ZeroControllerValues( void );

extern BOOL gamepad_AxisSlider( f32 &rfNewValue,		// if TRUE is returned, the new value is stored here
							    f32 fLastValue,			// the last value returned (changes will be made from here)
								f32 fMin=0.0f,			// the min acceptable value stored in rfNewValue
								f32 fMax=1.0f,			// the max acceptable value stored in rfNewValue
								f32 fStepSize=0.001f,	// the smallest absolute amount that value should change between calls (MUST BE POSITIVE, > 0)
								f32 fMaxChangePerSec=0.5f,// how fast can the value change in 1 second (MUST BE POSITIVE, > 0)
								GamePad_Slider_Axis_e nAxis=GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X,// what axis do we want to sample
								u32 nPort=0 );			// what controller port do we want to sample

extern void gamepad_SetDebugPortIndex( u32 nIndex );


/////////////////////////////////////////////////////////////////////////////////////////
//
// Gamepad cheat codes:
//
/////////////////////////////////////////////////////////////////////////////////////////

//
// Cheat codes
//
enum
{
	GAMEPAD_CHEAT_FIRST = 0,
#if FANG_ENABLE_DEV_FEATURES
	GAMEPAD_CHEAT_LEVEL_WIN,
#endif // FANG_ENABLE_DEV_FEATURES
	GAMEPAD_CHEAT_CODE_COUNT,
};


//
//
//
struct Gamepad_CheatState_t
{
	u8	nTimesEntered;			// This is incremented each time the cheat code is entered
	s8	nPosInCheatSequence;	// The current position in the cheat button sequence
};

// Structure that holds the cheat state for each controller
extern Gamepad_CheatState_t Gamepad_aCheatState[GAMEPAD_MAX_PORT_COUNT][GAMEPAD_CHEAT_CODE_COUNT];

// Call to reset cheat codes for all controllers
extern void Gamepad_ResetCheatCodes( void );


#endif

