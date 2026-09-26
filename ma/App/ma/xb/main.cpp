//////////////////////////////////////////////////////////////////////////////////////
// main.cpp - Xbox main entry function.
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
// 02/16/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "dx/fdx8.h"
#include "fclib.h"
#include "floop.h"
#include "../gameloop.h"
#include "xtl.h"
#include "xbox.h"
#include "sas_user.h"
#include "launcher.h"

#if FANG_DEBUG_BUILD
	#include "XbDm.h"
#endif

#if FANG_DEBUG_BUILD
	#define _FANG_HEAP_MB			24.5f
#else
	#define _FANG_HEAP_MB			23.5f
#endif
#define _DEFAULT_GAME_ROOT		"D:\\"
#define _MASTER_FILE_PATHNAME	"d:\\mettlearms_xb.mst"
#define _MASTER_MOVIE_PATH		"d:\\MOVIES\\"


static GameloopInitParm_t _GameInitParms;

static void _BootXbox( void );

static char _GameRootPathName[128];
static char _MasterFilePathName[128];
static char _MoviePathName[128];


int main( int argc, char *argv[] ) {
	LD_DEMO *pLauncherData = NULL;

	fang_Init();

	// Retrieve the data sent to us by the launcher, if any...
	XGetLaunchInfo( &FDX8_uLaunchType, &FDX8_LaunchData );

	if ( FDX8_uLaunchType == LDT_TITLE )
	{
		pLauncherData = (LD_DEMO *)FDX8_LaunchData.Data;
		fclib_strcpy( FDX8_szXBEName, pLauncherData->szLaunchedXBE );
		s32 nLen = fclib_strlen( _GameRootPathName );
		while ( nLen >= 0 )
		{
			if ( _GameRootPathName[nLen] == '\\' )
			{
				_GameRootPathName[nLen + 1] = 0;
				break;
			}
			nLen--;
		}

		Fang_nLaunchType = FANG_LAUNCH_TYPE_DEMO;

		fclib_strcpy( _MoviePathName, _GameRootPathName );
		fclib_strcat( _MoviePathName, "MOVIES\\" );
		fclib_strcpy( _MasterFilePathName, _GameRootPathName );
		fclib_strcat( _MasterFilePathName, "mettlearms_xb.mst" );
	}
	else if ( FDX8_uLaunchType == LDT_FROM_DEBUGGER_CMDLINE )
	{
		Fang_nLaunchType = FANG_LAUNCH_TYPE_DEBUGGER;

		#if FANG_DEBUG_BUILD
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_d.xbe" );
		#elif FANG_RELEASE_BUILD
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_r.xbe" );
		#elif FANG_PRODUCTION_BUILD
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_p.xbe" );
		#else
			fclib_strcpy( FDX8_szXBEName, "d:\\default.xbe" );
		#endif
		fclib_strcpy( _GameRootPathName, _DEFAULT_GAME_ROOT );
		fclib_strcpy( _MoviePathName, _MASTER_MOVIE_PATH );
		fclib_strcpy( _MasterFilePathName, _MASTER_FILE_PATHNAME );
	}
	else if ( FDX8_uLaunchType == LDT_FROM_DASHBOARD )
	{
		Fang_nLaunchType = FANG_LAUNCH_TYPE_STANDALONE;

		#if LAUNCHER_E3_SINGLE_PLAYER_VERSION
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_E3Single.xbe" );
		#elif LAUNCHER_E3_MULTI_PLAYER_VERSION
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_E3Multi.xbe" );
		#else
			#if FANG_DEBUG_BUILD
				fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_d.xbe" );
			#elif FANG_RELEASE_BUILD
				fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_r.xbe" );
			#else
				fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_p.xbe" );
			#endif
		#endif
		fclib_strcpy( _GameRootPathName, _DEFAULT_GAME_ROOT );
		fclib_strcpy( _MoviePathName, _MASTER_MOVIE_PATH );
		fclib_strcpy( _MasterFilePathName, _MASTER_FILE_PATHNAME );
	}
	else
	{
		Fang_nLaunchType = FANG_LAUNCH_TYPE_STANDALONE;

		#if LAUNCHER_E3_SINGLE_PLAYER_VERSION
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_E3Single.xbe" );
		#elif LAUNCHER_E3_MULTI_PLAYER_VERSION
			fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_E3Multi.xbe" );
		#else
			#if FANG_DEBUG_BUILD
				fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_d.xbe" );
			#elif FANG_RELEASE_BUILD
				fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_r.xbe" );
			#else
				fclib_strcpy( FDX8_szXBEName, "d:\\ma_xb_p.xbe" );
			#endif
		#endif

		fclib_strcpy( _GameRootPathName, _DEFAULT_GAME_ROOT );
		fclib_strcpy( _MoviePathName, _MASTER_MOVIE_PATH );
		fclib_strcpy( _MasterFilePathName, _MASTER_FILE_PATHNAME );
	}



	Fang_ConfigDefs.nRes_HeapBytes = (u32)((f32)(_FANG_HEAP_MB) * (f32)(1024 * 1024));
	Fang_ConfigDefs.pszFile_GameRootPathName = _GameRootPathName;
	Fang_ConfigDefs.pszFile_MasterFilePathName = _MasterFilePathName;
	Fang_ConfigDefs.pszMovie_BasePathName = _MoviePathName;
	Fang_ConfigDefs.nDraw_XBVBCount = 16;
	Fang_ConfigDefs.nDraw_D3DVBVertexCount = 128;
	Fang_ConfigDefs.nAudio_MaxSoundBytes = 8388608;		// 8MB total WAV memory limit on Xbox?
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 1024000;
	Fang_ConfigDefs.nWorld_MaxIntersects = 3500;
	Fang_ConfigDefs.bCheckPoint_StartupSystem = TRUE;
	Fang_ConfigDefs.bExceptionHandler_StartupSystem = TRUE;

	Fang_ConfigDefs.nMaxParticleEmitters = 500;
	Fang_ConfigDefs.nMaxParticles = 3000;
	Fang_ConfigDefs.nMaxParticleEmitterSprites = 1000;

	#if !FANG_PRODUCTION_BUILD
		Fang_ConfigDefs.nText_MaxCharsPerFrame = 550;
		Fang_ConfigDefs.nText_MaxCharsPerPrintf = 190;
	#endif

	if( !fang_Startup() ) {
		// Trouble starting Fang...
		_BootXbox();
	}

	_GameInitParms.fTargetFPS = GAMELOOP_DEFAULT_TARGET_FPS;
	_GameInitParms.bSkipLevelSelect = GAMELOOP_DEFAULT_SKIP_LEVEL_SELECT;
	_GameInitParms.nAnimPlaybackRate = GAMELOOP_DEFAULT_ANIM_PLAYBACK;
	_GameInitParms.bViewBounds = GAMELOOP_DEFAULT_VIEW_BOUNDS;
	_GameInitParms.bShowFPS = FALSE;
	_GameInitParms.bDrawScreenSafeArea = FALSE;
	_GameInitParms.nPlatform = GAMELOOP_DEFAULT_PLATFORM;
	_GameInitParms.nMaxSoundMgrSounds = GAMELOOP_DEFAULT_MAX_SOUNDS;
	_GameInitParms.pszInputFilename = NULL;
	_GameInitParms.pszScreenShotDir = GAMELOOP_DEFAULT_SCREENSHOT_DIR;
	_GameInitParms.BGColorRGB.Black();
	_GameInitParms.pExitFunc = NULL;
	_GameInitParms.pMinFunc = NULL;
	_GameInitParms.nMemCardUsageFlags = GAMELOOP_DEFAULT_MEMCARD_SLOT;
	_GameInitParms.pszMemCardDir = NULL;
	_GameInitParms.pauInputEmulationMap = NULL;
	_GameInitParms.pszInputEmulationDevName = NULL;
	_GameInitParms.bGovernFrameRate = FALSE;

	_GameInitParms.VidWin.VidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	_GameInitParms.VidWin.VidDev.nOrdinal = 0;
	_GameInitParms.VidWin.VidDev.nRenderer = FVID_RENDERER_HARDWARE;
	fclib_strcpy( _GameInitParms.VidWin.VidDev.szName, "Xbox NV2A" );
	_GameInitParms.VidWin.VidMode.nColorBits = 32;
	_GameInitParms.VidWin.VidMode.nDepthBits = 24;
	_GameInitParms.VidWin.VidMode.nStencilBits = 8;
	_GameInitParms.VidWin.VidMode.nPixelsAcross = 640;
	_GameInitParms.VidWin.VidMode.nPixelsDown = 480;
	_GameInitParms.VidWin.VidMode.nFlags = FVID_MODEFLAG_NONE;
	_GameInitParms.VidWin.fUnitFSAA = 0.0f;
	if ( FDX8_uLaunchType == LDT_TITLE && pLauncherData )
	{
		FASSERT( pLauncherData );
		_GameInitParms.bDemoLaunched = TRUE;
		_GameInitParms.uTimeoutInterval = pLauncherData->dwTimeout;
	}
	else
	{
		_GameInitParms.bDemoLaunched = FALSE;
		_GameInitParms.uTimeoutInterval = 0;
	}

	#if FANG_PRODUCTION_BUILD
		#if 0
			_GameInitParms.VidWin.nSwapInterval = 1;
		#else
			_GameInitParms.VidWin.nSwapInterval = 0;
			_GameInitParms.bGovernFrameRate = TRUE;
		#endif
	#else
		#if SAS_ACTIVE_USER == SAS_USER_CHRIS
			_GameInitParms.VidWin.nSwapInterval = 1;

		#elif SAS_ACTIVE_USER == SAS_USER_STEVE
			_GameInitParms.VidWin.nSwapInterval = 0;
			_GameInitParms.bGovernFrameRate = TRUE;

		#else
			_GameInitParms.VidWin.nSwapInterval = 0;

		#endif
	#endif

	_GameInitParms.bInstallAudio = TRUE;
	_GameInitParms.bLoadWorldOnly = FALSE;

extern long _fexception_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );
extern long _fexception_Production_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );

	if( Fang_nLaunchType == FANG_LAUNCH_TYPE_STANDALONE ) {
		// only run exception handler from a standalone launch
		__try {
			if( !gameloop_Start( &_GameInitParms ) ) {
				// Gameloop could not be started...
				fang_Shutdown();
				_BootXbox();
			}
		}
		#if( !FANG_ENABLE_DEV_FEATURES )
			__except( _fexception_Production_Handler( GetExceptionInformation() ) ) {
		#else
			__except( _fexception_Handler( GetExceptionInformation() ) ) {
		#endif
			// exception handling actually performed in filter statement of __except above
		}
	} else {
		// non-standalone launch, don't run exception handler
		if( !gameloop_Start( &_GameInitParms ) ) {
			// Gameloop could not be started...
			fang_Shutdown();
			_BootXbox();
		}
	}

	// Gameloop has exited...

	fang_Shutdown();
	_BootXbox();

	return 0;
}

static void _BootXbox( void ) {
//	#if FANG_DEBUG_BUILD
//		DmReboot( DMBOOT_WARM );
//	#else
	if( Gameloop_ExitDestination == GAMELOOP_EXIT_DESTINATION_NONE ) {
		if ( FDX8_uLaunchType == LDT_TITLE )
		{
			LD_DEMO *pLauncherData = (LD_DEMO *)FDX8_LaunchData.Data;
			if ( pLauncherData )
			{
				XLaunchNewImage( pLauncherData->szLauncherXBE, &FDX8_LaunchData );
			}
			else
			{
				XLaunchNewImage( NULL, NULL );
			}
		}
		else
		{
			XLaunchNewImage( NULL, NULL );
		}
//	#endif
	} else if( Gameloop_ExitDestination == GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM ) {

		// launch to the xbox memory portion of the dashboard
		LD_LAUNCH_DASHBOARD launchDB;
		fang_MemZero( &launchDB, sizeof( LD_LAUNCH_DASHBOARD ) );
		launchDB.dwReason = XLD_LAUNCH_DASHBOARD_MEMORY;
		XLaunchNewImage( NULL, (LAUNCH_DATA*)&launchDB );
	} else {
		FASSERT_NOW;
	}

	for(;;);
}

