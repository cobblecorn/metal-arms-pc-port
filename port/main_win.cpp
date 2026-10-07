// main_win.cpp - Windows entry point for the Metal Arms source port.
//
// Replaces the original MFC "mawin" launcher dialog (ma/App/ma/win/ma_winDlg.cpp).
// The boot sequence follows the Xbox entry point (ma/App/ma/xb/main.cpp): configure
// Fang, start it, describe the video mode, and hand off to gameloop_Start(), which
// runs the game on its own thread. This thread owns the render window, so it just
// pumps messages until the game asks to exit.
//
// Usage: ma_port [-data <dir>] [-mst <file>] [-res WxH] [-fullscreen] [-level <world-resource> | -mission <world-resource>] [-world-only <world-resource> | -export-character-meshes <list-file>] [-log <file>] [-asset-log <file>]
//
//   -data <dir>     directory holding the game's data (default: gamedata\files)
//   -mst <file>     master file name inside the data dir (default: mettlearms_gc.mst)
//   -res WxH        window/screen resolution (default: 1280x960)
//   -fullscreen     run fullscreen instead of in a window
//   -borderless     a borderless window covering the desktop
//   -windowed       a normal window (the default unless settings.ini [Display] says otherwise)
//   -no-audio       skip game sound setup and mute Bink movie audio
//   -mute           load and run all audio (so its errors are logged) but play it silently; for tests
//   -dev-menu       boot into the development launcher (level picker) instead of the retail front end
//   -console        open a console window showing the log (the game is a windowed app without one)
//   -port-diag      log the port's periodic PORT-* diagnostics (also MA_PORT_DIAG=1)
//   -no-vsync       present immediately instead of on the display's refresh (for measuring)
//   -discord-app-id <id> show Discord Rich Presence under this Discord application instead of the
//                   port's own (also MA_PORT_DISCORD_APP_ID); "off" turns Rich Presence off
//   -discord-large-image <asset-key-or-url> rich-presence image from that application's assets
//                   (also MA_PORT_DISCORD_LARGE_IMAGE)
//   -debug-info     draw the game's debug overlays: on-screen script messages and errors (errors
//                   pause the game), frame rate, checkpoint and AI debug drawing. Scripts always log.
//   -mission <name> load a registered single-player world with its mission data
//   -coop <2-4>     experimental local campaign co-op player slots; requires -mission; shared or separate inputs
//   -level <name>    launch a world directly as a generic debug level
//   -world-only <name> load a world resource, then exit before game/audio setup
//   -export-character-meshes <file> load one MESH resource name per line and write rigged model data
//   -log <file>     write the engine's debug output here (default: ma_port.log)
//   -asset-log <file> write Fang's resource-loading output here (default: ma_port_asset_log.txt)
//   -mouse-sensitivity <n> raw mouse sensitivity in degrees per count (default 0.1)
//   -aim-assist <auto|on|off> target assistance: auto = controller aiming only (default)
//   -input-layout <shared|separate> shared: keyboard/mouse and pad 1 drive port 0 (default);
//                   separate: keyboard/mouse alone on port 0, pads 1-3 on ports 1-3 (local co-op)
//   -button-prompts <auto|keyboard|xbox|playstation> choose prompt glyphs and wording (default auto)
//   -test-keys <s:vk,...> press these virtual keys (e.g. 40:0x1B) that many seconds after start, for
//                   unattended tests with -shots; they work without the window having focus. "g8:0x1B"
//                   counts from the first gameplay frame instead (pauses a mission 8 s into play)
//   -shots <dir>    save the back buffer to <dir>\shot_NNN.bmp every -shot-every frames (default 300)
//   -save-dir <dir> the save root: Profiles and Co-op folders and settings.ini (default: %APPDATA%\MAGITS)
//   -start-at X,Y,Z[,YAW] test/play aid: move player 1 to this world position (yaw in degrees) once play begins
//   -sfx-db <dB>    trim for sound effects other than dialogue; default -11, 0 = retail mix
//   -aniso <n>      anisotropic texture filtering level (default 16, capped by the GPU; 1 = off)
//   -test-win-level <s> test aid: complete the loaded level s seconds in (reaches the results screen)
//   -test-give <item>   test aid: give player 1 a weapon/throwable (e.g. "coring charge") and select it
//   -coop-hold on|off  co-op tripwire events wait for every player (default on; off = test aid)
//   -player-sfx-db <dB> further trim for the player's own 2D sounds (weapons, footsteps); default -6, 0 = retail mix
//   -instance-label <name> add a short label to the window title (useful for parallel test windows)

#include "res/resource.h"
#include "discord_rpc.h"
#include "fang.h"
#include "fclib.h"
#include "fvid.h"
#include "floop.h"
#include "ffile.h"
#include "fmovie2.h"
#include "gameloop.h"
#include "launcher.h"
#include "pc_input.h"
#include "pc_display.h"

extern BOOL FDX8Vid_bPortBorderless;	// Fang2/dx/fdx8vid.cpp

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
#define _DEFAULT_ASSET_LOG_FILE	"ma_port_asset_log.txt"

static const f32 _FANG_HEAP_MB = 128.0f;		// desktop has plenty; the original PC build used 64

static char _szDataDir[MAX_PATH];
static char _szGameRoot[MAX_PATH + 2];
static char _szMasterName[MAX_PATH];			// bare file name from -mst
static char _szMasterFile[MAX_PATH * 2];		// full path
static char _szMovieDir[MAX_PATH * 2];
static char _szLogFile[MAX_PATH];
static char _szAssetLogFile[MAX_PATH];
static char _szStartLevel[64];
static char _szMission[64];
static char _szWorldOnly[64];
static char _szCharacterMeshList[MAX_PATH];
static char _szInstanceLabel[64];
static bool _bStartAt = false;
static float _afStartAt[4];				// -start-at X,Y,Z[,YAW degrees]
static float _fTestWinLevelSecs = 0.0f;	// -test-win-level S
static char _szTestGive[64];				// -test-give ITEM: give player 1 this weapon/throwable and select it
static int _nReqWidth = 1280, _nReqHeight = 960;
static bool _bFullscreen = false;
static bool _bBorderless = false;
static bool _bModeArg = false, _bResArg = false;	// the command line chose these; settings.ini is not used for them
static bool _bNoAudio = false;
static bool _bMute = false;
extern BOOL FAudio_bPortMuteOutput;	// Fang2/dx/fdx8audio.cpp
static int _nCampaignCoopPlayers = 1;
static bool _bDebugInfo = false;
static bool _bConsole = false;
static bool _bPortDiag = false;
static bool _bNoVsync = false;
static char _szDiscordAppId[32];
// The port's own Discord application, used unless -discord-app-id / MA_PORT_DISCORD_APP_ID names another.
static const char _szDefaultDiscordAppId[] = "1553650972218363985";
static char _szDiscordLargeImage[128];
static char _szDiscordLargeText[128];
static bool _bDevMenu = false;

