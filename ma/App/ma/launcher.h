//////////////////////////////////////////////////////////////////////////////////////
// Launcher.h - 
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
// 01/22/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _LAUNCHER_H_
#define _LAUNCHER_H_ 1

#include "fang.h"

#if FANG_ENABLE_DEV_FEATURES
	#define LAUNCHER_GO_DIRECTLY_TO_RETAIL_WRAPPERS		FALSE	// setting to TRUE will cause the wrappers to boot directly into the retail wrappers (the shipped product will need this to be TRUE)
#else
	#define LAUNCHER_GO_DIRECTLY_TO_RETAIL_WRAPPERS		TRUE	// setting to TRUE will cause the wrappers to boot directly into the retail wrappers (the shipped product will need this to be TRUE)
#endif
#define LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS				FALSE	// this must be an e3 version

// allows us to not include the picklevel code when we boot directly into the wrappers
#define LAUNCHER_INCLUDE_DEV_MENU						!LAUNCHER_GO_DIRECTLY_TO_RETAIL_WRAPPERS


// set 1 of the following TRUE to control what is offered on the E3 wrapper screen
// If neither are set, but LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS is TRUE, the single 
// player consumer demo is created.
#define LAUNCHER_E3_SINGLE_PLAYER_VERSION			FALSE	// the E3 singleplayer demo version
#define LAUNCHER_E3_MULTI_PLAYER_VERSION			FALSE	// the E3 multiplayer demo version


typedef enum {
	LAUNCHER_FROM_BOOTUP = 0,
	LAUNCHER_FROM_PICK_LEVEL,		// the development menu
	LAUNCHER_FROM_WRAPPERS,			// the wrapper system
	LAUNCHER_FROM_GAME,
	LAUNCHER_FROM_QUICK_SKIP,		// quick skip directly to a level
	LAUNCHER_FROM_E3,				// the E3 wrapper
	LAUNCHER_FROM_STARTUP_OPTIONS,	// the Gamecube startup options select screen

	LAUNCHER_FROM_COUNT
} LauncherFrom_e;

typedef enum {
	LAUNCHER_DECISION_LOAD_NEXT = 0,
	LAUNCHER_DECISION_RESTART,

	LAUNCHER_DECISION_COUNT
} LauncherDecision_e;

// If the game was launched by a demo launcher, this flag will be set
extern BOOL Launcher_bDemoLaunched;

extern BOOL launcher_InitSystem( void );
extern void launcher_UninitSystem( void );

extern BOOL launcher_EnterMenus( LauncherFrom_e nFrom );
extern BOOL launcher_StartGame( LauncherFrom_e nFrom, BOOL bLoadTestMode=FALSE );
extern void launcher_EndOfGameDecisions( LauncherDecision_e nDecision );
extern BOOL launcher_IsInLoadTestMode( void );
extern BOOL launcher_LaunchedFromWrappers( void );

extern void launcher_LoadFailure( void );
extern void launcher_DrawLoadingScreen( void );
extern void launcher_DrawLoadingText( void );

#endif