// main_win.cpp - Windows entry point for the Metal Arms source port.
//
// Replaces the original MFC "mawin" launcher dialog (ma/App/ma/win/ma_winDlg.cpp).
// The boot sequence follows the Xbox entry point (ma/App/ma/xb/main.cpp): configure
// Fang, start it, describe the video mode, and hand off to gameloop_Start(), which
// runs the game on its own thread. This thread owns the render window, so it just
// pumps messages until the game asks to exit.
//
// Usage: ma_port [-data <dir>] [-mst <file>] [-res WxH] [-fullscreen] [-level <world-resource>] [-world-only <world-resource>] [-log <file>]
//
//   -data <dir>     directory holding the game's data (default: gamedata\files)
//   -mst <file>     master file name inside the data dir (default: mettlearms_gc.mst)
//   -res WxH        window/screen resolution (default: 1280x960)
//   -fullscreen     run fullscreen instead of in a window
//   -level <name>    launch a world directly as a generic debug level
//   -world-only <name> load a world resource, then exit before game/audio setup
//   -log <file>     write the engine's debug output here (default: ma_port.log)
//   -shots <dir>    save the back buffer to <dir>\shot_NNN.bmp every -shot-every frames (default 300)

#include "fang.h"
#include "fclib.h"
#include "fvid.h"
#include "floop.h"
#include "ffile.h"
#include "gameloop.h"

#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <rtcapi.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define _DEFAULT_DATA_DIR		"gamedata\\files"
#define _DEFAULT_MASTER_FILE	"mettlearms_gc.mst"
#define _DEFAULT_LOG_FILE		"ma_port.log"

static const f32 _FANG_HEAP_MB = 128.0f;		// desktop has plenty; the original PC build used 64

static char _szDataDir[MAX_PATH];
static char _szGameRoot[MAX_PATH + 2];
static char _szMasterName[MAX_PATH];			// bare file name from -mst
static char _szMasterFile[MAX_PATH * 2];		// full path
static char _szMovieDir[MAX_PATH * 2];
static char _szLogFile[MAX_PATH];
static char _szStartLevel[64];
static char _szWorldOnly[64];
static int _nReqWidth = 1280, _nReqHeight = 960;
static bool _bFullscreen = false;

static FILE *_pLog = NULL;
static DWORD _nMainThreadId = 0;
static GameloopInitParm_t _GameInitParms;

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

static void _LogV( const char *pszFormat, va_list Args )
{
	char szBuf[2048];
	_vsnprintf( szBuf, sizeof(szBuf) - 1, pszFormat, Args );
	szBuf[sizeof(szBuf) - 1] = 0;

	OutputDebugStringA( szBuf );
	fputs( szBuf, stdout );
	if( _pLog )
	{
		fputs( szBuf, _pLog );
		fflush( _pLog );
	}
}

static void _Log( const char *pszFormat, ... )
{
	va_list Args;
	va_start( Args, pszFormat );
	_LogV( pszFormat, Args );
	va_end( Args );
}

// Handler that Fang calls for its DEVPRINTF output.
static void _FangPrintf( cchar *pszFormat, FANG_VA_LIST Args )
{
	_LogV( pszFormat, Args );
}

// ---------------------------------------------------------------------------
// Debug-CRT diagnostics.
//
// The Debug CRT normally reports asserts, invalid parameters, pure-virtual calls, and
// /RTC (run-time check: stack/uninitialized-variable corruption) failures by popping a
// blocking "Microsoft Visual C++ Runtime Library" / "Debug Assertion Failed!" message
// box. In a headless or automated run there's nobody to click it, so the process just
// sits there forever looking like a hang. All of these are redirected here to the log
// instead, so a real run always produces a real diagnosis.
// ---------------------------------------------------------------------------

static int __cdecl _RTCErrorHandler( int nErrType, const char *pszFile, int nLine, const char *pszModule, const char *pszFormat, ... )
{
	char szMsg[1024];
	va_list Args;
	va_start( Args, pszFormat );
	_vsnprintf( szMsg, sizeof(szMsg) - 1, pszFormat, Args );
	va_end( Args );
	szMsg[sizeof(szMsg) - 1] = 0;

	_Log( "\n*** RUN-TIME CHECK FAILURE (type %d) at %s:%d [%s]:\n    %s\n", nErrType, pszFile ? pszFile : "?", nLine, pszModule ? pszModule : "?", szMsg );
	if( _pLog ) fflush( _pLog );
	return 0;	// 0 = continue running (like clicking "Ignore"); nonzero = break into a debugger
}

