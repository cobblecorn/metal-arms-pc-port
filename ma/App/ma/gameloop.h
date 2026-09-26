//////////////////////////////////////////////////////////////////////////////////////
// gameloop.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/28/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _GAMELOOP_H_
#define _GAMELOOP_H_ 1

#include "fang.h"
#include "fvid.h"
#include "fcolor.h"

#if FANG_PLATFORM_WIN
	#define GAMELOOP_EXTERNAL_DEMO			FALSE
#else
	#define GAMELOOP_EXTERNAL_DEMO			TRUE// all non windows builds can be seen outside the company
#endif

extern BOOL Gameloop_bSwapEnabled;
extern BOOL Gameloop_bDrawEnabled;
extern BOOL Gameloop_bOverrideFrameTime;
extern BOOL Gameloop_bInstallAudio;

typedef enum {
	GAMELOOP_ANIM_PLAYBACK_30FPS = 0,
	GAMELOOP_ANIM_PLAYBACK_60FPS,
	GAMELOOP_ANIM_PLAYBACK_REALTIME,

	GAMELOOP_ANIM_PLAYBACK_COUNT
} Gameloop_AnimPlaybackRates_e;

typedef enum {
	GAMELOOP_PLATFORM_XB = 0,
	GAMELOOP_PLATFORM_GC,
	GAMELOOP_PLATFORM_PS2,			//CPS 4.7.03

	GAMELOOP_PLATFORM_COUNT
} Gameloop_Platform_e;

enum {
	GAMELOOP_MEMCARD_SLOT1 = 0x1,
	GAMELOOP_MEMCARD_SLOT2 = 0x2,
	GAMELOOP_MEMCARD_SLOT3 = 0x4,
	GAMELOOP_MEMCARD_SLOT4 = 0x8,
	GAMELOOP_MEMCARD_ALL   = 0xF,
	
	GAMELOOP_MEMCARD_NONE = 0
};

typedef enum {
	GAMELOOP_EXIT_DESTINATION_NONE			= 0,
	GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM,

	GAMELOOP_EXIT_DESTINATION_COUNT
} Gameloop_ExitDestination_e;

#define GAMELOOP_DEFAULT_TARGET_FPS				60.0f
#define GAMELOOP_DEFAULT_RES_X					640
#define GAMELOOP_DEFAULT_REX_Y					480
#define GAMELOOP_DEFAULT_SKIP_LEVEL_SELECT		FALSE
#define GAMELOOP_DEFAULT_ANIM_PLAYBACK			GAMELOOP_ANIM_PLAYBACK_REALTIME
#define GAMELOOP_DEFAULT_VIEW_BOUNDS			FALSE
#define GAMELOOP_DEFAULT_SHOW_FPS				FALSE
#define GAMELOOP_DEFAULT_DRAW_SCREEN_SAFE_AREA	FALSE
#define GAMELOOP_DEFAULT_PLATFORM				GAMELOOP_PLATFORM_XB
#define GAMELOOP_DEFAULT_MAX_SOUNDS				200
#define GAMELOOP_DEFAULT_LOG_FILE				NULL
#define GAMELOOP_DEFAULT_SCREENSHOT_DIR			NULL
#define GAMELOOP_DEFAULT_MEMCARD_SLOT			GAMELOOP_MEMCARD_ALL

typedef void Gameloop_ExitFunc_t( void );
typedef void Gameloop_MinimizeFunc_t( void );

typedef struct {
	f32 fTargetFPS;
	FVidWin_t VidWin;
	BOOL bSkipLevelSelect;
	Gameloop_AnimPlaybackRates_e nAnimPlaybackRate;
	BOOL bViewBounds;
	BOOL bShowFPS;
	BOOL bDrawScreenSafeArea;
	Gameloop_Platform_e nPlatform;
	u32 nMaxSoundMgrSounds;
	cchar *pszInputFilename;
	cchar *pszScreenShotDir;
	CFColorRGB BGColorRGB;
	Gameloop_ExitFunc_t *pExitFunc;
	Gameloop_MinimizeFunc_t *pMinFunc;
	u32 nMemCardUsageFlags;// bitmask of GAMELOOP_MEMCARD_ values
	cchar *pszMemCardDir;
	u32 *pauInputEmulationMap;    // DX8 only, ignored on other platforms.
	u8 *pszInputEmulationDevName; // DX8 only, ignored on other platforms.
	BOOL bInstallAudio;
	BOOL bLoadRegisteredMission; // Port: resolve quick-launch world through the mission registry.
	BOOL bLoadWorldOnly; // Port diagnostic: load a world resource, then exit before localized game setup.
	BOOL bPlayerDeath;
	BOOL bDebugAI;
	u32 nLegServoLevel;
	u32 nArmServoLevel;
	BOOL bShowEntityInfo;
	BOOL bGovernFrameRate;
	BOOL bDemoLaunched;		// The game was launched by a demo shell app
	u32 uTimeoutInterval;	// Timeout interval in milliseconds

} GameloopInitParm_t;

