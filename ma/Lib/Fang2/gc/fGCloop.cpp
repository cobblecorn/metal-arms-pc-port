//////////////////////////////////////////////////////////////////////////////////////
// fGCloop.cpp - Fang gameloop module (GameCube version).
//
// Author: John Lafleur
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
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "floop.h"
#include "ftimer.h"
#include "fpadio.h"
#include "faudio.h"
#include "fforce.h"
#include "ffile.h"

#define _USE_BOXFILTER		FALSE

#if _USE_BOXFILTER
	#include "fboxfilter.h"
#endif


#include "fperf.h"


// These are all updated at the start of each frame:
u64 FLoop_nPreviousLoopTicks;	// The number of ticks from the previous call to the main loop to the current call
f32 FLoop_fPreviousLoopSecs;	// The number of seconds from the previous call to the main loop to the current call
f32 FLoop_fPreviousLoopOOSecs;	// 1.0f / FLoop_fPreviousLoopSecs
u64 FLoop_nTotalLoopTicks;		// The number of ticks since the game loop has been started
u32	FLoop_nTotalLoopSecs;		// Low Precision version of the time since the game loop has been started
u32 FLoop_nTicksPerSec;			// The number of ticks per second (frequency)
f32 FLoop_fTicksPerSec;			// The number of ticks per second (frequency)
f32 FLoop_fSecsPerTick;			// The number of seconds per tick (period)

// These values always represent real-time.
// They are never slowed down.
u64 FLoop_nRealPreviousLoopTicks;	// The number of ticks from the previous call to the main loop to the current call
f32 FLoop_fRealPreviousLoopSecs;	// The number of seconds from the previous call to the main loop to the current call
f32 FLoop_fRealPreviousLoopOOSecs;	// 1.0f / FLoop_fPreviousLoopSecs
u64 FLoop_nRealTotalLoopTicks;		// The number of ticks since the game loop has been started


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
BOOL FLoop_bGamePaused;


#define _MAX_ALLOWED_DELTA_SECS_FOR_DEBUG_BUILDS		(1.0f / 5.0f)



static BOOL _bModuleInitialized;

static f32 _fTimeScale;
static f32 _fOOTimeScale;		// This will be 0 when _fTimeScale is 0

static volatile f32 _fTargetFramesPerSec;
static volatile f32 _fTargetSecsPerFrame;
static volatile u64 _nTargetTicksPerFrame;
static volatile f32 _fTargetTicksPerFrame;
static volatile f32 _fTargetFramesPerTick;

static volatile f32 _fScaledTargetFramesPerSec;
static volatile f32 _fScaledTargetSecsPerFrame;
static volatile u64 _nScaledTargetTicksPerFrame;
static volatile f32 _fScaledTargetTicksPerFrame;

static volatile f32 _fBaseFramesPerSec;
static volatile f32 _fBaseSecsPerFrame;
static volatile u64 _nBaseTicksPerFrame;
static volatile f32 _fBaseTicksPerFrame;
static volatile f32 _fBaseFramesPerTick;

static volatile f32 _fScaledBaseFramesPerSec;
static volatile f32 _fScaledBaseSecsPerFrame;
static volatile u64 _nScaledBaseTicksPerFrame;
static volatile f32 _fScaledBaseTicksPerFrame;

static volatile BOOL _bFixedLoopTimeMode;
static volatile BOOL _bGovernFrameRate;
static volatile BOOL _bResetMode;


static void _UpdateScaledTargetValues( void );
static void _UpdateScaledBaseValues( void );



//
//
//
BOOL floop_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	#if FANG_PLATFORM_WIN
		_pGameloopThread = NULL;
	#endif

	FLoop_fRealPreviousLoopSecs = FLOOP_INITIAL_FRAME_SECS;
	FLoop_fRealPreviousLoopOOSecs = 1.0f / FLOOP_INITIAL_FRAME_SECS;

	_fTimeScale = 1.0f;
	_fOOTimeScale = 1.0f / _fTimeScale;
	FLoop_bGamePaused = FALSE;

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void floop_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	floop_UninstallGameloop();

	_bModuleInitialized = FALSE;
}