static void __cdecl _PurecallHandler( void )
{
	_Log( "\n*** PURE VIRTUAL FUNCTION CALL (R6025) - calling abort()\n" );
	if( _pLog ) fflush( _pLog );
	abort();
}

static void __cdecl _InvalidParameterHandler( const wchar_t *pszExpr, const wchar_t *pszFunc, const wchar_t *pszFile, unsigned int nLine, uintptr_t )
{
	_Log( "\n*** CRT INVALID PARAMETER at %ls:%u in %ls(%ls) - calling abort()\n", pszFile ? pszFile : L"?", nLine, pszFunc ? pszFunc : L"?", pszExpr ? pszExpr : L"?" );
	if( _pLog ) fflush( _pLog );
	abort();
}

static void __cdecl _SigAbortHandler( int )
{
	_Log( "\n*** abort() called\n" );
	if( _pLog ) fflush( _pLog );
}

static int __cdecl _CrtReportHook( int nReportType, char *pszMessage, int *pnReturnValue )
{
	static const char *const apszType[] = { "WARN", "ERROR", "ASSERT" };
	_Log( "\n*** CRT %s: %s\n", (nReportType >= 0 && nReportType <= 2) ? apszType[nReportType] : "REPORT", pszMessage ? pszMessage : "(no message)" );
	if( _pLog ) fflush( _pLog );
	if( pnReturnValue ) *pnReturnValue = 0;
	return TRUE;	// TRUE = we handled it; don't also show the CRT's own dialog
}

static void _InstallCrtDiagnostics( void )
{
	_RTC_SetErrorFunc( _RTCErrorHandler );
	_set_purecall_handler( _PurecallHandler );
	_set_invalid_parameter_handler( _InvalidParameterHandler );
	signal( SIGABRT, _SigAbortHandler );
	_CrtSetReportHook( _CrtReportHook );
	_set_error_mode( _OUT_TO_STDERR );
}

// ---------------------------------------------------------------------------
// Crash reporting: log the exception and a symbolized call stack (needs the .pdb
// next to the exe). Invaluable while bringing up 20-year-old code on a new platform.
// ---------------------------------------------------------------------------

static LONG WINAPI _CrashFilter( EXCEPTION_POINTERS *pEx )
{
	static volatile LONG nReentry = 0;
	if( InterlockedIncrement( &nReentry ) > 1 )
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	const EXCEPTION_RECORD *pRec = pEx->ExceptionRecord;
	_Log( "\n*** CRASH: exception 0x%08x at 0x%p (thread %lu)\n", (unsigned)pRec->ExceptionCode, pRec->ExceptionAddress, GetCurrentThreadId() );
	if( pRec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && pRec->NumberParameters >= 2 )
	{
		_Log( "    access violation: %s address 0x%p\n", pRec->ExceptionInformation[0] == 0 ? "reading" : (pRec->ExceptionInformation[0] == 1 ? "writing" : "executing"), (void *)pRec->ExceptionInformation[1] );
	}

	HANDLE hProcess = GetCurrentProcess();
	SymSetOptions( SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );
	SymInitialize( hProcess, NULL, TRUE );

	CONTEXT Ctx = *pEx->ContextRecord;
	STACKFRAME64 Frame;
	memset( &Frame, 0, sizeof(Frame) );
	Frame.AddrPC.Offset = Ctx.Eip;		Frame.AddrPC.Mode = AddrModeFlat;
	Frame.AddrFrame.Offset = Ctx.Ebp;	Frame.AddrFrame.Mode = AddrModeFlat;
	Frame.AddrStack.Offset = Ctx.Esp;	Frame.AddrStack.Mode = AddrModeFlat;

	for( int i = 0; i < 40; i++ )
	{
		if( !StackWalk64( IMAGE_FILE_MACHINE_I386, hProcess, GetCurrentThread(), &Frame, &Ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL ) ) break;
		if( Frame.AddrPC.Offset == 0 ) break;

		char aSymBuf[sizeof(SYMBOL_INFO) + 256];
		SYMBOL_INFO *pSym = (SYMBOL_INFO *)aSymBuf;
		memset( pSym, 0, sizeof(aSymBuf) );
		pSym->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSym->MaxNameLen = 255;

		DWORD64 nDisp64 = 0;
		DWORD nDisp = 0;
		IMAGEHLP_LINE64 Line;
		memset( &Line, 0, sizeof(Line) );
		Line.SizeOfStruct = sizeof(Line);

		const bool bSym = !!SymFromAddr( hProcess, Frame.AddrPC.Offset, &nDisp64, pSym );
		const bool bLine = !!SymGetLineFromAddr64( hProcess, Frame.AddrPC.Offset, &nDisp, &Line );
		if( bSym && bLine )	_Log( "    #%d 0x%08x %s  (%s:%lu)\n", i, (unsigned)Frame.AddrPC.Offset, pSym->Name, Line.FileName, Line.LineNumber );
		else if( bSym )		_Log( "    #%d 0x%08x %s\n", i, (unsigned)Frame.AddrPC.Offset, pSym->Name );
		else				_Log( "    #%d 0x%08x\n", i, (unsigned)Frame.AddrPC.Offset );
	}

	fflush( stdout );
	if( _pLog ) fflush( _pLog );
	return EXCEPTION_EXECUTE_HANDLER;
}

