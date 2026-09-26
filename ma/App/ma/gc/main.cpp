//////////////////////////////////////////////////////////////////////////////////////
// main.cpp - GameCube main entry function.
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
// 05/17/02 Lafleur       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include <dolphin.h>

#include "fang.h"
#include "fclib.h"
#include "../gameloop.h"
#include "floop.h"
#include "fgc.h"


#if FANG_DEBUG_BUILD
	#define _FANG_HEAP_MB				20.f
#elif FANG_PRODUCTION_BUILD
	#define _FANG_HEAP_MB				17.65f
#else
	#define _FANG_HEAP_MB				19.0f
#endif
#define _MASTER_FILE_PATHNAME			"mettlearms_gc.mst"
#define _MASTER_MOVIE_PATH				"movies"

#define _MB_SIZE						(1024 * 1024)

#define _SHOW_THREAD_AND_MEMORY_STATUS	(TRUE & FANG_ENABLE_DEV_FEATURES)


static GameloopInitParm_t _GameInitParms;


static void _BootGameCube( void );
static void* GCmain( void *pParam );


OSThread	Thread;
u8			ThreadStack[64 * 1024]; //RAF -- increased from 16K to 64K of thread space to support Bink Movies

static		u32 _nFramesSuspensionTests;
static		u32 _nFramesSuspendedCount;


//
//
//
void main( void )
{
	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = (u32)(_FANG_HEAP_MB * _MB_SIZE);
	Fang_ConfigDefs.pszFile_MasterFilePathName = _MASTER_FILE_PATHNAME;
	Fang_ConfigDefs.pszMovie_BasePathName = _MASTER_MOVIE_PATH;
	Fang_ConfigDefs.nAudio_MaxSoundBytes = 10 * _MB_SIZE;
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 5 * _MB_SIZE;
	Fang_ConfigDefs.bCheckPoint_StartupSystem = TRUE;
#if !FANG_PRODUCTION_BUILD
	Fang_ConfigDefs.nText_MaxCharsPerFrame = 550;
	Fang_ConfigDefs.nText_MaxCharsPerPrintf = 190;
#endif

	if ( !fang_Startup() ) 
	{
		// Trouble starting Fang...
		return;
	}

	#if _SHOW_THREAD_AND_MEMORY_STATUS
		OSReport( "main() - Creating game thread.\n" );
	#endif
	
	// Clear the frame suspension counters
	_nFramesSuspensionTests = 0;
	_nFramesSuspendedCount = 0;
	
	#if _SHOW_THREAD_AND_MEMORY_STATUS
		s32 nBytesFree = OSCheckHeap( FGC_HeapHandle );
		OSReport( "main() - Free Memory before thread creation main(): %d\n", nBytesFree);
	#endif
	
	if ( !OSCreateThread(
		&Thread,							// ptr to the thread to init
		GCmain,								// ptr to the start routine
		NULL,								// param passed to start routine
		ThreadStack + sizeof ThreadStack,	// initial stack address
		sizeof ThreadStack,					// stack size
		31,									// scheduling priority
		OS_THREAD_ATTR_DETACH ) )			// detached by default
	{
		#if _SHOW_THREAD_AND_MEMORY_STATUS
			OSReport( "main() - Failed to create game thread.\n" );
		#endif
		_BootGameCube();    
	}

	//
	// Starts the thread
	//
	#if _SHOW_THREAD_AND_MEMORY_STATUS
		OSReport( "main() - Entering game thread.\n" );
	#endif
	OSResumeThread( &Thread );
	
	do
	{
		#if _SHOW_THREAD_AND_MEMORY_STATUS
			_nFramesSuspensionTests++;
			if ( OSIsThreadSuspended( &Thread ) )
			{
				_nFramesSuspendedCount++;
			
				if ( _nFramesSuspendedCount % 60 == 0 )
				{
					OSReport( "main() - Game Thread is suspended.\n" );
					OSReport( "         Thread suspended on %3.2f percent of vsyncs.\n", ((f32)_nFramesSuspendedCount / (f32)_nFramesSuspensionTests) * 100.f );
				}
//	    		GXBool bOverHigh, bReadIdle, bCmdIdle, bBrkpt, bUnderLow = FALSE;
//	    		while ( !bUnderLow )
//	    		{
//					GXGetGPStatus( &bOverHigh, &bUnderLow, &bReadIdle, &bCmdIdle, &bBrkpt );
//				}
//				OSResumeThread( &Thread );
			}
				
		#endif
		
		// Sleep the thread until next Vsync
		VIWaitForRetrace();
		
	} while (!OSIsThreadTerminated(&Thread));
/*    
	while( !OSIsThreadTerminated(&Thread) );
	{
		nSum++;
		OSReport( "Out of the Game Thread %d.\n", nSum );
	};
*/
	#if _SHOW_THREAD_AND_MEMORY_STATUS
		OSReport( "main() - Game Thread exited.\n" );
	#endif
	
	_BootGameCube();    
}


//
//
//
static void* GCmain( void *pParam )
{
	#pragma unused( pParam )
	
	GXSetCurrentGXThread();
	
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
//	_GameInitParms.pszInputFilename = "RampTest2";
	_GameInitParms.pszInputFilename = "DoorAI_test";
	_GameInitParms.bSkipLevelSelect = FALSE;

	_GameInitParms.VidWin.VidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	_GameInitParms.VidWin.VidDev.nOrdinal = 0;
	_GameInitParms.VidWin.VidDev.nRenderer = FVID_RENDERER_HARDWARE;
	fclib_strcpy( _GameInitParms.VidWin.VidDev.szName, "Nintendo GameCube" );
	_GameInitParms.VidWin.VidMode.nColorBits = 32;
	_GameInitParms.VidWin.VidMode.nDepthBits = 24;
	_GameInitParms.VidWin.VidMode.nStencilBits = 8;
	_GameInitParms.VidWin.VidMode.nPixelsAcross = 640;
	_GameInitParms.VidWin.VidMode.nPixelsDown = 480;
	_GameInitParms.VidWin.VidMode.nFlags = FVID_MODEFLAG_NONE;
	_GameInitParms.VidWin.fUnitFSAA = 0.0f;
	_GameInitParms.VidWin.nSwapInterval = 0;
	_GameInitParms.bInstallAudio = TRUE;
	_GameInitParms.bLoadWorldOnly = FALSE;
	_GameInitParms.bDemoLaunched = FALSE;
	_GameInitParms.uTimeoutInterval = 0;

	if ( !gameloop_Start( &_GameInitParms ) )
	{
		// Gameloop could not be started...
		#if _SHOW_THREAD_AND_MEMORY_STATUS
			OSReport( "main() - Game loop failed to start.\n" );
		#endif
		return NULL;
	}

	// Gameloop has exited...
	return NULL;
}


//
//
//
static void _BootGameCube( void ) 
{
	DEVPRINTF( "MAIN.CPP - Failed to enter game loop.  Rebooting.\n" );
	#if _SHOW_THREAD_AND_MEMORY_STATUS
		OSReport( "MAIN.CPP - Rebooting.\n" );
	#endif
	
	switch( Gameloop_ExitDestination ) {

	case GAMELOOP_EXIT_DESTINATION_NONE:
		OSResetSystem( OS_RESET_RESTART, 0, FALSE );
		break;
		
	case GAMELOOP_EXIT_DESTINATION_MEMORYSYSTEM:
		OSResetSystem( OS_RESET_HOTRESET, 0, TRUE );
		break;
	}

	for(;;){}
}