//
//
//
f32 floop_SetTargetFramesPerSec( f32 fTargetFramesPerSec ) 
{
	f32 fPrevVal;

	FASSERT( _bModuleInitialized );
	FASSERT( fTargetFramesPerSec > 0.0f );

	fPrevVal = _fTargetFramesPerSec;
	_fTargetFramesPerSec = fTargetFramesPerSec;
	_fTargetSecsPerFrame = 1.0f / fTargetFramesPerSec;

	_fTargetTicksPerFrame = _fTargetSecsPerFrame * FLoop_fTicksPerSec;
	_nTargetTicksPerFrame = (u64)_fTargetTicksPerFrame;
	_fTargetFramesPerTick = _fTargetFramesPerSec * FLoop_fSecsPerTick;

	_UpdateScaledTargetValues();

	return fPrevVal;
}


//
//
//
f32 floop_GetTargetFramesPerSec( void ) 
{
	FASSERT( _bModuleInitialized );
	return _fTargetFramesPerSec;
}


//
//
//
f32 floop_GetTargetSecsPerFrame( void ) 
{
	FASSERT( _bModuleInitialized );
	return _fTargetSecsPerFrame;
}


//
// Set fBaseFramesPerSec to 0.0f to disable base frame rate clamping.
//
f32 floop_SetBaseFramesPerSec( f32 fBaseFramesPerSec ) 
{
	f32 fPrevVal;

	FASSERT( _bModuleInitialized );
	FASSERT( fBaseFramesPerSec >= 0.0f );

	fPrevVal = _fBaseFramesPerSec;

	if ( fBaseFramesPerSec > 0.0f ) 
	{
		_fBaseFramesPerSec = fBaseFramesPerSec;
		_fBaseSecsPerFrame = 1.0f / fBaseFramesPerSec;

		_fBaseTicksPerFrame = _fBaseSecsPerFrame * FLoop_fTicksPerSec;
		_nBaseTicksPerFrame = (u64)_fBaseTicksPerFrame;
		_fBaseFramesPerTick = _fBaseFramesPerSec * FLoop_fSecsPerTick;
	} 
	else 
	{
		_fBaseFramesPerSec = 0.0f;
		_fBaseSecsPerFrame = 0.0f;

		_fBaseTicksPerFrame = 0.0f;
		_nBaseTicksPerFrame = 0;
		_fBaseFramesPerTick = 0;
	}

	_UpdateScaledBaseValues();

	return fPrevVal;
}


//
//
//
f32 floop_GetBaseFramesPerSec( void ) 
{
	FASSERT( _bModuleInitialized );
	return _fBaseFramesPerSec;
}


//
//
//
f32 floop_GetBaseSecsPerFrame( void ) 
{
	FASSERT( _bModuleInitialized );
	return _fBaseSecsPerFrame;
}


//
//
//
void floop_PauseGame( BOOL bPause ) 
{
	FASSERT( _bModuleInitialized );

	bPause = !!bPause;

	if ( bPause != FLoop_bGamePaused ) 
	{
		FLoop_bGamePaused = bPause;
		_UpdateScaledTargetValues();
		_UpdateScaledBaseValues();

		// set the vars so that they are acurate now, instead of waiting for the next gameloop
		if( FLoop_bGamePaused ) {
            FLoop_nPreviousLoopTicks = 0;
			FLoop_fPreviousLoopSecs = 0.0f;
			FLoop_fPreviousLoopOOSecs = 0.0f;
		} else {
			FLoop_nPreviousLoopTicks = FLoop_nRealPreviousLoopTicks;
			FLoop_fPreviousLoopSecs = FLoop_fRealPreviousLoopSecs;
			FLoop_fPreviousLoopOOSecs = FLoop_fRealPreviousLoopOOSecs;			
		}
	}
}


//
//
//
f32 floop_SetTimeScale( f32 fTimeScale ) 
{
	f32 fPrevVal = _fTimeScale;

	FASSERT( _bModuleInitialized );
	FASSERT( fTimeScale > 0.0f );

	if ( fTimeScale >= 0.0001f ) 
	{
		_fTimeScale = fTimeScale;
		_fOOTimeScale = 1.0f / _fTimeScale;
	} 
	else 
	{
		_fTimeScale = 0.0f;
		_fOOTimeScale = 0.0f;
	}

	_UpdateScaledTargetValues();
	_UpdateScaledBaseValues();

	return fPrevVal;
}