// ---------------------------------------------------------------------------
// Game callbacks (invoked from the game thread)
// ---------------------------------------------------------------------------

static void _GameloopExit( void )
{
	// The game has finished. Wake the main thread's message pump.
	PostThreadMessage( _nMainThreadId, WM_QUIT, 0, 0 );
}

static void _GameloopMinimize( void )
{
	// Nothing to do: Windows handles minimizing the render window.
}

// ---------------------------------------------------------------------------
// Command line
// ---------------------------------------------------------------------------

static void _Usage( void )
{
	_Log( "Usage: ma_port [-data <dir>] [-mst <file>] [-res WxH] [-fullscreen] [-level <world-resource>] [-world-only <world-resource>] [-log <file>] [-shots <dir> [-shot-every <frames>]]\n" );
}

static bool _ParseArgs( int argc, char **argv )
{
	strcpy( _szDataDir, _DEFAULT_DATA_DIR );
	strcpy( _szMasterName, _DEFAULT_MASTER_FILE );
	strcpy( _szLogFile, _DEFAULT_LOG_FILE );
	_szStartLevel[0] = 0;
	_szWorldOnly[0] = 0;

	for( int i = 1; i < argc; i++ )
	{
		const char *pszArg = argv[i];
		const bool bHasValue = (i + 1 < argc);

		if( !_stricmp( pszArg, "-data" ) && bHasValue )				strncpy( _szDataDir, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-mst" ) && bHasValue )			strncpy( _szMasterName, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-log" ) && bHasValue )			strncpy( _szLogFile, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-level" ) && bHasValue )		strncpy( _szStartLevel, argv[++i], sizeof(_szStartLevel) - 1 );
		else if( !_stricmp( pszArg, "-world-only" ) && bHasValue )	strncpy( _szWorldOnly, argv[++i], sizeof(_szWorldOnly) - 1 );
		else if( !_stricmp( pszArg, "-fullscreen" ) )				_bFullscreen = true;
		else if( !_stricmp( pszArg, "-shots" ) && bHasValue )		SetEnvironmentVariableA( "MA_PORT_SHOTS", argv[++i] );	// read by compat/d3d8_compat.cpp
		else if( !_stricmp( pszArg, "-shot-every" ) && bHasValue )	SetEnvironmentVariableA( "MA_PORT_SHOT_EVERY", argv[++i] );
		else if( !_stricmp( pszArg, "-res" ) && bHasValue )
		{
			if( sscanf( argv[++i], "%dx%d", &_nReqWidth, &_nReqHeight ) != 2 || _nReqWidth < 320 || _nReqHeight < 200 )
			{
				_Log( "Bad -res value '%s' (expected e.g. 1280x960)\n", argv[i] );
				return false;
			}
		}
		else
		{
			_Log( "Unknown or incomplete option: %s\n", pszArg );
			_Usage();
			return false;
		}
	}
	if( _szStartLevel[0] && _szWorldOnly[0] )
	{
		_Log( "Choose either -level or -world-only.\n" );
		_Usage();
		return false;
	}

	// Normalize the data directory to end in a backslash and derive the other paths from it.
	strncpy( _szGameRoot, _szDataDir, MAX_PATH );
	size_t nLen = strlen( _szGameRoot );
	if( nLen == 0 || (_szGameRoot[nLen - 1] != '\\' && _szGameRoot[nLen - 1] != '/') )
	{
		_szGameRoot[nLen++] = '\\';
		_szGameRoot[nLen] = 0;
	}
	sprintf( _szMasterFile, "%s%s", _szGameRoot, _szMasterName );
	sprintf( _szMovieDir, "%sMovies\\", _szGameRoot );
	return true;
}