typedef BOOL GameloopFcn_t( void );


extern Gameloop_Platform_e Gameloop_nPlatform;
extern BOOL Gameloop_bDrawHUD;
extern BOOL Gameloop_bDrawDebugInfo;
extern Gameloop_ExitDestination_e Gameloop_ExitDestination;

extern BOOL gameloop_Start( GameloopInitParm_t *pParms );
extern void gameloop_End( void );

extern void gameloop_SetViewBounds( BOOL bEnable );
extern BOOL gameloop_GetViewBounds( void );

extern BOOL gameloop_GetPlayerDeath( void );
extern void gameloop_SetPlayerDeath( BOOL bEnable );
extern BOOL gameloop_GetDebugAI( void );
extern void gameloop_SetDebugAI( BOOL bEnable );

extern BOOL gameloop_GetShowEntityInfo( void );
extern void gameloop_SetShowEntityInfo( BOOL bEnable );

extern u32 gameloop_GetLegServoLevel( void );
extern u32 gameloop_GetArmServoLevel( void );
extern void gameloop_SetLegServoLevel( u32 nUpgradeLevel );
extern void gameloop_SetArmServoLevel( u32 nUpgradeLevel );

extern void gameloop_ShowFPS( BOOL bShow );
extern BOOL gameloop_GetShowFPS( void );

extern void gameloop_ShowScreenSafeArea( BOOL bShow );
extern BOOL gameloop_GetShowScreenSafeArea( void );

extern void gameloop_SetAnimPlaybackRate( Gameloop_AnimPlaybackRates_e nRate );
extern Gameloop_AnimPlaybackRates_e gameloop_GetAnimPlaybackRate( void );

extern void gameloop_SetLoopHandlers( GameloopFcn_t *pFcnWork, GameloopFcn_t *pFcnDraw, GameloopFcn_t *pFcnInit );
extern void gameloop_GetLoopHandlers( GameloopFcn_t **ppFcnWork, GameloopFcn_t **ppFcnDraw, GameloopFcn_t **ppFcnInit );

extern void gameloop_GetBackgroundColor( CFColorRGB &rRGB );

extern FINLINE void gameloop_SetSwapEnabled(BOOL bSwapEnabled) { Gameloop_bSwapEnabled = bSwapEnabled; }
extern FINLINE BOOL gameloop_GetSwapEnabled() { return(Gameloop_bSwapEnabled); }

extern FINLINE void gameloop_SetDrawEnabled(BOOL bDrawEnabled) { Gameloop_bDrawEnabled = bDrawEnabled; }
extern FINLINE BOOL gameloop_GetDrawEnabled() { return(Gameloop_bDrawEnabled); }

// Set this to FALSE if you want to override the frame time yourself.
extern FINLINE void gameloop_SetFrameTimeOverride(BOOL bOverride) { Gameloop_bOverrideFrameTime = bOverride; }

// call this to initialize the game systems which require that the localization data 
// (ie, the language the game will run in) already be specified.
extern BOOL gameloop_InitLocalizedGameSystems( void );

// call this function to schedule a gameloop exit at the top of the next loop
extern void gameloop_ScheduleExit( void );

// call this function to see if the platform should be reset.
// If it should be reset, if bResetImmediately == TRUE, then the OS is immediately rebooted.
// If bResetImmediately == FALSE, then the gameloop is scheduled for exit and the app shuts down normally then
// reboots.
// This function returns TRUE or FALSE, based on whether or not the platform needs to be reset.
// (Obviously, in the Immediate reset case, this function won't return, because the reset will already have been issued...)
extern BOOL gameloop_CheckForPlatformReset( BOOL bResetImmediately );
// This function will disable the ability to reset the platform.  This is
// usefull for the Gamecube when a fatal error has been reached.
extern void gameloop_DisablePlatformReset( void ); //
// call this function to tell the game what to do when it exits the game loop.
extern FINLINE void gameloop_SetExitDestination( Gameloop_ExitDestination_e nExitDestination ) { Gameloop_ExitDestination = nExitDestination; }

// call this function to disable parts of the overlaid graphics, usually for taking better screenshots
extern void gameloop_EnableOverlayDrawElements( BOOL bEnableHUD, BOOL bEnableDebugInfo );

// call to get the level name to skip to
extern cchar *gameloop_GetSkipLevelName( void );
extern BOOL gameloop_GetSkipLevelUseMissionData( void );

extern void gameloop_ResetIdleTimer();
extern void gameloop_EnableIdleTimer( BOOL bEnable );

#endif