//
//
//
f32 floop_GetTimeScale( void ) 
{
	FASSERT( _bModuleInitialized );
	return _fTimeScale;
}

//
//
//
BOOL floop_EnableFixedLoopTimeMode( BOOL bEnable ) 
{
	BOOL bPrevVal;

	FASSERT( _bModuleInitialized );

	bPrevVal = _bFixedLoopTimeMode;
	_bFixedLoopTimeMode = bEnable;

	return bPrevVal;
}


//
//
//
BOOL floop_IsFixedLoopTimeModeEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	return _bFixedLoopTimeMode;
}


//
//
//
BOOL floop_EnableGovernor( BOOL bEnable ) 
{
	BOOL bPrevVal;

	FASSERT( _bModuleInitialized );

	bPrevVal = _bGovernFrameRate;
	_bGovernFrameRate = bEnable;

	return bPrevVal;
}


//
//
//
BOOL floop_IsGovernorEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	return _bGovernFrameRate;
}


//
//
//
void floop_Reset( void ) 
{
	FASSERT( _bModuleInitialized );

	FLoop_nRealPreviousLoopTicks = 0;
	FLoop_fRealPreviousLoopOOSecs = _fTargetFramesPerSec;
	FLoop_fRealPreviousLoopSecs = 1.0f / FLoop_fRealPreviousLoopOOSecs;
	FLoop_nRealTotalLoopTicks = 0;

	FLoop_nPreviousLoopTicks = (u64)((f32)FLoop_nRealPreviousLoopTicks);
	FLoop_fPreviousLoopSecs = FLoop_fRealPreviousLoopSecs;
	FLoop_fPreviousLoopOOSecs = FLoop_fRealPreviousLoopOOSecs;
	FLoop_nTotalLoopTicks = (u64)((f32)FLoop_nRealTotalLoopTicks);
	FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.

	_bResetMode = TRUE;
}



//-----------------------------------------------------------------------------------------------------------------
// Gameloop functions:


//
//
//
BOOL floop_InstallGameloop( FLoopInit_t *pFcnInit, FLoopMain_t *pFcnMain, FLoopTerm_t *pFcnTerm, void *pParameter, f32 fTargetFramesPerSec, f32 fBaseFramesPerSec ) 
{
	u64 nCurrentClockTicks, nDeltaClockTicks, nLoopSecsTimerStartTicks, nScaledDeltaClockTicks;
	f32 fFrameSecs, fWaitSecs;
	CFTimer GovernTimer, WaitTimer;

	FASSERT( _bModuleInitialized );

	_bFixedLoopTimeMode = FALSE;
	_bGovernFrameRate = FALSE;
	_bResetMode = FALSE;

	floop_SetTimeScale( 1.0f );
	FLoop_bGamePaused = FALSE;

	FLoop_nTicksPerSec = FTimer_nClockFrequency;
	FLoop_fTicksPerSec = (f32)FTimer_nClockFrequency;
	FLoop_fSecsPerTick = FTimer_fClockPeriod;

	floop_SetTargetFramesPerSec( fTargetFramesPerSec );
	floop_SetBaseFramesPerSec( fBaseFramesPerSec );

	FLoop_nRealPreviousLoopTicks = 0;
	FLoop_fRealPreviousLoopOOSecs = fTargetFramesPerSec;
	FLoop_fRealPreviousLoopSecs = 1.0f / FLoop_fRealPreviousLoopOOSecs;
	FLoop_fRealPreviousLoopSecs = FLOOP_INITIAL_FRAME_SECS;
	FLoop_fRealPreviousLoopOOSecs = 1.0f / FLOOP_INITIAL_FRAME_SECS;
	FLoop_nRealTotalLoopTicks = 0;

	FLoop_nPreviousLoopTicks = (u64)((f32)FLoop_nRealPreviousLoopTicks);
	FLoop_fPreviousLoopSecs = FLoop_fRealPreviousLoopSecs;
	FLoop_fPreviousLoopOOSecs = FLoop_fRealPreviousLoopOOSecs;
	FLoop_nTotalLoopTicks = (u64)((f32)FLoop_nRealTotalLoopTicks);
	FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.

	if ( pFcnInit ) 
	{
		if ( !pFcnInit( pParameter ) )
		{
			// Init failed...

			if ( pFcnTerm ) 
				pFcnTerm( FLOOP_TERMCODE_INITFAILED, pParameter );

			return FALSE;
		}
	}

	if ( pFcnMain ) 
	{
		nLoopSecsTimerStartTicks = ftimer_Clock_GetTicks();

		GovernTimer.Reset();

		while ( TRUE ) 
		{
			fperf_Reset();

			if ( !pFcnMain( FALSE, pParameter ) ) 
				break;

			ffile_Work();
			faudio_Work();

			if ( fpadio_IsInstalled() )
			{
				fforce_Work();
				fpadio_ApplyAllForcefeedbacks();
			}

			// Govern frame rate...
			if ( _bGovernFrameRate ) 
			{
				fFrameSecs = GovernTimer.SampleSeconds();
				fWaitSecs = _fTargetSecsPerFrame - fFrameSecs;

				if ( fWaitSecs > 0.0f ) 
				{
					WaitTimer.Reset();
					while ( WaitTimer.SampleSeconds() < fWaitSecs );
				}

				GovernTimer.Reset();
			}

			// Measure amount of time since last frame...
			nCurrentClockTicks = ftimer_Clock_GetTicks();

			if ( _bResetMode || _bFixedLoopTimeMode )
			{
				// We're in fixed loop time mode...
				nDeltaClockTicks = _nTargetTicksPerFrame;
				_bResetMode = FALSE;
			} 
			else 
			{
				// Compute delta ticks since last time we sampled the clock...
				nDeltaClockTicks = nCurrentClockTicks - nLoopSecsTimerStartTicks;
			}

			// Compute global time variables...
			FLoop_nRealTotalLoopTicks += nDeltaClockTicks;
			FLoop_nRealPreviousLoopTicks = nDeltaClockTicks;
			FLoop_fRealPreviousLoopSecs = (f32)FLoop_nRealPreviousLoopTicks * FLoop_fSecsPerTick;
			FLoop_fRealPreviousLoopOOSecs = 1.0f / FLoop_fRealPreviousLoopSecs;

			if( !FLoop_bGamePaused ) {
				if( _fTimeScale == 1.0f ) {
					nScaledDeltaClockTicks = nDeltaClockTicks;
				} else {
					nScaledDeltaClockTicks = (u64)((f32)nDeltaClockTicks * _fTimeScale);
				}
			} else {
				nScaledDeltaClockTicks = 0;
			}

			FLoop_nPreviousLoopTicks = nScaledDeltaClockTicks;
			FLoop_nTotalLoopTicks += FLoop_nPreviousLoopTicks;
			FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.
			FLoop_fPreviousLoopSecs = (f32)FLoop_nPreviousLoopTicks * FLoop_fSecsPerTick;
			FLoop_fPreviousLoopOOSecs = 1.0f / FLoop_fPreviousLoopSecs;

			// Save off sampled ticks...
			nLoopSecsTimerStartTicks = nCurrentClockTicks;

			#if FANG_DEBUG_BUILD
				// Debug builds...
				if ( FLoop_fRealPreviousLoopSecs > _MAX_ALLOWED_DELTA_SECS_FOR_DEBUG_BUILDS ) 
				{
					// Frame time is huge. Programmer has probably set a break point...
					FLoop_nRealTotalLoopTicks -= nDeltaClockTicks;
					FLoop_nRealTotalLoopTicks += _nTargetTicksPerFrame;

					FLoop_nRealPreviousLoopTicks = _nTargetTicksPerFrame;
					FLoop_fRealPreviousLoopSecs = _fTargetSecsPerFrame;
					FLoop_fRealPreviousLoopOOSecs = _fTargetFramesPerSec;

					FLoop_nTotalLoopTicks -= nScaledDeltaClockTicks;
					FLoop_nTotalLoopTicks += _nScaledTargetTicksPerFrame;
					FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.

					FLoop_nPreviousLoopTicks = _nScaledTargetTicksPerFrame;
					FLoop_fPreviousLoopSecs = _fScaledTargetSecsPerFrame;
					FLoop_fPreviousLoopOOSecs = _fScaledTargetFramesPerSec;
				}
			#else
				// Release builds...
				if ( _fBaseFramesPerSec > 0.0f ) 
				{
					if ( FLoop_fRealPreviousLoopSecs > _fBaseSecsPerFrame ) 
					{
						FLoop_nRealTotalLoopTicks -= nDeltaClockTicks;
						FLoop_nRealTotalLoopTicks += _nBaseTicksPerFrame;

						FLoop_nRealPreviousLoopTicks = _nBaseTicksPerFrame;
						FLoop_fRealPreviousLoopSecs = _fBaseSecsPerFrame;
						FLoop_fRealPreviousLoopOOSecs = _fBaseFramesPerSec;

						FLoop_nTotalLoopTicks -= nScaledDeltaClockTicks;
						FLoop_nTotalLoopTicks += _nScaledBaseTicksPerFrame;
						FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.

						FLoop_nPreviousLoopTicks = _nScaledBaseTicksPerFrame;
						FLoop_fPreviousLoopSecs = _fScaledBaseSecsPerFrame;
						FLoop_fPreviousLoopOOSecs = _fScaledBaseFramesPerSec;
					}
				}
			#endif
		}

		if ( pFcnTerm ) 
			pFcnTerm( FLOOP_TERMCODE_SUICIDE, pParameter );
	} 
	else 
	{
		if( pFcnTerm ) 
			pFcnTerm( FLOOP_TERMCODE_NULLLOOPFCN, pParameter );
	}

	return TRUE;
}


