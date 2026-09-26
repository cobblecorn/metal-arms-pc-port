//////////////////////////////////////////////////////////////////////////////////////
// fdx8loop.cpp - Fang gameloop module (Windows version).
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
// 10/22/99 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#if FANG_PLATFORM_WIN		 //pgm. If you need to include mfc files, it is best to do it nearest the top of the file if possible.
	#include <windows.h>
	#include <process.h>
#else
	#include <xtl.h>
#endif

#include "floop.h"
#include "ftimer.h"
#include "fpadio.h"
#include "fforce.h"
#include "fboxfilter.h"
#include "faudio.h"
#include "fperf.h"
#include "ffile.h"

#if FANG_PLATFORM_XB
	#include "fforce.h"
#endif

#define _USE_BOXFILTER		FALSE

#if _USE_BOXFILTER
	#include "fboxfilter.h"
#endif

#ifndef _FANGDEF_PRODUCTION_BUILD
#ifdef _FANGDEF_PLATFORM_XB
	#include "fserver.h"
#endif
#endif

// These are all updated at the start of each frame:
u64 FLoop_nPreviousLoopTicks;		// The number of ticks from the previous call to the main loop to the current call
f32 FLoop_fPreviousLoopSecs;		// The number of seconds from the previous call to the main loop to the current call
f32 FLoop_fPreviousLoopOOSecs;		// 1.0f / FLoop_fPreviousLoopSecs
u64 FLoop_nTotalLoopTicks;			// The number of ticks since the game loop has been started
u32 FLoop_nTotalLoopSecs;			// Low Precision version of the time since the game loop has been started

u32 FLoop_nTicksPerSec;				// The number of ticks per second (frequency)
f32 FLoop_fTicksPerSec;				// The number of ticks per second (frequency)
f32 FLoop_fSecsPerTick;				// The number of seconds per tick (period)

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



#if FANG_PLATFORM_WIN
	#define _SETUP_LOOP_EXIT	( WM_USER + 300 )

	// Port note: the original derived from MFC's CWinThread. This is a minimal Win32
	// replacement with the same lifecycle: InitInstance() -> Run() -> ExitInstance(),
	// a per-thread message queue for control messages, and auto-delete on exit.
	class CGameThread
	{
	public:
		CGameThread();
		~CGameThread();

		BOOL CreateThread( void );
		BOOL PostThreadMsg( UINT nMsg, WPARAM wParam, LPARAM lParam );	// (named to avoid the PostThreadMessage macro)

	protected:
		void OnSetupLoopExit( WPARAM wParam, LPARAM lParam );

		BOOL InitInstance();
		int ExitInstance();
		int Run();

		BOOL _ExecOneLoop( void );

		static unsigned __stdcall _ThreadEntry( void *pArg );

		HANDLE m_hThread;
		HANDLE m_hReadyEvent;		// signaled once the thread's message queue exists
		DWORD m_nThreadId;
		BOOL m_bAutoDelete;

#if _USE_BOXFILTER
		FBoxFilterHandle_t m_fltFrameTime;
#endif
	};

#endif// FANG_PLATFORM_WIN



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

#if FANG_PLATFORM_WIN
	static BOOL _bLoopSecsTimerRunning;
	static u64 _nLoopSecsTimerStartTicks;

	static u64 _nGovernTimerStartTicks;

	static CGameThread *_pGameloopThread = NULL;
	static FLoopInit_t *_pFcnGameThreadInit = NULL;
	static FLoopMain_t *_pFcnGameThreadMain = NULL;
	static FLoopTerm_t *_pFcnGameThreadTerm = NULL;
	static void *_pGameThreadParameter = NULL;
	static volatile BOOL _bGameThreadRunning = FALSE;
	static volatile BOOL _bGameThreadKillRequest = FALSE;
	static volatile BOOL _bGameThreadLoopExited = FALSE;
	static volatile BOOL _bGameThreadPause = FALSE;
	static volatile BOOL _bGameThreadStep = FALSE;
