//////////////////////////////////////////////////////////////////////////////////////
// floop.h - Fang gameloop module.
// 
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/21/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FLOOP_H_
#define _FLOOP_H_ 1

#include "fang.h"


typedef enum {
	FLOOP_TERMCODE_SHUTDOWNACK,		// Gameloop shutdown in reponse to a shutdown request
	FLOOP_TERMCODE_SUICIDE,			// Gameloop terminated itself without a shutdown request
	FLOOP_TERMCODE_INITFAILED,		// Gameloop init function failed
	FLOOP_TERMCODE_NULLLOOPFCN,		// Gameloop pointer was NULL

	FLOOP_TERMCODE_COUNT
} FLoopTermCode_t;

#define FLOOP_INITIAL_FRAME_SECS			(1.0f / 60.0f)
#define FLOOP_DEFAULT_BASE_FRAMES_PER_SEC	(1.0f / 15.0f)


typedef BOOL FLoopInit_t( void *pParameter );
typedef BOOL FLoopMain_t( BOOL bExitRequest, void *pParameter );
typedef void FLoopTerm_t( FLoopTermCode_t nTermCode, void *pParameter );


// These are all updated at the start of each frame:
extern u64 FLoop_nPreviousLoopTicks;		// The number of ticks from the previous call to the main loop to the current call
extern f32 FLoop_fPreviousLoopSecs;			// The number of seconds from the previous call to the main loop to the current call
extern f32 FLoop_fPreviousLoopOOSecs;		// 1.0f / FLoop_fPreviousLoopSecs (IMPORTANT: This will be 0 if FLoop_fPreviousLoopSecs is 0, which is when the game is paused)
extern u64 FLoop_nTotalLoopTicks;			// The number of ticks since the game loop has been started
extern u32 FLoop_nTotalLoopSecs;			// Low Precision version of time since the game loop has been started

extern u32 FLoop_nTicksPerSec;				// The number of ticks per second (frequency)
extern f32 FLoop_fTicksPerSec;				// The number of ticks per second (frequency)
extern f32 FLoop_fSecsPerTick;				// The number of seconds per tick (period)

// These values always represent real-time.
// They are never slowed down.
extern u64 FLoop_nRealPreviousLoopTicks;	// The number of ticks from the previous call to the main loop to the current call
extern f32 FLoop_fRealPreviousLoopSecs;		// The number of seconds from the previous call to the main loop to the current call
extern f32 FLoop_fRealPreviousLoopOOSecs;	// 1.0f / FLoop_fPreviousLoopSecs
extern u64 FLoop_nRealTotalLoopTicks;		// The number of ticks since the game loop has been started


// This will be TRUE when the game is paused.
// Use floop_PauseGame() to pause the game.
// However, FLoop_bGamePaused may be read
// directly for speed reasons.
//
// Game pausing is done by internally setting
// the time scale to 0.0f. Under this condition,
// FLoop_fPreviousLoopSecs will also be 0.
// However, it's important to note that
// FLoop_fPreviousLoopOOSecs will also be 0
// when the game is paused!
extern BOOL FLoop_bGamePaused;




extern BOOL floop_ModuleStartup( void );
extern void floop_ModuleShutdown( void );

extern BOOL floop_InstallGameloop( FLoopInit_t *pFcnInit, FLoopMain_t *pFcnMain, FLoopTerm_t *pFcnTerm, void *pParameter, f32 fTargetFramesPerSec, f32 fBaseFramesPerSec=FLOOP_DEFAULT_BASE_FRAMES_PER_SEC );
extern void floop_UninstallGameloop( void );

extern BOOL floop_IsGameloopRunning( void );

extern void floop_Reset( void );

extern BOOL floop_PauseGameloop( BOOL bPause );
extern BOOL floop_GetGameloopPauseState( void );
extern BOOL floop_SingleStepGameloop( void );

extern f32 floop_SetTargetFramesPerSec( f32 fTargetFramesPerSec );
extern f32 floop_GetTargetFramesPerSec( void );
extern f32 floop_GetTargetSecsPerFrame( void );

extern f32 floop_SetBaseFramesPerSec( f32 fBaseFramesPerSec );
extern f32 floop_GetBaseFramesPerSec( void );
extern f32 floop_GetBaseSecsPerFrame( void );

extern void floop_PauseGame( BOOL bPause );

extern f32 floop_SetTimeScale( f32 fTimeScale );
extern f32 floop_GetTimeScale( void );

extern BOOL floop_EnableFixedLoopTimeMode( BOOL bEnable );
extern BOOL floop_IsFixedLoopTimeModeEnabled( void );

extern BOOL floop_EnableGovernor( BOOL bEnable );
extern BOOL floop_IsGovernorEnabled( void );



#endif