//
//
//
void floop_UninstallGameloop( void ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
BOOL floop_IsGameloopRunning( void ) 
{
	FASSERT( _bModuleInitialized );
	return TRUE;
}


//
//
//
BOOL floop_PauseGameloop( BOOL bPause ) 
{
	FASSERT( _bModuleInitialized );
	bPause;
	return TRUE;
}


//
//
//
BOOL floop_GetGameloopPauseState( void ) 
{
	FASSERT( _bModuleInitialized );
	return TRUE;
}


//
//
//
BOOL floop_SingleStepGameloop( void ) 
{
	FASSERT( _bModuleInitialized );
	return TRUE;
}


//
//
//
static void _UpdateScaledTargetValues( void ) 
{
	if ( !FLoop_bGamePaused ) 
	{
		_fScaledTargetTicksPerFrame = _fTargetTicksPerFrame * _fTimeScale;
		_nScaledTargetTicksPerFrame = (u64)((f32)_nTargetTicksPerFrame * _fTimeScale);
		_fScaledTargetFramesPerSec = _fTargetFramesPerSec * _fOOTimeScale;
		_fScaledTargetSecsPerFrame = _fTargetSecsPerFrame * _fTimeScale;
	} 
	else 
	{
		_fScaledTargetTicksPerFrame = 0.0f;
		_nScaledTargetTicksPerFrame = 0;
		_fScaledTargetFramesPerSec = 0.0f;
		_fScaledTargetSecsPerFrame = 0.0f;
	}
}


//
//
//
static void _UpdateScaledBaseValues( void ) 
{
	if ( _fBaseFramesPerSec>0.0f && !FLoop_bGamePaused ) 
	{
		_fScaledBaseFramesPerSec = _fBaseFramesPerSec * _fOOTimeScale;
		_fScaledBaseSecsPerFrame = _fBaseSecsPerFrame * _fTimeScale;
		_fScaledBaseTicksPerFrame = _fScaledBaseSecsPerFrame * FLoop_fTicksPerSec;
		_nScaledBaseTicksPerFrame = (u64)_fScaledBaseTicksPerFrame;
	} 
	else 
	{
		_fScaledBaseFramesPerSec = 0.0f;
		_fScaledBaseSecsPerFrame = 0.0f;
		_fScaledBaseTicksPerFrame = 0.0f;
		_nScaledBaseTicksPerFrame = 0;
	}
}

