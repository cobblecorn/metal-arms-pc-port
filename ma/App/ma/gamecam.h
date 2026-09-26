//////////////////////////////////////////////////////////////////////////////////////
// gamecam.h - 
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
// 01/24/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _GAMECAM_H_
#define _GAMECAM_H_ 1

#include "fang.h"
#include "fcamera.h" 
//#include "bot.h"
class CBot;

#define GAMECAM_DONT_MONITOR_CONTROLS		-1

typedef enum {
	GAME_CAM_PLAYER_1 = 0,	// player 1 is either the full screen (1 players), left screen side (2 players), or upper left corner (3 or 4 players) 
	GAME_CAM_PLAYER_2,		// player 2 is either the right screen side (2 players) or upper right corner (3 or 4 players)
	GAME_CAM_PLAYER_3,		// player 3 is always the lower left corner (3 or 4 players)
	GAME_CAM_PLAYER_4,		// player 4 is always the lower right corner (4 players)

	GAME_CAM_MAX_COUNT
} GameCamPlayer_e;

typedef enum {
	GAME_CAM_TYPE_NOT_IN_USE = -1,// USE INTERNALLY

	GAME_CAM_TYPE_ROBOT_3RD = 0,// the normal 3rd person robot camera
	GAME_CAM_TYPE_ROBOT_1ST,	// the 1st person robot camera
	GAME_CAM_TYPE_MANUAL,		// a manually driven camera
	GAME_CAM_TYPE_SIMPLE,
	GAME_CAM_TYPE_CUTSCENE,		// the cutscene camera
//TBD...	GAME_CAM_TYPE_RACING,		// the racing camera
//TBD...	GAME_CAM_TYPE_BIRDSEYE,		// a high camera, looking straight down on the action
//TBD...	GAME_CAM_TYPE_CONVERSATION,	// the camera used during conversations

	GAME_CAM_TYPE_COUNT
} GameCamType_e;

typedef enum {
	GAMECAM_DEBUG_MODES_NOT_DEBUGGING = 0,
	GAMECAM_DEBUG_MODES_FREE_CAM,			// regular debug camera with a locked player bot (free or lookat camera)
	GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM,	// the debug camera is locked and the player can move around (the render cam is the debug cam)
	GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM2,	// the debug camera is locked and the player can move around (the render cam is on the bot)
	GAMECAM_DEBUG_MODES_LOCKED_MOVING_LOOKAT_CAM,	// the debug camera is locked in the lookat mode while the player can move around (the render cam is the debug cam)

	GAMECAM_DEBUG_MODES_COUNT
} GameCam_DebugModes_e;

// Turn a player index into a game camera identifier
#define PLAYER_CAM(n)	((GameCamPlayer_e)(GAME_CAM_PLAYER_1 + n))

typedef struct gamecam_CamBotTransInfo_s {
    u32		uPlayer;
	CBot*	pBot1;
	CBot*	pBot2;
	f32		fUnitInterp;
} gamecam_CamBotTransInfo_t;

typedef struct gamecam_CamBotTransResult_s {
	f32			fFOV;
	CFMtx43A	mtxCamera;
	CFQuatA		qCamera;
} gamecam_CamBotTransResult_t;


// forward declare classes used:
class CFVec3Obj;
class CFQuatObj;
class CCamManualInfo;
class CCamBarterInfo;
class CCamSimpleInfo;
class CCamCutsceneInfo;
class CCamBarter;
class CCamBot;

// Call once at bootup, this function will create GAME_CAM_MAX_COUNT players cameras.
// This function will initialize the camera system, so the viewport system must
// be initialized before calling.
extern BOOL gamecam_InitSystem( void );
extern void gamecam_UninitSystem( void );

// Call after each level has been loaded.  This function will scan the loaded
// world and init needed level specific classes.
extern BOOL gamecam_InitLevelCameras( s32 nAllowDebuggingByPlayer=GAMECAM_DONT_MONITOR_CONTROLS,
									  u32 nControllerPortOfPlayer=0 );

extern void gamecam_ExitAnyDebugCam( void );

// Call at the end of each level.  This call will return the gamecam system back
// to how the state was before the last call to gamecam_InitLevelCameras().
extern void gamecam_PostLevelCleanup( void );

extern void SetPlayerToAllowDebugging( s32 nAllowDebuggingByPlayer=GAMECAM_DONT_MONITOR_CONTROLS,
									   u32 nControllerPortOfPlayer=0 );

// Call during a level to assign a certain bot to a player.  Different cameramen
// handle different camera types, thus the different functions.
extern void gamecam_SwitchPlayerTo3rdPersonCamera( GameCamPlayer_e nPlayer, CBot *pBot );
extern void gamecam_SwitchPlayerTo1stPersonCamera( GameCamPlayer_e nPlayer, CBot *pBot );
extern void gamecam_SwitchPlayerToManualCamera( GameCamPlayer_e nPlayer, CCamManualInfo *pCamInfo );
extern void gamecam_SwitchPlayerToSimpleCamera( GameCamPlayer_e nPlayer, CCamSimpleInfo *pCamInfo );
extern void gamecam_SwitchPlayerToCutsceneCamera( GameCamPlayer_e nPlayer, CCamCutsceneInfo *pCamInfo );

// Call to make a certain player ACTIVE.  This will change what camera
// gamecam_GetActiveCamera() returns.
extern BOOL gamecam_SetActiveCamera( GameCamPlayer_e nPlayer );
extern CFCamera *gamecam_GetActiveCamera( void );

extern CFCameraMan *gamecam_GetCameraManByIndex( GameCamPlayer_e nPlayer, GameCamType_e *pnCamTypeOut = NULL );

// Call to find out if system is in debug mode.
extern BOOL gamecam_IsInDebugMode( void );

// Returns the camera that is being debugged right now, NULL if not in debug mode.
extern CFCamera *gamecam_GetCameraBeingDebugged( GameCam_DebugModes_e *pnMode=NULL );

// Work() should be called once per frame, but should be called after the bots have been 
// put into their final location this frame.  Once the Work() call completes, all cameras 
// will be in their final location for this frame.
extern void gamecam_Work();

// This function will set the active viewport and initialize the camera stack with the 
// settings contained in the ACTIVE camera (unless in debug mode, then the debug mode
// camera will be used instead).  This call should occur before any 3D rendering
// takes place, but anytime after the current frame's Work() call.
extern BOOL gamecam_SetViewportAndInitCamStack();

// Draw() should be called as one of the last 3D draw functions in the gameloop.  Most of
// the time Draw() will simply return, but sometimes (in debug mode) certain information be
// drawn into the scene.
extern void gamecam_Draw();

// This function will calculate the camera position for each bot, then update the manual camera so that it interpolates between them.
extern void gamecam_DoCamBotTransition( const gamecam_CamBotTransInfo_t *pTransCamInfo, CCamManualInfo *pCamManInfo, gamecam_CamBotTransResult_t *pResult,
										gamecam_CamBotTransResult_t *pBot1Result, gamecam_CamBotTransResult_t *pBot2Result );

#endif