static FILE *_pLog = NULL;
static DWORD _nMainThreadId = 0;
static GameloopInitParm_t _GameInitParms;

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------

// The game never writes the log file (or the console) itself: lines go into a buffer that a background
// thread writes out every 50 ms. Writing and flushing each line from the game thread stalled frames for
// up to a second whenever the disk was busy. Crash, report and exit paths write synchronously
// (_LogFlush), so the last lines before a crash still reach the file.
#define _LOG_BUFFER_MAX		(16u << 20)
static SRWLOCK _LogBufLock = SRWLOCK_INIT;		// the pending text
static SRWLOCK _LogWriteLock = SRWLOCK_INIT;	// one writer at a time, in order
static char *_pLogBuf, *_pLogOut;
static size_t _nLogBufUsed, _nLogBufCap, _nLogOutCap;
static unsigned _nLogDropped;
static HANDLE _hLogThread;
static volatile LONG _nLogQuit;

static void _LogAppend( const char *psz )
{
	const size_t nLen = strlen( psz );
	AcquireSRWLockExclusive( &_LogBufLock );
	if( _nLogBufUsed + nLen > _nLogBufCap )
	{
		size_t nCap = _nLogBufCap ? _nLogBufCap : 65536;
		while( nCap < _nLogBufUsed + nLen && nCap < _LOG_BUFFER_MAX ) nCap *= 2;
		char *pNew = ( nCap >= _nLogBufUsed + nLen ) ? (char *)realloc( _pLogBuf, nCap ) : NULL;
		if( pNew ) { _pLogBuf = pNew; _nLogBufCap = nCap; }
	}
	if( _nLogBufUsed + nLen <= _nLogBufCap )
	{
		memcpy( _pLogBuf + _nLogBufUsed, psz, nLen );
		_nLogBufUsed += nLen;
	}
	else
	{
		_nLogDropped++;
	}
	ReleaseSRWLockExclusive( &_LogBufLock );
}

// Writes out everything logged so far.
static void _LogFlush( void )
{
	AcquireSRWLockExclusive( &_LogWriteLock );
	AcquireSRWLockExclusive( &_LogBufLock );
	size_t nLen = _nLogBufUsed;
	const unsigned nDropped = _nLogDropped;
	_nLogDropped = 0;
	if( nLen > _nLogOutCap )
	{
		char *pNew = (char *)realloc( _pLogOut, nLen );
		if( pNew ) { _pLogOut = pNew; _nLogOutCap = nLen; }
	}
	const bool bCopied = nLen <= _nLogOutCap;
	if( bCopied )
	{
		if( nLen ) memcpy( _pLogOut, _pLogBuf, nLen );
		_nLogBufUsed = 0;
		ReleaseSRWLockExclusive( &_LogBufLock );
	}
	// (out of memory for the copy: write straight from the buffer, holding it)
	const char *pText = bCopied ? _pLogOut : _pLogBuf;
	if( nLen )
	{
		fwrite( pText, 1, nLen, stdout );
		if( _pLog ) fwrite( pText, 1, nLen, _pLog );
	}
	if( !bCopied )
	{
		_nLogBufUsed = 0;
		ReleaseSRWLockExclusive( &_LogBufLock );
	}
	if( nDropped && _pLog ) fprintf( _pLog, "(log: %u lines dropped, the log buffer was full)\n", nDropped );
	if( ( nLen || nDropped ) && _pLog ) fflush( _pLog );
	ReleaseSRWLockExclusive( &_LogWriteLock );
}

static DWORD WINAPI _LogWriter( void * )
{
	while( !InterlockedCompareExchange( &_nLogQuit, 0, 0 ) )
	{
		Sleep( 50 );
		_LogFlush();
	}
	return 0;
}

static void _LogV( const char *pszFormat, va_list Args )
{
	char szBuf[2048];
	_vsnprintf( szBuf, sizeof(szBuf) - 1, pszFormat, Args );
	szBuf[sizeof(szBuf) - 1] = 0;

	OutputDebugStringA( szBuf );
	_LogAppend( szBuf );
}

static void _Log( const char *pszFormat, ... )
{
	va_list Args;
	va_start( Args, pszFormat );
	_LogV( pszFormat, Args );
	va_end( Args );
}

