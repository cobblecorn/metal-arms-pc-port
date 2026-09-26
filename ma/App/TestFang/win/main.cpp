
////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN.CPP
//
//		Windows main file for FANG test app
//
////////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

// Fang includes
#include "fang.h"
#include "ffile.h"
#include "fvid.h"
#include "floop.h"
#include "fclib.h"
#include "fpad.h"
#include "ftext.h"
#include "fviewport.h"
#include "floop.h"

// Test includes
#include "..\CommonMain.h"

volatile BOOL _AppActive = TRUE;


//
//
//
BOOL WinLoopInit( void *pParameter )
{
	FVidWin_t WindowParams;
	fang_MemSet( &WindowParams, 0, sizeof( FVidWin_t ) );
	WindowParams.nSwapInterval = 0;
	WindowParams.fUnitFSAA = 0.f;
	WindowParams.VidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	WindowParams.VidDev.nOrdinal = 0;
	WindowParams.VidDev.nRenderer = FVID_RENDERER_HARDWARE;
	fclib_strcpy( WindowParams.VidDev.szName, "NVIDIA GeForce3" );
	WindowParams.VidMode.nColorBits = 32;
	WindowParams.VidMode.nDepthBits = 24;
	WindowParams.VidMode.nStencilBits = 8;
	WindowParams.VidMode.nPixelsAcross = 800;
	WindowParams.VidMode.nPixelsDown = 600;
	WindowParams.VidMode.nFlags = FVID_MODEFLAG_WINDOWED;
	WindowParams.fUnitFSAA = 0.0f;
	WindowParams.nSwapInterval = 0;
	fvid_CreateWindow( &WindowParams );

	g_pViewport = fviewport_Create();
	fviewport_InitPersp( g_pViewport, FMATH_DEG2RAD( 45.f ), 0.1f, 1000.f );
	fviewport_SetActive( g_pViewport );

//	fpadio_Install();
//	fpad_Install();	
	ftext_Install();

	LoopInit( pParameter );

	return TRUE;
}

//
//
//
BOOL WinLoopMain( BOOL bExitRequest, void *pParameter )
{
	CFVec3 vTranslation;
	f32 fYRotation = 0, fXRotation = 0;
	vTranslation.Zero();

	if ( GetAsyncKeyState( VK_RIGHT ) || GetAsyncKeyState( VK_NUMPAD6 ) )
	{
		vTranslation.x = 20 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( VK_LEFT ) || GetAsyncKeyState( VK_NUMPAD4 ) )
	{
		vTranslation.x = -20 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( VK_UP ) || GetAsyncKeyState( VK_NUMPAD8 ) )
	{
		vTranslation.z = 20 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( VK_DOWN ) || GetAsyncKeyState( VK_NUMPAD2 ) )
	{
		vTranslation.z = -20 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( 'A' ) )
	{
		fYRotation = -1 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( 'D' ) )
	{
		fYRotation = 1 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( 'W' ) )
	{
		vTranslation.y = 20 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( 'S' ) )
	{
		vTranslation.y = -20 * FLoop_fPreviousLoopSecs;
	}
	if ( GetAsyncKeyState( ' ' ) )
	{
		SubmitInput( INPUT_TRIGGER_1 );
	}
	if ( GetAsyncKeyState( VK_F1 ) )
	{
		SubmitInput( INPUT_TRIGGER_2 );
	}
	if ( GetAsyncKeyState( VK_ESCAPE ) || fvid_HasUserClosedWindow() )
	{
		bExitRequest = TRUE;
	}

	UpdateCamera( fYRotation, fXRotation, &vTranslation );

	return LoopMain( bExitRequest, pParameter );
}

//
//
//
void WinLoopTerm( FLoopTermCode_t nTermCode, void *pParameter )
{
	LoopTerm( nTermCode, pParameter );
	_AppActive = FALSE;
}

//
//
//
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int sw )
{
	// We can only have one instance of this running
	if ( hPrevInstance )
	{
		return FALSE;
	}

	fang_Init();
	Fang_ConfigDefs.pszFile_MasterFilePathName = "c:\\Art\\Master\\MettleArms_xb.mst";
	Fang_ConfigDefs.nRes_HeapBytes = 16 * 1024 * 1024; // Use an 16 MB heap
	Fang_ConfigDefs.nAudio_MaxSoundBytes = 8192000;
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 4048000;
	fang_Startup();

	floop_InstallGameloop( WinLoopInit, WinLoopMain, WinLoopTerm, NULL, 60, 0 );

	//*********************
	//*****MAIN MESSAGE LOOP
	//
	_AppActive = TRUE;
	while ( _AppActive )
	{
		Sleep( 0 );
	}

	return 0;
}


