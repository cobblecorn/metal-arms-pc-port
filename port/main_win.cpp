// main_win.cpp - Windows entry point for the Metal Arms source port.
//
// Replaces the original MFC "mawin" launcher dialog (ma/App/ma/win/ma_winDlg.cpp).
// The boot sequence follows the Xbox entry point (ma/App/ma/xb/main.cpp): configure
// Fang, start it, describe the video mode, and hand off to gameloop_Start(), which
// runs the game on its own thread. This thread owns the render window, so it just
// pumps messages until the game asks to exit.
//
// Usage: ma_port [-data <dir>] [-mst <file>] [-res WxH] [-fullscreen] [-log <file>]
//
//   -data <dir>     directory holding the game's data (default: gamedata\files)
//   -mst <file>     master file name inside the data dir (default: mettlearms_gc.mst)
//   -res WxH        window/screen resolution (default: 1280x960)
//   -fullscreen     run fullscreen instead of in a window
//   -log <file>     write the engine's debug output here (default: ma_port.log)

#include "fang.h"
#include "fclib.h"
#include "fvid.h"
#include "floop.h"
#include "ffile.h"
#include "gameloop.h"

#include <windows.h>
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
	_Log( "Usage: ma_port [-data <dir>] [-mst <file>] [-res WxH] [-fullscreen] [-log <file>]\n" );
}

static bool _ParseArgs( int argc, char **argv )
{
	strcpy( _szDataDir, _DEFAULT_DATA_DIR );
	strcpy( _szMasterName, _DEFAULT_MASTER_FILE );
	strcpy( _szLogFile, _DEFAULT_LOG_FILE );

	for( int i = 1; i < argc; i++ )
	{
		const char *pszArg = argv[i];
		const bool bHasValue = (i + 1 < argc);

		if( !_stricmp( pszArg, "-data" ) && bHasValue )				strncpy( _szDataDir, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-mst" ) && bHasValue )			strncpy( _szMasterName, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-log" ) && bHasValue )			strncpy( _szLogFile, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-fullscreen" ) )				_bFullscreen = true;
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

	//////////////////////////////////////////////////////////////////////
	// Game loop parameters
	memset( &_GameInitParms, 0, sizeof(_GameInitParms) );

	_GameInitParms.fTargetFPS = GAMELOOP_DEFAULT_TARGET_FPS;
	_GameInitParms.bSkipLevelSelect = GAMELOOP_DEFAULT_SKIP_LEVEL_SELECT;
	_GameInitParms.nAnimPlaybackRate = GAMELOOP_DEFAULT_ANIM_PLAYBACK;
	_GameInitParms.bViewBounds = GAMELOOP_DEFAULT_VIEW_BOUNDS;
	_GameInitParms.bShowFPS = FALSE;
	_GameInitParms.bDrawScreenSafeArea = FALSE;
	_GameInitParms.nPlatform = GAMELOOP_PLATFORM_GC;			// the retail data set is the GameCube one
	_GameInitParms.nMaxSoundMgrSounds = GAMELOOP_DEFAULT_MAX_SOUNDS;
	_GameInitParms.pszInputFilename = NULL;						// NULL = normal game flow (front end, level select, ...)
	_GameInitParms.pszScreenShotDir = GAMELOOP_DEFAULT_SCREENSHOT_DIR;
	_GameInitParms.BGColorRGB.Black();
	_GameInitParms.pExitFunc = _GameloopExit;
	_GameInitParms.pMinFunc = _GameloopMinimize;
	_GameInitParms.nMemCardUsageFlags = GAMELOOP_MEMCARD_NONE;
	_GameInitParms.pszMemCardDir = NULL;
	_GameInitParms.pauInputEmulationMap = NULL;
	_GameInitParms.pszInputEmulationDevName = NULL;
	_GameInitParms.bInstallAudio = TRUE;
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