static void _DiscordLog( const char *pszText )
{
	_Log( "%s", pszText );
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

// Log a symbolized call stack from a captured context (needs the .pdb next to the exe).
static void _LogStack( CONTEXT Ctx, int nMaxFrames )
{
	static bool bSymInit = false;
	HANDLE hProcess = GetCurrentProcess();
	if( !bSymInit )
	{
		SymSetOptions( SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );
		SymInitialize( hProcess, NULL, TRUE );
		bSymInit = true;
	}

	STACKFRAME64 Frame;
	memset( &Frame, 0, sizeof(Frame) );
	Frame.AddrPC.Offset = Ctx.Eip;		Frame.AddrPC.Mode = AddrModeFlat;
	Frame.AddrFrame.Offset = Ctx.Ebp;	Frame.AddrFrame.Mode = AddrModeFlat;
	Frame.AddrStack.Offset = Ctx.Esp;	Frame.AddrStack.Mode = AddrModeFlat;

	for( int i = 0; i < nMaxFrames; i++ )
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
}

// Log already captured code addresses, symbolized.
static void _LogAddresses( const DWORD *panAddr, int nAddr )
{
	HANDLE hProcess = GetCurrentProcess();
	static bool bSymInit = false;
	if( !bSymInit )
	{
		SymSetOptions( SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );
		SymInitialize( hProcess, NULL, TRUE );
		bSymInit = true;
	}
	for( int i = 0; i < nAddr; i++ )
	{
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
		const bool bSym = !!SymFromAddr( hProcess, panAddr[i], &nDisp64, pSym );
		const bool bLine = !!SymGetLineFromAddr64( hProcess, panAddr[i], &nDisp, &Line );
		if( bSym && bLine )	_Log( "    #%d 0x%08x %s  (%s:%lu)\n", i, (unsigned)panAddr[i], pSym->Name, Line.FileName, Line.LineNumber );
		else if( bSym )		_Log( "    #%d 0x%08x %s\n", i, (unsigned)panAddr[i], pSym->Name );
		else				_Log( "    #%d 0x%08x\n", i, (unsigned)panAddr[i] );
	}
}

// -port-diag: when a frame stalls for over 100 ms (MA_PORT_STALL_MS), log where the game thread is. The thread is
// suspended only while its return addresses are copied off its frame-pointer chain (no allocation or
// locks while it is stopped: it might hold them); the symbols are looked up after it resumes. Frame
// pointers are only reliable in the Debug build. The engine stops drawing while its window is inactive,
// so stalls are only sampled while it is active.
extern volatile LONG FVid_nPortSwapTick, FVid_nPortGameThreadId;

static int _CopyFrameChain( DWORD nEip, DWORD nEbp, DWORD *panAddr, int nMax )
{
	int nAddr = 0;
	panAddr[nAddr++] = nEip;
	while( nAddr < nMax && nEbp )
	{
		DWORD nNext = 0, nRet = 0;
		__try
		{
			nNext = ((const DWORD *)nEbp)[0];
			nRet = ((const DWORD *)nEbp)[1];
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			break;
		}
		if( !nRet || nNext <= nEbp ) break;
		panAddr[nAddr++] = nRet;
		nEbp = nNext;
	}
	return nAddr;
}

static DWORD WINAPI _StallWatchdog( void * )
{
	LONG nSampledTick = 0;
	int nReports = 0;
	// MA_PORT_STALL_MS lowers (or raises) the threshold to catch shorter hitches
	char szStallMs[16];
	DWORD nThresholdMs = 100;
	if( GetEnvironmentVariableA( "MA_PORT_STALL_MS", szStallMs, sizeof(szStallMs) ) > 0 && atoi( szStallMs ) >= 20 )
	{
		nThresholdMs = (DWORD)atoi( szStallMs );
	}
	while( nReports < 50 )
	{
		Sleep( 10 );
		const LONG nSwapTick = InterlockedCompareExchange( &FVid_nPortSwapTick, 0, 0 );
		const DWORD nThreadId = (DWORD)InterlockedCompareExchange( &FVid_nPortGameThreadId, 0, 0 );
		if( !nSwapTick || !nThreadId || nSwapTick == nSampledTick ) continue;
		const DWORD nStall = GetTickCount() - (DWORD)nSwapTick;
		if( nStall < nThresholdMs ) continue;
		if( fvid_IsMinimized() ) continue;	// inactive: the engine stops drawing on purpose
		nSampledTick = nSwapTick;

		HANDLE hThread = OpenThread( THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, nThreadId );
		if( !hThread ) continue;
		DWORD anAddr[40];
		int nAddr = 0;
		if( SuspendThread( hThread ) != (DWORD)-1 )
		{
			CONTEXT Ctx;
			memset( &Ctx, 0, sizeof(Ctx) );
			Ctx.ContextFlags = CONTEXT_CONTROL;
			if( GetThreadContext( hThread, &Ctx ) )
			{
				nAddr = _CopyFrameChain( Ctx.Eip, Ctx.Ebp, anAddr, 40 );
			}
			ResumeThread( hThread );
		}
		CloseHandle( hThread );
		if( nAddr )
		{
			_Log( "PORT-STALL the game thread has not finished a frame for %lu ms; it is in:\n", nStall );
			_LogAddresses( anAddr, nAddr );
			nReports++;
		}
	}
	return 0;
}

// Asserts and run-time checks continue after logging, so one firing every frame can flood
// the log (each line is flushed) until the game appears frozen. Each distinct report is
// logged for its first 10 occurrences, with a stack on the first, then at 100, 1000, ...
static SRWLOCK _ReportLock = SRWLOCK_INIT;

// Fang assertions show a modal dialog, so capture the current symbolized stack to the run log
// before displaying it. This makes assertions such as fmath_Sqrt's input check actionable.
extern "C" void port_LogFangAssertionStack( const char *pszFile, int nLine )
{
	AcquireSRWLockExclusive( &_ReportLock );
	static DWORD anLoggedAssertSites[64];
	static unsigned nLoggedAssertSites = 0;
	DWORD nSiteKey = 2166136261u;
	for( const char *psz = pszFile; psz && *psz; ++psz ) nSiteKey = (nSiteKey ^ (unsigned char)*psz) * 16777619u;
	nSiteKey = (nSiteKey ^ (DWORD)nLine) * 16777619u;
	for( unsigned i = 0; i < nLoggedAssertSites; ++i )
	{
		if( anLoggedAssertSites[i] == nSiteKey )
		{
			ReleaseSRWLockExclusive( &_ReportLock );
			return;
		}
	}
	if( nLoggedAssertSites < sizeof(anLoggedAssertSites) / sizeof(anLoggedAssertSites[0]) )
		anLoggedAssertSites[nLoggedAssertSites++] = nSiteKey;
	CONTEXT Ctx;
	RtlCaptureContext( &Ctx );
	_Log( "    (first Fang assertion at %s:%d; stack follows)\n", pszFile ? pszFile : "?", nLine );
	_LogStack( Ctx, 24 );
	_LogFlush();
	ReleaseSRWLockExclusive( &_ReportLock );
}

// -start-at: player.cpp asks for the test start position once player 1 has control.
extern "C" int port_GetStartAt( float *pafXYZYaw )
{
	if( !_bStartAt )
		return 0;
	memcpy( pafXYZYaw, _afStartAt, sizeof(_afStartAt) );
	return 1;
}

// -test-give: player.cpp gives player 1 this item once play begins (NULL = none).
extern "C" const char *port_GetTestGive( void )
{
	return _szTestGive[0] ? _szTestGive : NULL;
}

// -test-win-level: gamepad.cpp completes the level this many seconds into the game loop (0 = off).
extern "C" float port_GetTestWinLevelSecs( void )
{
	return _fTestWinLevelSecs;
}

// Returns how many times this report text has been seen. Call with _ReportLock held.
static unsigned _CountReport( const char *pszKey )
{
	static unsigned anSeen[128], anCount[128];
	static int nNumSeen = 0;
	unsigned nHash = 2166136261u;
	for( const char *psz = pszKey; psz && *psz; ++psz ) nHash = (nHash ^ (unsigned char)*psz) * 16777619u;
	for( int i = 0; i < nNumSeen; ++i ) if( anSeen[i] == nHash ) return ++anCount[i];
	if( nNumSeen < (int)(sizeof(anSeen) / sizeof(anSeen[0])) )
	{
		anSeen[nNumSeen] = nHash;
		anCount[nNumSeen] = 1;
		++nNumSeen;
	}
	return 1;
}

static bool _ShouldLogReport( unsigned nCount )
{
	if( nCount <= 10 ) return true;
	for( unsigned nPow = 100; nPow && nPow <= nCount; nPow = (nPow <= 0xFFFFFFFFu / 10) ? nPow * 10 : 0 )
	{
		if( nCount == nPow ) return true;
	}
	return false;
}

// Logs the repeat note (after the report text) and, on the first occurrence, the stack.
static void _LogReportDetails( unsigned nCount )
{
	if( nCount == 10 ) _Log( "    (10 occurrences; further repeats are logged at 100, 1000, ...)\n" );
	else if( nCount > 10 ) _Log( "    (occurrence %u)\n", nCount );
	if( nCount == 1 )
	{
		CONTEXT Ctx;
		RtlCaptureContext( &Ctx );
		_Log( "    (first occurrence; stack follows)\n" );
		_LogStack( Ctx, 24 );
	}
}

static int __cdecl _RTCErrorHandler( int nErrType, const char *pszFile, int nLine, const char *pszModule, const char *pszFormat, ... )
{
	char szMsg[1024];
	va_list Args;
	va_start( Args, pszFormat );
	_vsnprintf( szMsg, sizeof(szMsg) - 1, pszFormat, Args );
	va_end( Args );
	szMsg[sizeof(szMsg) - 1] = 0;

	AcquireSRWLockExclusive( &_ReportLock );
	const unsigned nCount = _CountReport( szMsg );
	if( _ShouldLogReport( nCount ) )
	{
		_Log( "\n*** RUN-TIME CHECK FAILURE (type %d) at %s:%d [%s]:\n    %s\n", nErrType, pszFile ? pszFile : "?", nLine, pszModule ? pszModule : "?", szMsg );
		_LogReportDetails( nCount );
		_LogFlush();
	}
	ReleaseSRWLockExclusive( &_ReportLock );
	return 0;	// 0 = continue running (like clicking "Ignore"); nonzero = break into a debugger
}

static void __cdecl _PurecallHandler( void )
{
	_Log( "\n*** PURE VIRTUAL FUNCTION CALL (R6025) - calling abort()\n" );
	_LogFlush();
	abort();
}

static void __cdecl _InvalidParameterHandler( const wchar_t *pszExpr, const wchar_t *pszFunc, const wchar_t *pszFile, unsigned int nLine, uintptr_t )
{
	_Log( "\n*** CRT INVALID PARAMETER at %ls:%u in %ls(%ls) - calling abort()\n", pszFile ? pszFile : L"?", nLine, pszFunc ? pszFunc : L"?", pszExpr ? pszExpr : L"?" );
	_LogFlush();
	abort();
}

static void __cdecl _SigAbortHandler( int )
{
	_Log( "\n*** abort() called\n" );
	_LogFlush();
}

static int __cdecl _CrtReportHook( int nReportType, char *pszMessage, int *pnReturnValue )
{
	static const char *const apszType[] = { "WARN", "ERROR", "ASSERT" };

	AcquireSRWLockExclusive( &_ReportLock );
	const unsigned nCount = _CountReport( pszMessage );
	if( _ShouldLogReport( nCount ) )
	{
		_Log( "\n*** CRT %s: %s\n", (nReportType >= 0 && nReportType <= 2) ? apszType[nReportType] : "REPORT", pszMessage ? pszMessage : "(no message)" );
		_LogReportDetails( nCount );
	}
	ReleaseSRWLockExclusive( &_ReportLock );

	_LogFlush();
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

	CONTEXT Ctx = *pEx->ContextRecord;
	if( pRec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && pRec->NumberParameters >= 2 &&
		pRec->ExceptionInformation[0] == 8 && Ctx.Eip == pRec->ExceptionInformation[1] &&
		!IsBadReadPtr( (const void *)Ctx.Esp, sizeof(DWORD) ) )
	{
		// A call through a bad pointer: the caller's return address is on top of the stack.
		_Log( "    (bad call target; unwinding from the return address)\n" );
		Ctx.Eip = *(const DWORD *)Ctx.Esp;
		Ctx.Esp += sizeof(DWORD);
	}
	_LogStack( Ctx, 40 );

	fflush( stdout );
	_LogFlush();
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
	_Log( "Usage: ma_port [-data <dir>] [-mst <file>] [-res WxH] [-fullscreen] [-no-audio] [-console] [-port-diag] [-discord-app-id <id> [-discord-large-image <asset-key-or-url>] [-discord-large-text <tooltip>]] [-debug-info] [-dev-menu] [-level <world-resource> | -mission <world-resource> [-coop 2-4] | -world-only <world-resource> | -export-character-meshes <list-file>] [-log <file>] [-asset-log <file>] [-instance-label <name>] [-shots <dir> [-shot-every <frames>]] [-mouse-sensitivity <n>] [-aim-assist auto|on|off] [-input-layout shared|separate] [-button-prompts auto|keyboard|xbox|playstation] [-save-dir <dir>] [-start-at X,Y,Z[,YAW]] [-sfx-db <dB>] [-player-sfx-db <dB>]\n" );
}

static bool _DataDirHasMaster( const char *pszDataDir )
{
	char szMasterPath[MAX_PATH * 3];
	int nLength = snprintf( szMasterPath, sizeof( szMasterPath ), "%s\\%s", pszDataDir, _szMasterName );
	if( nLength <= 0 || nLength >= sizeof( szMasterPath ) )
		return false;
	DWORD dwAttributes = GetFileAttributesA( szMasterPath );
	return dwAttributes != INVALID_FILE_ATTRIBUTES && !( dwAttributes & FILE_ATTRIBUTE_DIRECTORY );
}

static bool _ResolveDataDirFromExecutable( void )
{
	// Explorer starts a clicked EXE with its own directory as the working directory. Walk up
	// from the EXE so the repository's default gamedata/files works from build/Release too.
	char szSearchDir[MAX_PATH];
	DWORD nPathLength = GetModuleFileNameA( NULL, szSearchDir, sizeof( szSearchDir ) );
	if( nPathLength == 0 || nPathLength >= sizeof( szSearchDir ) )
		return false;
	char *pszSlash = strrchr( szSearchDir, '\\' );
	char *pszForwardSlash = strrchr( szSearchDir, '/' );
	if( pszForwardSlash && ( !pszSlash || pszForwardSlash > pszSlash ) )
		pszSlash = pszForwardSlash;
	if( !pszSlash )
		return false;
	*pszSlash = 0;

	const bool bDataDirAbsolute = _szDataDir[0] == '\\' || _szDataDir[0] == '/' ||
		( _szDataDir[0] && _szDataDir[1] == ':' && ( _szDataDir[2] == '\\' || _szDataDir[2] == '/' ) );
	if( bDataDirAbsolute )
		return false;

	for( int nDepth = 0; nDepth < 8; ++nDepth )
	{
		char szCandidate[MAX_PATH * 2];
		int nCandidateLength = snprintf( szCandidate, sizeof( szCandidate ), "%s\\%s", szSearchDir, _szDataDir );
		if( nCandidateLength > 0 && nCandidateLength < sizeof( szCandidate ) &&
			_DataDirHasMaster( szCandidate ) && strlen( szCandidate ) < sizeof( _szDataDir ) )
		{
			strcpy( _szDataDir, szCandidate );
			return true;
		}

		pszSlash = strrchr( szSearchDir, '\\' );
		pszForwardSlash = strrchr( szSearchDir, '/' );
		if( pszForwardSlash && ( !pszSlash || pszForwardSlash > pszSlash ) )
			pszSlash = pszForwardSlash;
		if( !pszSlash || pszSlash <= szSearchDir + 2 )
			break;
		*pszSlash = 0;
	}
	return false;
}

static bool _ParseArgs( int argc, char **argv )
{
	strcpy( _szDataDir, _DEFAULT_DATA_DIR );
	strcpy( _szMasterName, _DEFAULT_MASTER_FILE );
	strcpy( _szLogFile, _DEFAULT_LOG_FILE );
	strcpy( _szAssetLogFile, _DEFAULT_ASSET_LOG_FILE );
	_szStartLevel[0] = 0;
	_szMission[0] = 0;
	_szWorldOnly[0] = 0;
	_szCharacterMeshList[0] = 0;
	_szInstanceLabel[0] = 0;

	for( int i = 1; i < argc; i++ )
	{
		const char *pszArg = argv[i];
		const bool bHasValue = (i + 1 < argc);

		if( !_stricmp( pszArg, "-data" ) && bHasValue ) {
			strncpy( _szDataDir, argv[++i], MAX_PATH - 1 );
		}
		else if( !_stricmp( pszArg, "-mst" ) && bHasValue )			strncpy( _szMasterName, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-log" ) && bHasValue )			strncpy( _szLogFile, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-asset-log" ) && bHasValue )		strncpy( _szAssetLogFile, argv[++i], MAX_PATH - 1 );
		else if( !_stricmp( pszArg, "-level" ) && bHasValue )		strncpy( _szStartLevel, argv[++i], sizeof(_szStartLevel) - 1 );
		else if( !_stricmp( pszArg, "-mission" ) && bHasValue )		strncpy( _szMission, argv[++i], sizeof(_szMission) - 1 );
		else if( !_stricmp( pszArg, "-coop" ) && bHasValue ) {
			char *pEnd = NULL;
			const long nPlayers = strtol( argv[++i], &pEnd, 10 );
			if( !pEnd || *pEnd || nPlayers < 2 || nPlayers > 4 ) {
				_Log( "-coop must be 2, 3, or 4.\n" );
				return false;
			}
			_nCampaignCoopPlayers = (int)nPlayers;
		}
		else if( !_stricmp( pszArg, "-world-only" ) && bHasValue )	strncpy( _szWorldOnly, argv[++i], sizeof(_szWorldOnly) - 1 );
		else if( !_stricmp( pszArg, "-export-character-meshes" ) && bHasValue )	strncpy( _szCharacterMeshList, argv[++i], sizeof(_szCharacterMeshList) - 1 );
		else if( !_stricmp( pszArg, "-instance-label" ) && bHasValue ) strncpy( _szInstanceLabel, argv[++i], sizeof(_szInstanceLabel) - 1 );
		else if( !_stricmp( pszArg, "-fullscreen" ) )				{ _bFullscreen = true; _bBorderless = false; _bModeArg = true; }
		else if( !_stricmp( pszArg, "-borderless" ) )				{ _bFullscreen = false; _bBorderless = true; _bModeArg = true; }
		else if( !_stricmp( pszArg, "-windowed" ) )				{ _bFullscreen = _bBorderless = false; _bModeArg = true; }
		else if( !_stricmp( pszArg, "-no-audio" ) )				_bNoAudio = true;
		else if( !_stricmp( pszArg, "-mute" ) )					_bMute = true;
		else if( !_stricmp( pszArg, "-debug-info" ) )				_bDebugInfo = true;
		else if( !_stricmp( pszArg, "-console" ) )					_bConsole = true;
		else if( !_stricmp( pszArg, "-port-diag" ) )				_bPortDiag = true;
		else if( !_stricmp( pszArg, "-no-vsync" ) )					_bNoVsync = true;
		else if( !_stricmp( pszArg, "-discord-app-id" ) && bHasValue )	strncpy( _szDiscordAppId, argv[++i], sizeof(_szDiscordAppId) - 1 );
		else if( !_stricmp( pszArg, "-discord-large-image" ) && bHasValue ) strncpy( _szDiscordLargeImage, argv[++i], sizeof(_szDiscordLargeImage) - 1 );
		else if( !_stricmp( pszArg, "-discord-large-text" ) && bHasValue ) strncpy( _szDiscordLargeText, argv[++i], sizeof(_szDiscordLargeText) - 1 );
		else if( !_stricmp( pszArg, "-dev-menu" ) )					_bDevMenu = true;
		else if( !_stricmp( pszArg, "-mouse-sensitivity" ) && bHasValue ) {
			char *pEnd;
			const char *pszValue = argv[++i];
			double fValue = strtod( pszValue, &pEnd );
			if( *pEnd || !(fValue >= 0.001 && fValue <= 10.0) ) {
				_Log( "Mouse sensitivity must be between 0.001 and 10 degrees per count.\n" );
				return false;
			}
			SetEnvironmentVariableA( "MA_PORT_MOUSE_SENSITIVITY", pszValue );
		}
		else if( !_stricmp( pszArg, "-aim-assist" ) && bHasValue ) {
			PcAimAssistMode nMode;
			if( !pcinput_ParseAimAssistMode( argv[i + 1], &nMode ) ) {
				_Log( "-aim-assist must be auto (controller only), on, or off.\n" );
				return false;
			}
			SetEnvironmentVariableA( "MA_PORT_AIM_ASSIST", argv[++i] );
		}
		else if( !_stricmp( pszArg, "-input-layout" ) && bHasValue ) {
			PcInputLayout nLayout;
			if( !pcinput_ParseLayout( argv[i + 1], &nLayout ) ) {
				_Log( "-input-layout must be shared or separate.\n" );
				return false;
			}
			SetEnvironmentVariableA( "MA_PORT_INPUT_LAYOUT", argv[++i] );
		}
		else if( !_stricmp( pszArg, "-button-prompts" ) && bHasValue ) {
			PcPromptStyle nStyle;
			if( !pcinput_ParsePromptStyle( argv[i + 1], &nStyle ) ) {
				_Log( "-button-prompts must be auto, keyboard, xbox, or playstation.\n" );
				return false;
			}
			SetEnvironmentVariableA( "MA_PORT_BUTTON_PROMPTS", argv[++i] );
		}
		else if( !_stricmp( pszArg, "-test-keys" ) && bHasValue )	SetEnvironmentVariableA( "MA_PORT_TEST_KEYS", argv[++i] );	// read by pc_input.cpp
		else if( !_stricmp( pszArg, "-shots" ) && bHasValue )		SetEnvironmentVariableA( "MA_PORT_SHOTS", argv[++i] );	// read by compat/d3d8_compat.cpp
		else if( !_stricmp( pszArg, "-shot-every" ) && bHasValue )	SetEnvironmentVariableA( "MA_PORT_SHOT_EVERY", argv[++i] );
		else if( !_stricmp( pszArg, "-save-dir" ) && bHasValue )	SetEnvironmentVariableA( "MA_PORT_SAVE_DIR", argv[++i] );	// read by Fang2/dx/fdx8storage.cpp
		else if( !_stricmp( pszArg, "-player-sfx-db" ) && bHasValue )	SetEnvironmentVariableA( "MA_PORT_PLAYER_SFX_DB", argv[++i] );	// read by Fang2/dx/fdx8audio.cpp
		else if( !_stricmp( pszArg, "-sfx-db" ) && bHasValue )			SetEnvironmentVariableA( "MA_PORT_SFX_DB", argv[++i] );		// read by Fang2/dx/fdx8audio.cpp
		else if( !_stricmp( pszArg, "-aniso" ) && bHasValue )			SetEnvironmentVariableA( "MA_PORT_ANISO", argv[++i] );		// read by compat/d3d8_compat.cpp
		else if( !_stricmp( pszArg, "-test-win-level" ) && bHasValue )	_fTestWinLevelSecs = (float)atof( argv[++i] );				// read by gamepad.cpp
		else if( !_stricmp( pszArg, "-test-give" ) && bHasValue )		strncpy( _szTestGive, argv[++i], sizeof(_szTestGive) - 1 );	// read by player.cpp
		else if( !_stricmp( pszArg, "-coop-hold" ) && bHasValue )	SetEnvironmentVariableA( "MA_PORT_COOP_HOLD", _stricmp( argv[++i], "off" ) ? "1" : "0" );	// read by entity.cpp
		else if( !_stricmp( pszArg, "-start-at" ) && bHasValue )
		{
			_afStartAt[3] = 0.0f;
			if( sscanf( argv[++i], "%f,%f,%f,%f", &_afStartAt[0], &_afStartAt[1], &_afStartAt[2], &_afStartAt[3] ) < 3 )
			{
				_Log( "Bad -start-at value '%s' (expected X,Y,Z or X,Y,Z,YAW)\n", argv[i] );
				return false;
			}
			_bStartAt = true;
		}
		else if( !_stricmp( pszArg, "-res" ) && bHasValue )
		{
			if( sscanf( argv[++i], "%dx%d", &_nReqWidth, &_nReqHeight ) != 2 || _nReqWidth < 320 || _nReqHeight < 200 )
			{
				_Log( "Bad -res value '%s' (expected e.g. 1280x960)\n", argv[i] );
				return false;
			}
			_bResArg = true;
		}
		else
		{
			_Log( "Unknown or incomplete option: %s\n", pszArg );
			_Usage();
			return false;
		}
	}
	if( (!!_szStartLevel[0] + !!_szWorldOnly[0] + !!_szCharacterMeshList[0] + !!_szMission[0]) > 1 )
	{
		_Log( "Choose only one of -level, -mission, -world-only, or -export-character-meshes.\n" );
		_Usage();
		return false;
	}
	if( _nCampaignCoopPlayers > 1 ) {
		if( !_szMission[0] ) {
			_Log( "-coop requires -mission <registered campaign world>.\n" );
			return false;
		}
	}
	if( !_DataDirHasMaster( _szDataDir ) && !_ResolveDataDirFromExecutable() )
	{
		char szMessage[MAX_PATH + 256];
		snprintf( szMessage, sizeof( szMessage ), "Could not find '%s' in the game data folder:\n\n%s\n\n"
			"Use -data to point to the folder containing the master file.", _szMasterName, _szDataDir );
		MessageBoxA( NULL, szMessage, "Metal Arms PC Port: Game Data Not Found", MB_OK | MB_ICONERROR );
		return false;
	}
	_szDiscordAppId[sizeof(_szDiscordAppId) - 1] = 0;
	_szDiscordLargeImage[sizeof(_szDiscordLargeImage) - 1] = 0;
	_szDiscordLargeText[sizeof(_szDiscordLargeText) - 1] = 0;

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
			if( !bWindowed && pMode->nColorBits == 32 )
			{
				pcdisplay_AddAvailableResolution( (int)pMode->nPixelsAcross, (int)pMode->nPixelsDown );
			}
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

	// Render at real pixels on scaled (high-DPI) desktops instead of being stretched by Windows.
	SetProcessDPIAware();

	if( !_ParseArgs( argc, argv ) )
	{
		return 2;
	}
	fmovie2_SetAudioEnabled( !_bNoAudio && !_bMute );
	FAudio_bPortMuteOutput = _bMute ? TRUE : FALSE;

	// The exe is a windowed (GUI) app, so players get no console full of engine and script output.
	// -console opens one; output redirected by the parent (as the test tools do) still arrives.
	if( _bConsole && AllocConsole() )
	{
		freopen( "CONOUT$", "w", stdout );
		setvbuf( stdout, NULL, _IONBF, 0 );
	}
	char szDiag[8];
	Fang_bPortDiag = _bPortDiag || ( GetEnvironmentVariableA( "MA_PORT_DIAG", szDiag, sizeof(szDiag) ) > 0 && szDiag[0] == '1' );
	if( Fang_bPortDiag )
	{
		CloseHandle( CreateThread( NULL, 0, _StallWatchdog, NULL, 0, NULL ) );
	}
	if( !_szDiscordAppId[0] )
	{
		GetEnvironmentVariableA( "MA_PORT_DISCORD_APP_ID", _szDiscordAppId, sizeof(_szDiscordAppId) );
	}
	if( !_szDiscordLargeImage[0] )
	{
		GetEnvironmentVariableA( "MA_PORT_DISCORD_LARGE_IMAGE", _szDiscordLargeImage, sizeof(_szDiscordLargeImage) );
	}
	if( !_szDiscordLargeText[0] )
	{
		GetEnvironmentVariableA( "MA_PORT_DISCORD_LARGE_TEXT", _szDiscordLargeText, sizeof(_szDiscordLargeText) );
	}
	if( !_szDiscordAppId[0] )
	{
		strcpy( _szDiscordAppId, _szDefaultDiscordAppId );
	}
	if( !_stricmp( _szDiscordAppId, "off" ) || !_stricmp( _szDiscordAppId, "none" ) || !strcmp( _szDiscordAppId, "0" ) )
	{
		_szDiscordAppId[0] = 0;
	}
	if( _szDiscordAppId[0] )
	{
		discord_SetLog( _DiscordLog );
		if( discord_Start( _szDiscordAppId, _szDiscordLargeImage, _szDiscordLargeText ) )
		{
			discord_SetActivity( "Starting up", "", true );
		}
		else
		{
			_Log( "Discord: '%s' is not a Discord application ID; Rich Presence is off.\n", _szDiscordAppId );
		}
	}

	_pLog = fopen( _szLogFile, "w" );
	_hLogThread = CreateThread( NULL, 0, _LogWriter, NULL, 0, NULL );
	atexit( _LogFlush );	// early error returns and exit() still write the last lines

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
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 8192000;	// the DX maximum: two 1 MB checkpoint streams plus data streaming
	Fang_ConfigDefs.nWorld_MaxIntersects = 3500;
	Fang_ConfigDefs.bCheckPoint_StartupSystem = TRUE;
	Fang_ConfigDefs.nText_MaxCharsPerFrame = 550;
	Fang_ConfigDefs.nText_MaxCharsPerPrintf = 190;
	Fang_ConfigDefs.nMaxParticleEmitters = 500;
	Fang_ConfigDefs.nMaxParticles = 3000;
	Fang_ConfigDefs.nMaxParticleEmitterSprites = 1000;
	// fdraw streams primitives into one dynamic VB. More space means fewer DISCARD locks
	// when 2D effects wrap it within a frame; the Fang default is only 2,048 vertices.
	Fang_ConfigDefs.nDraw_D3DVBVertexCount = 8192;
	_Log( "Dynamic draw buffer: %u vertices.\n", Fang_ConfigDefs.nDraw_D3DVBVertexCount );
	Fang_nLaunchType = FANG_LAUNCH_TYPE_STANDALONE;

	if( !fang_Startup() )
	{
		_Log( "Fang engine failed to start.\n" );
		return 1;
	}

	// Development builds default to drawing debug overlays (Gameloop_bDrawDebugInfo). On screen the
	// script monitors cover gameplay and a script error pauses the game until Jump is pressed, so
	// they are opt-in; scripts log messages and errors either way.
	Gameloop_bDrawDebugInfo = _bDebugInfo;
	Launcher_bShowDevBootMenu = _bDevMenu;

	if( !ffile_LogSetFilename( _szAssetLogFile ) )
	{
		_Log( "Could not create the Fang resource log.\n" );
		fang_Shutdown();
		return 1;
	}

	//////////////////////////////////////////////////////////////////////
	// Game loop parameters
	memset( &_GameInitParms, 0, sizeof(_GameInitParms) );

	_GameInitParms.fTargetFPS = GAMELOOP_DEFAULT_TARGET_FPS;
	_GameInitParms.bSkipLevelSelect = _szStartLevel[0] != 0 || _szMission[0] != 0 || _szCharacterMeshList[0] != 0;
	_GameInitParms.bLoadRegisteredMission = _szMission[0] != 0;
	_GameInitParms.nQuickLaunchCampaignPlayers = (u8)_nCampaignCoopPlayers;
	_GameInitParms.bPlayerDeath = TRUE;					// Match the original launcher default; campaign actors must be able to die.
	_GameInitParms.nAnimPlaybackRate = GAMELOOP_DEFAULT_ANIM_PLAYBACK;
	_GameInitParms.bViewBounds = GAMELOOP_DEFAULT_VIEW_BOUNDS;
	_GameInitParms.bShowFPS = FALSE;
	_GameInitParms.bDrawScreenSafeArea = FALSE;
	_GameInitParms.nPlatform = GAMELOOP_PLATFORM_GC;			// the retail data set is the GameCube one
	_GameInitParms.nMaxSoundMgrSounds = GAMELOOP_DEFAULT_MAX_SOUNDS;
	_GameInitParms.pszInputFilename = _szMission[0] ? _szMission : (_szStartLevel[0] ? _szStartLevel :
		(_szCharacterMeshList[0] ? _szCharacterMeshList : NULL));
	_GameInitParms.pszScreenShotDir = GAMELOOP_DEFAULT_SCREENSHOT_DIR;
	_GameInitParms.BGColorRGB.Black();
	_GameInitParms.pExitFunc = _GameloopExit;
	_GameInitParms.pMinFunc = _GameloopMinimize;
	_GameInitParms.nMemCardUsageFlags = GAMELOOP_MEMCARD_NONE;
	_GameInitParms.pszMemCardDir = NULL;
	_GameInitParms.pauInputEmulationMap = NULL;
	_GameInitParms.pszInputEmulationDevName = NULL;
	_GameInitParms.bInstallAudio = !_bNoAudio;
	_GameInitParms.bLoadWorldOnly = _szWorldOnly[0] != 0;
	_GameInitParms.bExportCharacterMeshes = _szCharacterMeshList[0] != 0;
	_GameInitParms.pszCharacterMeshList = _GameInitParms.bExportCharacterMeshes ? _szCharacterMeshList : NULL;
	if( _GameInitParms.bLoadWorldOnly )
	{
		_GameInitParms.bSkipLevelSelect = TRUE;
		_GameInitParms.pszInputFilename = _szWorldOnly;
	}
	_GameInitParms.bGovernFrameRate = FALSE;
	_GameInitParms.bDemoLaunched = FALSE;
	_GameInitParms.uTimeoutInterval = 0;

	// Display mode and resolution come from settings.ini [Display] unless the command line chose them.
	// Fullscreen without a saved resolution, and borderless always, use the desktop's.
	pcdisplay_Load();
	if( !_bModeArg )
	{
		_bFullscreen = pcdisplay_Mode() == PCDISPLAY_FULLSCREEN;
		_bBorderless = pcdisplay_Mode() == PCDISPLAY_BORDERLESS;
	}
	const int nDesktopWidth = GetSystemMetrics( SM_CXSCREEN ), nDesktopHeight = GetSystemMetrics( SM_CYSCREEN );
	if( _bBorderless )
	{
		_nReqWidth = nDesktopWidth;
		_nReqHeight = nDesktopHeight;
	}
	else if( !_bResArg )
	{
		int nSavedWidth, nSavedHeight;
		pcdisplay_Resolution( &nSavedWidth, &nSavedHeight );
		if( nSavedWidth && nSavedHeight )
		{
			_nReqWidth = nSavedWidth;
			_nReqHeight = nSavedHeight;
		}
		else if( _bFullscreen )
		{
			_nReqWidth = nDesktopWidth;
			_nReqHeight = nDesktopHeight;
		}
	}

	FVidWin_t &Win = _GameInitParms.VidWin;
	memset( &Win, 0, sizeof(Win) );
	if( !_PickVideoMode( &Win ) )
	{
		fang_Shutdown();
		return 1;
	}
	FDX8Vid_bPortBorderless = _bBorderless ? TRUE : FALSE;
	pcdisplay_SetLaunched( _bFullscreen ? PCDISPLAY_FULLSCREEN : (_bBorderless ? PCDISPLAY_BORDERLESS : PCDISPLAY_WINDOWED),
		(int)Win.VidMode.nPixelsAcross, (int)Win.VidMode.nPixelsDown );
	if( _bBorderless )
	{
		_Log( "Video: borderless window covering the %dx%d desktop\n", nDesktopWidth, nDesktopHeight );
	}
	Win.nSwapInterval = _bNoVsync ? 0 : 1;						// vsync unless -no-vsync
	Win.fUnitFSAA = 0.0f;
	Win.hInstance = GetModuleHandle( NULL );
	Win.hWnd = 0;
	Win.nIconIDI = IDI_MA_PORT;	// port/res/ma_port.rc
	Win.bAllowPowerSuspend = TRUE;
	Win.pFcnSuspend = NULL;
	// Identify test windows so concurrent instances can be monitored without confusing them with the
	// user's game session, and label whether each test has audio enabled.
	if( _szInstanceLabel[0] )
	{
		if( _bNoAudio || _bMute )
			snprintf( Win.szWindowTitle, sizeof( Win.szWindowTitle ), "Metal Arms: %.29s [TEST RUN - %s]", _szInstanceLabel, _bMute ? "MUTED" : "NO AUDIO" );
		else
			snprintf( Win.szWindowTitle, sizeof( Win.szWindowTitle ), "Metal Arms: %.29s [TEST RUN - AUDIO]", _szInstanceLabel );
	}
	else
		strcpy( Win.szWindowTitle, _bNoAudio ? "Metal Arms: Glitch in the System  [TEST RUN - NO AUDIO]" :
			( _bMute ? "Metal Arms: Glitch in the System  [TEST RUN - MUTED]" : "Metal Arms: Glitch in the System" ) );

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
	discord_Stop();
	fang_Shutdown();

	InterlockedExchange( &_nLogQuit, 1 );
	if( _hLogThread )
	{
		WaitForSingleObject( _hLogThread, 2000 );
		CloseHandle( _hLogThread );
		_hLogThread = NULL;
	}
	_LogFlush();
	if( _pLog ) fclose( _pLog );
	return 0;
}