#endif




#if FANG_PLATFORM_WIN
	static void _ClearGameThreadState( void );
#endif

static void _UpdateScaledTargetValues( void );
static void _UpdateScaledBaseValues( void );



BOOL floop_ModuleStartup( void ) {
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

void floop_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	floop_UninstallGameloop();

	_bModuleInitialized = FALSE;
}

f32 floop_SetTargetFramesPerSec( f32 fTargetFramesPerSec ) {
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

f32 floop_GetTargetFramesPerSec( void ) {
	FASSERT( _bModuleInitialized );
	return _fTargetFramesPerSec;
}

f32 floop_GetTargetSecsPerFrame( void ) {
	FASSERT( _bModuleInitialized );
	return _fTargetSecsPerFrame;
}

// Set fBaseFramesPerSec to 0.0f to disable base frame rate clamping.
f32 floop_SetBaseFramesPerSec( f32 fBaseFramesPerSec ) {
	f32 fPrevVal;

	FASSERT( _bModuleInitialized );
	FASSERT( fBaseFramesPerSec >= 0.0f );

	fPrevVal = _fBaseFramesPerSec;

	if( fBaseFramesPerSec > 0.0f ) {
		_fBaseFramesPerSec = fBaseFramesPerSec;
		_fBaseSecsPerFrame = 1.0f / fBaseFramesPerSec;

		_fBaseTicksPerFrame = _fBaseSecsPerFrame * FLoop_fTicksPerSec;
		_nBaseTicksPerFrame = (u64)_fBaseTicksPerFrame;
		_fBaseFramesPerTick = _fBaseFramesPerSec * FLoop_fSecsPerTick;
	} else {
		_fBaseFramesPerSec = 0.0f;
		_fBaseSecsPerFrame = 0.0f;

		_fBaseTicksPerFrame = 0.0f;
		_nBaseTicksPerFrame = 0;
		_fBaseFramesPerTick = 0;
	}

	_UpdateScaledBaseValues();

	return fPrevVal;
}

f32 floop_GetBaseFramesPerSec( void ) {
	FASSERT( _bModuleInitialized );
	return _fBaseFramesPerSec;
}

f32 floop_GetBaseSecsPerFrame( void ) {
	FASSERT( _bModuleInitialized );
	return _fBaseSecsPerFrame;
}

void floop_PauseGame( BOOL bPause ) {
	FASSERT( _bModuleInitialized );

	bPause = !!bPause;

	if( bPause != FLoop_bGamePaused ) {
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


f32 floop_SetTimeScale( f32 fTimeScale ) {
	f32 fPrevVal = _fTimeScale;

	FASSERT( _bModuleInitialized );
	FASSERT( fTimeScale > 0.0f );

	if( fTimeScale >= 0.0001f ) {
		_fTimeScale = fTimeScale;
		_fOOTimeScale = 1.0f / _fTimeScale;
	} else {
		_fTimeScale = 0.0f;
		_fOOTimeScale = 0.0f;
	}

	_UpdateScaledTargetValues();
	_UpdateScaledBaseValues();

	return fPrevVal;
}

f32 floop_GetTimeScale( void ) {
	FASSERT( _bModuleInitialized );
	return _fTimeScale;
}

BOOL floop_EnableFixedLoopTimeMode( BOOL bEnable ) {
	BOOL bPrevVal;

	FASSERT( _bModuleInitialized );

	bPrevVal = _bFixedLoopTimeMode;
	_bFixedLoopTimeMode = bEnable;

	return bPrevVal;
}

BOOL floop_IsFixedLoopTimeModeEnabled( void ) {
	FASSERT( _bModuleInitialized );
	return _bFixedLoopTimeMode;
}

BOOL floop_EnableGovernor( BOOL bEnable ) {
	BOOL bPrevVal;

	FASSERT( _bModuleInitialized );

	bPrevVal = _bGovernFrameRate;
	_bGovernFrameRate = bEnable;

	return bPrevVal;
}

BOOL floop_IsGovernorEnabled( void ) {
	FASSERT( _bModuleInitialized );
	return _bGovernFrameRate;
}

void floop_Reset( void ) {
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

#if FANG_PLATFORM_WIN

#pragma optimize( "t", off )
// Creates a gameloop thread, begins execution of it, and returns immediately.
// If TRUE is returned, the gameloop thread was successfully created.
// Otherwise FALSE is returned and the gameloop is not running.
//
// If the gameloop thread is already running, it is terminated first.
//
// pFcnInit points to a user function that, if not NULL, will be called once at the beginning of the thread
// The function should return TRUE if it wishes the thread to continue, or FALSE if the thread should be terminated.
//
// pFcnMain points to a user function that, if not NULL, will be called in a repeated loop until the pFcnMain
// function returns FALSE. The bExitRequest parameter passed to pFcnMain indicates that the parent thread has
// requested the gameloop to shut down. If bExitRequest is TRUE, pFcnMain should cleanly shut down and return
// FALSE.
//
// pFcnTerm points to a user function that, if not NULL, will be called once when either pFcnInit or pFcnMain has
// returned FALSE. Note that pFcnTerm is called from the context of the gameloop thread.
//
// pParameter is a user-defined parameter that gets passed to the three gameloop functions.
BOOL floop_InstallGameloop( FLoopInit_t *pFcnInit, FLoopMain_t *pFcnMain, FLoopTerm_t *pFcnTerm, void *pParameter, f32 fTargetFramesPerSec, f32 fBaseFramesPerSec ) {
	FASSERT( _bModuleInitialized );

	floop_UninstallGameloop();

	_bFixedLoopTimeMode = FALSE;
	_bGovernFrameRate = FALSE;
	_bResetMode = FALSE;

	floop_SetTimeScale( 1.0f );
	FLoop_bGamePaused = FALSE;

	FLoop_nTicksPerSec = FTimer_nClockFrequency;
	FLoop_fTicksPerSec = (f32)FTimer_nClockFrequency;
	FLoop_fSecsPerTick = FTimer_fClockPeriod;

	_pFcnGameThreadInit = pFcnInit;
	_pFcnGameThreadMain = pFcnMain;
	_pFcnGameThreadTerm = pFcnTerm;
	_pGameThreadParameter = pParameter;

	_pGameloopThread = new CGameThread;
	if( _pGameloopThread == NULL ) {
		return FALSE;
	}

	floop_SetTargetFramesPerSec( fTargetFramesPerSec );
	floop_SetBaseFramesPerSec( fBaseFramesPerSec );

	FLoop_nRealPreviousLoopTicks = 0;
	FLoop_fRealPreviousLoopOOSecs = fTargetFramesPerSec;
	FLoop_fRealPreviousLoopSecs = 1.0f / FLoop_fRealPreviousLoopOOSecs;
	FLoop_nRealTotalLoopTicks = 0;

	FLoop_nPreviousLoopTicks = (u64)((f32)FLoop_nRealPreviousLoopTicks);
	FLoop_fPreviousLoopSecs = FLoop_fRealPreviousLoopSecs;
	FLoop_fPreviousLoopOOSecs = FLoop_fRealPreviousLoopOOSecs;
	FLoop_nTotalLoopTicks = (u64)((f32)FLoop_nRealTotalLoopTicks);
	FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.

	_bGameThreadRunning = TRUE;

	if( !_pGameloopThread->CreateThread() ) {
		// Could not create thread...
		_ClearGameThreadState();
		delete _pGameloopThread;
		_pGameloopThread = NULL;
		return FALSE;
	}

	return TRUE;
}
#pragma optimize( "t", on )


// Uninstalls the gameloop thread.
//
// IMPORTANT: Do not call this function from the context of the gameloop thread, or it will never return.
//            Note that pFcnInit, pFcnMain, and pFcnTerm execute in the context of the gameloop thread.
void floop_UninstallGameloop( void ) {
	FASSERT( _bModuleInitialized );

	if( _pGameloopThread ) {
		if( _bGameThreadRunning ) {
			// instead of just changing the state vars, post a message to do so,
			// this will ensure that the thread is not somewhere inside of the 
			// gameloop
			_pGameloopThread->PostThreadMsg( _SETUP_LOOP_EXIT, 0, 0 );

			while( _bGameThreadRunning ) {
				// no need to poll too often
				Sleep( 16 );
			}
		}

		_ClearGameThreadState();

		// Note that the thread auto-deletes its own object.
		_pGameloopThread = NULL;
	}
}

// Returns TRUE if the game thread is running.
// Returns FALSE otherwise.
BOOL floop_IsGameloopRunning( void ) {
	FASSERT( _bModuleInitialized );

	if( _pGameloopThread ) {
		return _bGameThreadRunning;
	} else {
		return FALSE;
	}
}

// Pauses or resumes the gameloop.
// Returns FALSE if the gameloop is not running, or if a kill request is pending.
// Otherwise returns TRUE.
// Usually the return code can just be ignored.
BOOL floop_PauseGameloop( BOOL bPause ) {
	FASSERT( _bModuleInitialized );

	if( _pGameloopThread ) {
		if( _bGameThreadRunning && !_bGameThreadKillRequest ) {
			_bGameThreadPause = bPause;
			_bGameThreadStep = FALSE;

			return TRUE;
		}
	}

	return FALSE;
}

// Returns TRUE if the gameloop is paused.
// Returns FALSE if the gameloop is running.
BOOL floop_GetGameloopPauseState( void ) {
	FASSERT( _bModuleInitialized );

	if( _pGameloopThread ) {
		return _bGameThreadPause;
	} else {
		return FALSE;
	}
}

// Single-steps the gameloop.
// If the gameloop is not already paused, this function simply calls PauseGameloop().
// Returns FALSE if the gameloop is not running, or if a kill request is pending.
// Otherwise returns TRUE.
// Usually the return code can just be ignored.
BOOL floop_SingleStepGameloop( void ) {
	FASSERT( _bModuleInitialized );

	if( _pGameloopThread ) {
		if( _bGameThreadRunning && !_bGameThreadKillRequest ) {
			if( _bGameThreadPause ) {
				_bGameThreadStep = TRUE;
			} else {
				floop_PauseGameloop( TRUE );
			}

			return TRUE;
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------------------------------------------
// CGameThread implementation:

// Constructor.
CGameThread::CGameThread() {
	m_bAutoDelete = TRUE;
	m_hThread = NULL;
	m_hReadyEvent = NULL;
	m_nThreadId = 0;

	#if _USE_BOXFILTER
		m_fltFrameTime = fboxfilter_Create_f32(16);
	#endif
}

// Destructor.
CGameThread::~CGameThread() {
	if( m_hThread ) {
		CloseHandle( m_hThread );
		m_hThread = NULL;
	}
	#if _USE_BOXFILTER
		fboxfilter_Delete_f32(m_fltFrameTime);
	#endif
}


int CGameThread::Run() {
	MSG Msg;
	BOOL bCallGameloop = TRUE;

	while( TRUE ) {
		if( PeekMessage( &Msg, NULL, 0, 0, PM_NOREMOVE) ) {
			// There is a message waiting...

			GetMessage( &Msg, NULL, 0, 0 );

			if( Msg.message == WM_QUIT ) {
				break;
			}

			if( Msg.hwnd == NULL && Msg.message == _SETUP_LOOP_EXIT ) {
				OnSetupLoopExit( Msg.wParam, Msg.lParam );
			} else {
				TranslateMessage( &Msg );
				DispatchMessage( &Msg );
			}
		} else {
			// No message waiting. Execute one game loop...

			if( bCallGameloop ) {
				bCallGameloop = _ExecOneLoop();
			}
		}
	}

	return ExitInstance();
}


BOOL CGameThread::_ExecOneLoop( void ) {
	u64 nCurrentClockTicks, nDeltaClockTicks, nScaledDeltaClockTicks;
	BOOL bLatchedKillRequest;
	f32 fFrameSecs, fWaitSecs;
	CFTimer WaitTimer;

	if( _bGameThreadLoopExited ) {
		this->PostThreadMsg(WM_QUIT, 0, 0);
		return FALSE;
	}

	if( _bGameThreadPause ) {
		// Gameloop is paused...
		_bLoopSecsTimerRunning = FALSE;

		if( !_bGameThreadStep ) {
			// Don't call gameloop...
			Sleep( 15 );
			return TRUE;
		}
	}

	if( _pFcnGameThreadMain ) {
		if( _bLoopSecsTimerRunning ) {
			// Sample clock...
			nCurrentClockTicks = ftimer_Clock_GetTicks();

			if( _bResetMode || _bFixedLoopTimeMode || _bGameThreadStep ) {
				// We're either in fixed loop time mode, or we're single-stepping...
				nDeltaClockTicks = _nTargetTicksPerFrame;
				_bResetMode = FALSE;
			} else {
				// Compute delta ticks since last time we sampled the clock...
				nDeltaClockTicks = nCurrentClockTicks - _nLoopSecsTimerStartTicks;
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
			_nLoopSecsTimerStartTicks = nCurrentClockTicks;

			#if FANG_DEBUG_BUILD
				// Debug builds...
				if( FLoop_fRealPreviousLoopSecs > _MAX_ALLOWED_DELTA_SECS_FOR_DEBUG_BUILDS ) {
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
				if( _fBaseFramesPerSec > 0.0f ) {
					if( FLoop_fRealPreviousLoopSecs > _fBaseSecsPerFrame ) {
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

			#if _USE_BOXFILTER
				// At this point, FLoop_fRealPreviousLoopSecs has been set somewhere up above.
				fboxfilter_Add_f32(m_fltFrameTime, FLoop_fRealPreviousLoopSecs);
				fboxfilter_Get_f32(m_fltFrameTime, NULL, &FLoop_fRealPreviousLoopSecs, NULL, NULL);
				FLoop_fRealPreviousLoopOOSecs = 1.0f / FLoop_fRealPreviousLoopSecs;
			#endif
		} else {
			// Sample clock...
			_nLoopSecsTimerStartTicks = ftimer_Clock_GetTicks();
			_bLoopSecsTimerRunning = TRUE;
		}

		fperf_Reset();

		bLatchedKillRequest = _bGameThreadKillRequest;		
		if( _pFcnGameThreadMain( bLatchedKillRequest, _pGameThreadParameter ) ) {
			// Gameloop function wishes to continue being called...

			ffile_Work();
			faudio_Work();

			if( _bGovernFrameRate ) {
				nCurrentClockTicks = ftimer_Clock_GetTicks();
				fFrameSecs = (f32)(nCurrentClockTicks - _nGovernTimerStartTicks) * FTimer_fClockPeriod;

				fWaitSecs = _fTargetSecsPerFrame - fFrameSecs;

				if( fWaitSecs > 0.0f ) {
					WaitTimer.Reset();
					while( !_bGameThreadKillRequest && WaitTimer.SampleSeconds()<fWaitSecs );
				}
			}

			_nGovernTimerStartTicks = ftimer_Clock_GetTicks();
		} else {
			// Gameloop function wishes to exit...
			_bGameThreadLoopExited = TRUE;
			_bGameThreadKillRequest = TRUE;

			if( _pFcnGameThreadTerm ) {
				if( bLatchedKillRequest ) {
					// Shutdown acknowledge...
					_pFcnGameThreadTerm( FLOOP_TERMCODE_SHUTDOWNACK, _pGameThreadParameter );
				} else {
					// Suicide...
					_pFcnGameThreadTerm( FLOOP_TERMCODE_SUICIDE, _pGameThreadParameter );
				}
			}
		}
	} else {
		// No gameloop function. Just exit...
		_bGameThreadLoopExited = TRUE;
		_bGameThreadKillRequest = TRUE;

		if( _pFcnGameThreadTerm ) {
			_pFcnGameThreadTerm( FLOOP_TERMCODE_NULLLOOPFCN, _pGameThreadParameter );
		}
	}

	// Turn off single step...
	_bGameThreadStep = FALSE;

	if( _bGameThreadLoopExited ) {
		this->PostThreadMsg(WM_QUIT, 0, 0);
		return FALSE;		
	}

	return TRUE;
}

unsigned __stdcall CGameThread::_ThreadEntry( void *pArg ) {
	CGameThread *pThread = (CGameThread *)pArg;
	MSG Msg;

	// Touching the message API creates this thread's message queue, so that
	// PostThreadMessage() from the parent can never fail for lack of a queue.
	PeekMessage( &Msg, NULL, WM_USER, WM_USER, PM_NOREMOVE );
	SetEvent( pThread->m_hReadyEvent );

	int nExitCode = 0;
	if( pThread->InitInstance() ) {
		nExitCode = pThread->Run();
	} else {
		nExitCode = pThread->ExitInstance();
	}

	if( pThread->m_bAutoDelete ) {
		delete pThread;
	}

	return (unsigned)nExitCode;
}

BOOL CGameThread::CreateThread( void ) {
	m_hReadyEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( m_hReadyEvent == NULL ) {
		return FALSE;
	}

	m_hThread = (HANDLE)_beginthreadex( NULL, 0, _ThreadEntry, this, 0, (unsigned *)&m_nThreadId );
	if( m_hThread == NULL ) {
		CloseHandle( m_hReadyEvent );
		m_hReadyEvent = NULL;
		return FALSE;
	}

	WaitForSingleObject( m_hReadyEvent, INFINITE );
	CloseHandle( m_hReadyEvent );
	m_hReadyEvent = NULL;

	return TRUE;
}

BOOL CGameThread::PostThreadMsg( UINT nMsg, WPARAM wParam, LPARAM lParam ) {
	return PostThreadMessage( m_nThreadId, nMsg, wParam, lParam );
}

BOOL CGameThread::InitInstance() {
	if( _pFcnGameThreadInit ) {
		if( !_pFcnGameThreadInit( _pGameThreadParameter ) ) {
			// Init failed...

			_bGameThreadLoopExited = TRUE;

			if( _pFcnGameThreadTerm ) {
				_pFcnGameThreadTerm( FLOOP_TERMCODE_INITFAILED, _pGameThreadParameter );
			}
		}
	}

	_bLoopSecsTimerRunning = FALSE;

	_nGovernTimerStartTicks = ftimer_Clock_GetTicks();

	return TRUE;
}

int CGameThread::ExitInstance() {
	_bGameThreadRunning = FALSE;

	return 0;
}

void CGameThread::OnSetupLoopExit( WPARAM wParam, LPARAM lParam ) {

	_bGameThreadPause = FALSE;
	_bGameThreadStep = FALSE;
	_bGameThreadKillRequest = TRUE;
}

static void _ClearGameThreadState( void ) {

	_pFcnGameThreadInit = NULL;
	_pFcnGameThreadMain = NULL;
	_pFcnGameThreadTerm = NULL;
	_pGameThreadParameter = 0;

	_bGameThreadRunning = FALSE;
	_bGameThreadKillRequest = FALSE;
	_bGameThreadLoopExited = FALSE;
	_bGameThreadPause = FALSE;
	_bGameThreadStep = FALSE;
}


#else

// Xbox:


BOOL floop_InstallGameloop( FLoopInit_t *pFcnInit, FLoopMain_t *pFcnMain, FLoopTerm_t *pFcnTerm, void *pParameter, f32 fTargetFramesPerSec, f32 fBaseFramesPerSec ) {
	u64 nCurrentClockTicks, nDeltaClockTicks, nLoopSecsTimerStartTicks, nScaledDeltaClockTicks;
	f32 fFrameSecs, fWaitSecs;
	CFTimer GovernTimer, WaitTimer;

	FASSERT( _bModuleInitialized );

	_bFixedLoopTimeMode = FALSE;
#if FANG_PRODUCTION_BUILD
	_bGovernFrameRate = TRUE;
#else
	_bGovernFrameRate = FALSE;
#endif
	_bResetMode = FALSE;

	floop_SetTimeScale( 1.0f );
	FLoop_bGamePaused = FALSE;

	FLoop_nTicksPerSec = FTimer_nClockFrequency;
	FLoop_fTicksPerSec = (f32)FTimer_nClockFrequency;
	FLoop_fSecsPerTick = FTimer_fClockPeriod;

#if FANG_PRODUCTION_BUILD
	floop_SetTargetFramesPerSec( 59.94f );
#else
	floop_SetTargetFramesPerSec( fTargetFramesPerSec );
#endif
	floop_SetBaseFramesPerSec( fBaseFramesPerSec );

	FLoop_nRealPreviousLoopTicks = 0;
	FLoop_fRealPreviousLoopOOSecs = fTargetFramesPerSec;
	FLoop_fRealPreviousLoopSecs = 1.0f / FLoop_fRealPreviousLoopOOSecs;
	FLoop_nRealTotalLoopTicks = 0;

	FLoop_nPreviousLoopTicks = (u64)((f32)FLoop_nRealPreviousLoopTicks);
	FLoop_fPreviousLoopSecs = FLoop_fRealPreviousLoopSecs;
	FLoop_fPreviousLoopOOSecs = FLoop_fRealPreviousLoopOOSecs;
	FLoop_nTotalLoopTicks = (u64)((f32)FLoop_nRealTotalLoopTicks);
	FLoop_nTotalLoopSecs = (u32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick); //set this every time TotalLoopTicks changes.

	if( pFcnInit ) {
		if( !pFcnInit( pParameter ) ) {
			// Init failed...

			if( pFcnTerm ) {
				pFcnTerm( FLOOP_TERMCODE_INITFAILED, pParameter );
			}

			return FALSE;
		}
	}

	if( pFcnMain ) {
		#if !FANG_PRODUCTION_BUILD && FANG_PLATFORM_XB
			BOOL bShutdownMainApp = FALSE;
		#endif

		nLoopSecsTimerStartTicks = ftimer_Clock_GetTicks();

		GovernTimer.Reset();

		while( TRUE ) {
			fperf_Reset();

			// !!Nate
			#if !FANG_PRODUCTION_BUILD && FANG_PLATFORM_XB
				if ( fserver_GetTakeover() && fserver_ShutdownApplication() && !bShutdownMainApp) {
					FASSERT( !pFcnMain( TRUE, pParameter ) );

					// Do this since MA level selector sets the governor to TRUE
					floop_EnableGovernor( FALSE );

					if( pFcnTerm ) {
						pFcnTerm( FLOOP_TERMCODE_SUICIDE, pParameter );
					}

					if( !fserver_GraphicsStartup() ) {
						DEVPRINTF("fserver - Graphics didn't startup\n");
					}

					bShutdownMainApp = TRUE;
				} else {
					if( !fserver_GetTakeover() && !pFcnMain( FALSE, pParameter ) ) {
						break;
					}
				}

				fserver_Work();
			#else
				if( !pFcnMain( FALSE, pParameter ) ) {
					break;
				}
			#endif
					
			faudio_Work();
			if ( fpadio_IsInstalled() )
			{
				fforce_Work();
				fpadio_ApplyAllForcefeedbacks();
			}

			// Govern frame rate...
			if( _bGovernFrameRate ) {
				fFrameSecs = GovernTimer.SampleSeconds();
				fWaitSecs = _fTargetSecsPerFrame - fFrameSecs;

				if( fWaitSecs > 0.0f ) {
					WaitTimer.Reset();
					while( WaitTimer.SampleSeconds() < fWaitSecs );
				}

				GovernTimer.Reset();
			}

			// Measure amount of time since last frame...
			nCurrentClockTicks = ftimer_Clock_GetTicks();

			if( _bResetMode || _bFixedLoopTimeMode ) {
				// We're in fixed loop time mode...
				nDeltaClockTicks = _nTargetTicksPerFrame;
				_bResetMode = FALSE;
			} else {
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

			if( FLoop_fPreviousLoopSecs == 0.0f )
			{
				FLoop_fPreviousLoopOOSecs = FMATH_MAX_FLOAT;
			}
			else
			{
				FLoop_fPreviousLoopOOSecs = 1.0f / FLoop_fPreviousLoopSecs;
			}

			// Save off sampled ticks...
			nLoopSecsTimerStartTicks = nCurrentClockTicks;

			#if FANG_DEBUG_BUILD
				// Debug builds...
				if( FLoop_fRealPreviousLoopSecs > _MAX_ALLOWED_DELTA_SECS_FOR_DEBUG_BUILDS ) {
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
				if( _fBaseFramesPerSec > 0.0f ) {
					if( FLoop_fRealPreviousLoopSecs > _fBaseSecsPerFrame ) {
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

		if( pFcnTerm ) {
			pFcnTerm( FLOOP_TERMCODE_SUICIDE, pParameter );
		}
	} else {
		if( pFcnTerm ) {
			pFcnTerm( FLOOP_TERMCODE_NULLLOOPFCN, pParameter );
		}
	}

	return TRUE;
}

void floop_UninstallGameloop( void ) {
	FASSERT( _bModuleInitialized );
}

BOOL floop_IsGameloopRunning( void ) {
	FASSERT( _bModuleInitialized );
	return TRUE;
}

BOOL floop_PauseGameloop( BOOL bPause ) {
	FASSERT( _bModuleInitialized );
	return TRUE;
}

BOOL floop_GetGameloopPauseState( void ) {
	FASSERT( _bModuleInitialized );
	return TRUE;
}

BOOL floop_SingleStepGameloop( void ) {
	FASSERT( _bModuleInitialized );
	return TRUE;
}

#endif



static void _UpdateScaledTargetValues( void ) {
	if( !FLoop_bGamePaused ) {
		_fScaledTargetTicksPerFrame = _fTargetTicksPerFrame * _fTimeScale;
		_nScaledTargetTicksPerFrame = (u64)((f32)_nTargetTicksPerFrame * _fTimeScale);
		_fScaledTargetFramesPerSec = _fTargetFramesPerSec * _fOOTimeScale;
		_fScaledTargetSecsPerFrame = _fTargetSecsPerFrame * _fTimeScale;
	} else {
		_fScaledTargetTicksPerFrame = 0.0f;
		_nScaledTargetTicksPerFrame = 0;
		_fScaledTargetFramesPerSec = 0.0f;
		_fScaledTargetSecsPerFrame = 0.0f;
	}
}

static void _UpdateScaledBaseValues( void ) {
	if( _fBaseFramesPerSec>0.0f && !FLoop_bGamePaused ) {
		_fScaledBaseFramesPerSec = _fBaseFramesPerSec * _fOOTimeScale;
		_fScaledBaseSecsPerFrame = _fBaseSecsPerFrame * _fTimeScale;
		_fScaledBaseTicksPerFrame = _fScaledBaseSecsPerFrame * FLoop_fTicksPerSec;
		_nScaledBaseTicksPerFrame = (u64)_fScaledBaseTicksPerFrame;
	} else {
		_fScaledBaseFramesPerSec = 0.0f;
		_fScaledBaseSecsPerFrame = 0.0f;
		_fScaledBaseTicksPerFrame = 0.0f;
		_nScaledBaseTicksPerFrame = 0;
	}
}

