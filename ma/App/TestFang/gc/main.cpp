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
#include "fdatastreaming.h"

// Test includes
#include "..\CommonMain.h"


//
//
//
BOOL GCLoopInit( void *pParameter )
{
	FVidWin_t WindowParams;
	fang_MemSet( &WindowParams, 0, sizeof( FVidWin_t ) );
	WindowParams.nSwapInterval = 0;
	WindowParams.fUnitFSAA = 0.f;
	WindowParams.VidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	WindowParams.VidDev.nOrdinal = 0;
	WindowParams.VidDev.nRenderer = FVID_RENDERER_HARDWARE;
	fclib_strcpy( WindowParams.VidDev.szName, "Nintendo GameCube" );
	WindowParams.VidMode.nColorBits = 32;
	WindowParams.VidMode.nDepthBits = 24;
	WindowParams.VidMode.nStencilBits = 8;
	WindowParams.VidMode.nPixelsAcross = 640;
	WindowParams.VidMode.nPixelsDown = 480;
	WindowParams.VidMode.nFlags = FVID_MODEFLAG_NONE;
	WindowParams.fUnitFSAA = 0.0f;
	WindowParams.nSwapInterval = 0;
	fvid_CreateWindow( &WindowParams );

	g_pViewport = fviewport_Create();
	fviewport_InitPersp( g_pViewport, FMATH_DEG2RAD( 45.f ), 0.1f, 1000.f );
	fviewport_SetActive( g_pViewport );

	fpadio_Install();
	fpad_Install();	
	ftext_Install();

	return LoopInit( pParameter );
}


//
//
//
void main( void )
{
	fang_Init();
	
	FDS_StreamMgr.SetCacheSize( 2048000 );
	
	Fang_ConfigDefs.pszFile_MasterFilePathName = "MettleArms_gc.mst";
	Fang_ConfigDefs.nRes_HeapBytes = 17 * 1024 * 1024;
	Fang_ConfigDefs.nAudio_MaxSoundBytes = 8192000;
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 5.5 * (1024 * 1024);
	fang_Startup();

	floop_InstallGameloop( GCLoopInit, LoopMain, LoopTerm, NULL, 60, 0 );
}