// ---------------------------------------------------------------------------
// Video: pick the first hardware device and the mode closest to the request
// ---------------------------------------------------------------------------

static bool _PickVideoMode( FVidWin_t *pWin )
{
	fvid_Enumerate( FVID_RENDERER_HARDWARE );

	const u32 nDevCount = fvid_GetDeviceCount();
	if( nDevCount == 0 )
	{
		_Log( "No Direct3D devices were found.\n" );
		return false;
	}

	int nBestDev = -1, nBestMode = -1;
	long nBestScore = 0x7fffffff;

	for( u32 d = 0; d < nDevCount; d++ )
	{
		const FVidDev_t *pDev = fvid_GetDeviceInfo( d );
		if( pDev->nRenderer != FVID_RENDERER_HARDWARE ) continue;

		const u32 nModeCount = fvid_GetModeCount( d );
		for( u32 m = 0; m < nModeCount; m++ )
		{
			const FVidMode_t *pMode = fvid_GetModeInfo( d, m );

			const bool bWindowed = !!(pMode->nFlags & FVID_MODEFLAG_WINDOWED);
			if( bWindowed == _bFullscreen ) continue;
			if( pMode->nColorBits != 32 || pMode->nDepthBits < 24 ) continue;

			long nScore = labs( (long)pMode->nPixelsAcross - _nReqWidth ) + labs( (long)pMode->nPixelsDown - _nReqHeight );
			nScore = nScore * 4 + (pMode->nStencilBits ? 0 : 1);		// prefer a stencil buffer when otherwise equal
			if( nScore < nBestScore )
			{
				nBestScore = nScore;
				nBestDev = (int)d;
				nBestMode = (int)m;
			}
		}
	}

	if( nBestDev < 0 )
	{
		_Log( "No suitable %s video mode was found.\n", _bFullscreen ? "fullscreen" : "windowed" );
		return false;
	}

	const FVidDev_t *pDev = fvid_GetDeviceInfo( (u32)nBestDev );
	const FVidMode_t *pMode = fvid_GetModeInfo( (u32)nBestDev, (u32)nBestMode );
	_Log( "Video: %s, %dx%d, %d-bit color, %d-bit depth, %d-bit stencil, %s\n", pDev->szName,
		  pMode->nPixelsAcross, pMode->nPixelsDown, pMode->nColorBits, pMode->nDepthBits, pMode->nStencilBits,
		  (pMode->nFlags & FVID_MODEFLAG_WINDOWED) ? "windowed" : "fullscreen" );

	pWin->VidDev = *pDev;
	pWin->VidMode = *pMode;
	return true;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main( int argc, char **argv )
{
	_nMainThreadId = GetCurrentThreadId();
	SetUnhandledExceptionFilter( _CrashFilter );
	_InstallCrtDiagnostics();
	setvbuf( stdout, NULL, _IONBF, 0 );

	if( !_ParseArgs( argc, argv ) )
	{
		return 2;
	}

	_pLog = fopen( _szLogFile, "w" );

	_Log( "Metal Arms port\n  data:   %s\n  master: %s\n  movies: %s\n", _szGameRoot, _szMasterFile, _szMovieDir );

	//////////////////////////////////////////////////////////////////////
	// Fang engine
	fang_Init();

	Fang_ConfigDefs.pFang_FcnPrintf = _FangPrintf;
	Fang_ConfigDefs.nRes_HeapBytes = (u32)( _FANG_HEAP_MB * 1024.0f * 1024.0f );
	Fang_ConfigDefs.pszFile_GameRootPathName = _szGameRoot;
	Fang_ConfigDefs.pszFile_MasterFilePathName = _szMasterFile;
	Fang_ConfigDefs.pszMovie_BasePathName = _szMovieDir;
	Fang_ConfigDefs.nAudio_MaxSoundBytes = 1024 * (36 * 1024);
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 4048000;
	Fang_ConfigDefs.nWorld_MaxIntersects = 3500;
	Fang_ConfigDefs.bCheckPoint_StartupSystem = TRUE;
	Fang_ConfigDefs.nText_MaxCharsPerFrame = 550;
	Fang_ConfigDefs.nText_MaxCharsPerPrintf = 190;
	Fang_ConfigDefs.nMaxParticleEmitters = 500;
	Fang_ConfigDefs.nMaxParticles = 3000;
	Fang_ConfigDefs.nMaxParticleEmitterSprites = 1000;
	Fang_nLaunchType = FANG_LAUNCH_TYPE_STANDALONE;

	if( !fang_Startup() )
	{
		_Log( "Fang engine failed to start.\n" );
		return 1;
	}
	if( !ffile_LogSetFilename( "ma_port_asset_log.txt" ) )
	{
		_Log( "Could not create the Fang resource log.\n" );
		fang_Shutdown();
		return 1;
	}

	//////////////////////////////////////////////////////////////////////
	// Game loop parameters
	memset( &_GameInitParms, 0, sizeof(_GameInitParms) );

	_GameInitParms.fTargetFPS = GAMELOOP_DEFAULT_TARGET_FPS;
	_GameInitParms.bSkipLevelSelect = _szStartLevel[0] != 0;
	_GameInitParms.nAnimPlaybackRate = GAMELOOP_DEFAULT_ANIM_PLAYBACK;
	_GameInitParms.bViewBounds = GAMELOOP_DEFAULT_VIEW_BOUNDS;
	_GameInitParms.bShowFPS = FALSE;
	_GameInitParms.bDrawScreenSafeArea = FALSE;
	_GameInitParms.nPlatform = GAMELOOP_PLATFORM_GC;			// the retail data set is the GameCube one
	_GameInitParms.nMaxSoundMgrSounds = GAMELOOP_DEFAULT_MAX_SOUNDS;
	_GameInitParms.pszInputFilename = _szStartLevel[0] ? _szStartLevel : NULL;	// Non-empty = quick launch this world as a generic debug level.
	_GameInitParms.pszScreenShotDir = GAMELOOP_DEFAULT_SCREENSHOT_DIR;
	_GameInitParms.BGColorRGB.Black();
	_GameInitParms.pExitFunc = _GameloopExit;
	_GameInitParms.pMinFunc = _GameloopMinimize;
	_GameInitParms.nMemCardUsageFlags = GAMELOOP_MEMCARD_NONE;
	_GameInitParms.pszMemCardDir = NULL;
	_GameInitParms.pauInputEmulationMap = NULL;
	_GameInitParms.pszInputEmulationDevName = NULL;
	_GameInitParms.bInstallAudio = FALSE;
	_GameInitParms.bLoadWorldOnly = _szWorldOnly[0] != 0;
	if( _GameInitParms.bLoadWorldOnly )
	{
		_GameInitParms.bSkipLevelSelect = TRUE;
		_GameInitParms.pszInputFilename = _szWorldOnly;
	}
	_GameInitParms.bGovernFrameRate = FALSE;
	_GameInitParms.bDemoLaunched = FALSE;
	_GameInitParms.uTimeoutInterval = 0;

	FVidWin_t &Win = _GameInitParms.VidWin;
	memset( &Win, 0, sizeof(Win) );
	if( !_PickVideoMode( &Win ) )
	{
		fang_Shutdown();
		return 1;
	}
	Win.nSwapInterval = 1;										// vsync
	Win.fUnitFSAA = 0.0f;
	Win.hInstance = GetModuleHandle( NULL );
	Win.hWnd = 0;
	Win.nIconIDI = 0;
	Win.bAllowPowerSuspend = TRUE;
	Win.pFcnSuspend = NULL;
	strcpy( Win.szWindowTitle, "Metal Arms: Glitch in the System" );

	//////////////////////////////////////////////////////////////////////
	// Go. gameloop_Start() creates the window on this thread and runs the game on another.
	if( !gameloop_Start( &_GameInitParms ) )
	{
		_Log( "The game loop could not be started.\n" );
		fang_Shutdown();
		return 1;
	}

	// The render window belongs to this thread, so pump its messages until the game exits.
	MSG Msg;
	while( GetMessage( &Msg, NULL, 0, 0 ) > 0 )
	{
		TranslateMessage( &Msg );
		DispatchMessage( &Msg );
	}

	gameloop_End();
	fang_Shutdown();

	if( _pLog ) fclose( _pLog );
	return 0;
}
