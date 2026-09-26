//////////////////////////////////////////////////////////////////////////////////////
// xbtest.cpp - 
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 05/04/01 ayale       Modified for Xbox input test.
// 04/25/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "floop.h"
#include "ftimer.h"
#include "frenderer.h"
#include "fvid.h"
#include "fdraw.h"
#include "fviewport.h"
#include "fmath.h"
#include "fxfm.h"
#include "fresload.h"
#include "fmesh.h"
#include "ftex.h"
#include "fworld.h"
#include "finput.h"

#include <xtl.h>
#include "dsound.h"
#include <stdio.h>
#include <io.h>
#include "fsound.h"

static FViewport_t *_pViewport;
static CFXfm _ViewXfm;
static CFXfm _ModelXfm;
static f32 _fRadiansY;
static FMesh_t *_pMesh;
static CFMeshInst *_pMeshInst;
static FTexDef_t *_pRenderTarget;

static BOOL _InitFcn( void *pParameter );
static BOOL _MainLoop( BOOL bExitRequest, void *pParameter );
static void _TermFcn( FLoopTermCode_t nTermCode, void *pParameter );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e _fsound_Update( void );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//**********************************************************************************************************************
// START ***************************************************************************************************************
//**********************************************************************************************************************

#define _WAV_FILENAME1		"d:\\beefcake.wav"
#define _WAV_FILENAME2		"d:\\kickass.wav"
#define _WAV_FILENAME3		"d:\\song.wav"
#define _WAV_FILENAME4		"d:\\sonofabitch.wav"
#define _WAV_FILENAME5		"d:\\screwyou.wav"
#define _WAV_FILENAME6		"d:\\1.wav"
#define _WAV_FILENAME7		"d:\\2.wav"
#define _WAV_FILENAME8		"d:\\3.wav"

#define _MAX_SIMULTANEOUS_SOUNDS	5

typedef struct
{
	char szName[ FSOUND_MAX_UNIQUE_SOUND_NAME_LEN ];
	u32 uSize;
	void *pAddress;

} FSoundWavInfo_t;

typedef struct
{
	bool			bPressed;
	FSoundID_t		oSoundID;
	FSoundCookie_t	oCookieID;

} Button_t;

typedef struct
{
	FSoundCookie_t	oID;
	u32				uTimer;

} Cookie_t;

FSoundWavInfo_t g_pSounds[ FSOUND_MAX_UNIQUE_SOUNDS ];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LoadSounds( char *pszSounds[] )
{
	FILE *nFileHandle;
	u32 uCurrentSound = 0;
	char *pszName, *pszExt;

	if( ! pszSounds ) exit(0);

	while( pszSounds[ uCurrentSound ] && ( FSOUND_MAX_UNIQUE_SOUNDS > uCurrentSound ) )
	{
		nFileHandle = fopen( pszSounds[ uCurrentSound ], "rb" );
		if( ! nFileHandle )
		{
			fang_DevPrintf( "*** [ FSOUND ] Error %d: Can't open %s !!!\r\n", __LINE__, pszSounds[ uCurrentSound ] );
			exit(0);
		}

		g_pSounds[ uCurrentSound ].uSize = filelength( fileno( nFileHandle ) );

		if( 0 >= g_pSounds[ uCurrentSound ].uSize )
		{
			fang_DevPrintf( "*** [ FSOUND ] Error %d: Invalid file size for %s !!!\r\n", __LINE__, pszSounds[ uCurrentSound ] );
			exit(0);
		}

		g_pSounds[ uCurrentSound ].pAddress = (void *)malloc( g_pSounds[ uCurrentSound ].uSize );
		if( g_pSounds[ uCurrentSound ].pAddress == NULL )
		{
			fang_DevPrintf( "*** [ FSOUND ] Error %d: Can't allocate memory for %s !!!\r\n", __LINE__, pszSounds[ uCurrentSound ] );
			exit(0);
		}

		if( fread( g_pSounds[ uCurrentSound ].pAddress, 1, g_pSounds[ uCurrentSound ].uSize, nFileHandle ) < g_pSounds[ uCurrentSound ].uSize )
		{
			fang_DevPrintf( "*** [ FSOUND ] Error %d: Couldn't read entire file: %s !!!\r\n", __LINE__, pszSounds[ uCurrentSound ] );
			exit(0);
		}

		pszName = (char *)( 1 + strrchr( pszSounds[ uCurrentSound ], '\\' ) );
		pszExt = (char *)( strrchr( pszSounds[ uCurrentSound ], '.' ) );

		if( FSOUND_MAX_UNIQUE_SOUND_NAME_LEN > ( pszExt - pszName ) )
		{
			strncpy( g_pSounds[ uCurrentSound ].szName, pszName, ( pszExt - pszName ) );
			fang_DevPrintf("\r\n%s -> %s\r\n", pszSounds[ uCurrentSound ], g_pSounds[ uCurrentSound ].szName);
		}
		else
		{
			fang_DevPrintf("\r\n\r\nNAME TOO LONG!!!!!!!! -> %s\r\n", pszSounds[ uCurrentSound ]);
		}

		fclose( nFileHandle );

		++uCurrentSound;
	}
}

//**********************************************************************************************************************
// END *****************************************************************************************************************
//**********************************************************************************************************************

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void __cdecl main() {
	fang_Init();
	if( !fang_Startup() ) {
		DEVPRINTF( "fang_Startup() failed.\n" );
		for(;;);
	}

	fresload_SetPath( "d:\\art\\" );

//**********************************************************************************************************************
// START ***************************************************************************************************************
//**********************************************************************************************************************

	char szDebugText[ 2048 ];

	u32 uTimer = 0, uTimerTmp;
//	int index1, index2, iIndex;
	int iIndex;
	Button_t oButtons[ _MAX_SIMULTANEOUS_SOUNDS ];
//	FSoundFrame_t oFrames[ _MAX_SIMULTANEOUS_SOUNDS ];
	Cookie_t oCookies[ 46 ];
	FSoundCookie_t oCookieTmp;

	char *pszSounds[]  = { _WAV_FILENAME1, _WAV_FILENAME2, _WAV_FILENAME3, _WAV_FILENAME4, _WAV_FILENAME5, 0 };

	for( iIndex = 0; iIndex < ( sizeof( oCookies ) / sizeof( Cookie_t ) ); ++iIndex )
	{
		oCookies[ iIndex ].oID = -1;
	}

	LoadSounds( pszSounds );

	fang_DevPrintf("\r\n");

	for( iIndex = 0; iIndex < ( ( sizeof( pszSounds ) / sizeof( char * ) ) - 1 ); ++iIndex )
	{
		fang_DevPrintf( "LOADED: %s\r\n", g_pSounds[ iIndex ].szName );
	}

	fang_DevPrintf("\r\n");

	fsound_ModuleStartup();

//	oFrames[ 0 ] = fsound_FrameGetIndex();

// 	oButtons[ 0 ].oSoundID = fsound_SoundLoad( g_pSounds[ 0 ].szName, g_pSounds[ 0 ].pAddress, g_pSounds[ 0 ].uSize, NULL, true );
 	oButtons[ 0 ].oSoundID = fsound_SoundLoad( g_pSounds[ 0 ].szName, g_pSounds[ 0 ].pAddress, g_pSounds[ 0 ].uSize, NULL );
	oButtons[ 1 ].oSoundID = fsound_SoundLoad( g_pSounds[ 1 ].szName, g_pSounds[ 1 ].pAddress, g_pSounds[ 1 ].uSize, NULL );
	oButtons[ 2 ].oSoundID = fsound_SoundLoad( g_pSounds[ 2 ].szName, g_pSounds[ 2 ].pAddress, g_pSounds[ 2 ].uSize, NULL );
	oButtons[ 3 ].oSoundID = fsound_SoundLoad( g_pSounds[ 3 ].szName, g_pSounds[ 3 ].pAddress, g_pSounds[ 3 ].uSize, NULL );
	oButtons[ 4 ].oSoundID = fsound_SoundLoad( g_pSounds[ 4 ].szName, g_pSounds[ 4 ].pAddress, g_pSounds[ 4 ].uSize, NULL );
	oButtons[ 0 ].bPressed = false;
	oButtons[ 1 ].bPressed = false;
	oButtons[ 2 ].bPressed = false;
	oButtons[ 3 ].bPressed = false;
	oButtons[ 4 ].bPressed = false;

//**********************************************************************************************************************
// END *****************************************************************************************************************
//**********************************************************************************************************************

OutputDebugString( "\r\n");

FASSERT( TRUE == finput_ModuleStartup() );

FInputInit_t oInit;
/*
oInit.nSamplesPerSec = 0;
oInit.nHwnd = 0;
oInit.nHInstance = 0;
*/

FASSERT( FINPUT_NO_ERROR == finput_Install( &oInit ) );

u32 uDev, uDevIndex, uLoopCount = 0;

while( FINPUT_NO_ERROR != finput_EnumDevices( &uDev ) )
{
	++uLoopCount;
//	Sleep( 17 );
	Sleep( 100 );
}

sprintf( szDebugText, "Number of loops taken to init: %d\r\n", uLoopCount );
OutputDebugString( szDebugText );

sprintf( szDebugText, "Number of devices plugged in: %d.\r\n", uDev );
OutputDebugString( szDebugText );

const FInputDeviceInfo_t *oDevInfo;

FInputDeviceHandle_t oHandle[ 2 ];
FInputControlState_t oState[ 2 ];

for( uDevIndex = 0; uDevIndex < uDev; ++uDevIndex )
{
	FASSERT( FINPUT_NO_ERROR == finput_GetDeviceDescription( &oDevInfo, uDevIndex ) );

	sprintf( szDebugText, "%d) %s.\r\n", uDevIndex, oDevInfo->pszDescription );
	OutputDebugString( szDebugText );

	FASSERT( FINPUT_NO_ERROR == finput_AttachDevice( oDevInfo, &oHandle[ uDevIndex ], NULL, NULL ) );
}

while( 1 )
{
/*
	sprintf( szDebugText, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\r\n" );
	OutputDebugString( szDebugText );
*/
	for( uDevIndex = 0; uDevIndex < uDev; ++uDevIndex )
	{
		FASSERT( FINPUT_NO_ERROR == finput_GetCurrentControlState( oHandle[ uDevIndex ], &oState[ uDevIndex ] ) );
	}
/*
	for( uDevIndex = 0; uDevIndex < uDev; ++uDevIndex )
	{
		u32 uIndex;

		for( uIndex = 0; uIndex < oState[ uDevIndex ].nNumValidDigitalButtons; ++uIndex )
		{
			sprintf( szDebugText, "%d  ", oState[ uDevIndex ].abDigitalButtons[ uIndex ] );
			OutputDebugString( szDebugText );
		}

		for( uIndex = 0; uIndex < oState[ uDevIndex ].nNumValidAnalogButtons; ++uIndex )
		{
			sprintf( szDebugText, "%f  ", oState[ uDevIndex ].afUnitAnalogButtons[ uIndex ] );
			OutputDebugString( szDebugText );
		}

		for( uIndex = 0; uIndex < oState[ uDevIndex ].nNumValidAxis; ++uIndex )
		{
			sprintf( szDebugText, "%f  ", oState[ uDevIndex ].afAxis[ uIndex ] );
			OutputDebugString( szDebugText );
		}

		sprintf( szDebugText, "\r\n" );
		OutputDebugString( szDebugText );
	}
*/
//**********************************************************************************************************************
// START ***************************************************************************************************************
//**********************************************************************************************************************

for( u32 i = 0; i < _MAX_SIMULTANEOUS_SOUNDS; ++i )
{
	if( oState[ 0 ].afUnitAnalogButtons[ i ] )
	{
		if( oButtons[ i ].bPressed )
		{
			if( -1 != oButtons[ i ].oCookieID )
			{
				fsound_CookieSetVolume( oButtons[ i ].oCookieID, (u8)( ( FSOUND_MAX_VOLUME >> 1 ) + ( ( FSOUND_MAX_VOLUME >> 1 ) * oState[ 0 ].afAxis[1] ) ) );
//				fsound_CookieSetPan( oButtons[ i ].oCookieID, ( FSOUND_PAN_MIDDLE + ( FSOUND_PAN_MIDDLE * oState[ 0 ].afAxis[0] ) ) );
				fsound_CookieSetFrequency( oButtons[ i ].oCookieID, (u16)( 8000.0f + ( 8000.0f * oState[ 0 ].afAxis[ 3 ] ) ) );
			}
		}
		else
		{
/*
											( FSOUND_PAN_MIDDLE + ( FSOUND_PAN_MIDDLE * ControlState.afAxis[0] ) ),
*/
			oCookieTmp = fsound_CookiePlay(	oButtons[ i ].oSoundID,
											(u8)( ( FSOUND_MAX_VOLUME >> 1 ) + ( ( FSOUND_MAX_VOLUME >> 1 ) * oState[ 0 ].afAxis[1] ) ),
											FSOUND_PAN_MIDDLE,
											(u16)( 8000.0f + ( 8000.0f * oState[ 0 ].afAxis[ 3 ] ) ),
											0 );

			if( -1 == oCookieTmp )
			{
				u32 uClearCookie;

				uTimerTmp = -1;
				for( iIndex = 0; iIndex < ( sizeof( oCookies ) / sizeof( Cookie_t ) ); ++iIndex )
				{
					// find a cookie to stop.
					if( -1 == oCookies[ iIndex ].oID ) continue;

					if( uTimerTmp >= oCookies[ iIndex ].uTimer )
					{
						uTimerTmp  = oCookies[ iIndex ].uTimer;
						oCookieTmp = oCookies[ iIndex ].oID;
						uClearCookie = iIndex;
					}
				}

				if( uTimerTmp != -1 )
				{
					// We found a cookie.
					fang_DevPrintf("!!!!!!!!!!!!!!!!!!!!!!!! STOPPING VOICE: %d\r\n", oCookieTmp );

					if( FSOUND_ERROR == fsound_CookieStopAndRelease( oCookieTmp ) )
					{
						fang_DevPrintf("\t\t*** ERROR: %d *********** UNABLE TO RELEASE !!\r\n", __LINE__ );
						return;
					}
					else
					{
						oCookies[ uClearCookie ].oID = -1;
					}
				}
			}
			else
			{
				for( iIndex = 0; iIndex < ( sizeof( oCookies ) / sizeof( Cookie_t ) ); ++iIndex )
				{
					// find a slot.
					if( -1 == oCookies[ iIndex ].oID ) break;
				}

				if( iIndex != ( sizeof( oCookies ) / sizeof( Cookie_t ) ) )
				{
					// We found a slot.
					fang_DevPrintf("PLAYING VOICE: %d !!!!!!!!!!!!!!!!!!!!!!!!\r\n", oCookieTmp );

					oCookies[ iIndex ].oID              = oCookieTmp;
					oCookies[ iIndex ].uTimer           = uTimer;
					oButtons[ i ].oCookieID             = oCookieTmp;
					oButtons[ i ].bPressed              = true;
				}
			}
		}

		oButtons[ i ].bPressed = true;
	}
	else
	{
		oButtons[ i ].bPressed = false;
	}
}

for( iIndex = 0; iIndex < ( sizeof( oCookies ) / sizeof( Cookie_t ) ); ++iIndex )
{
	if( -1 == oCookies[ iIndex ].oID ) continue;

/*
	if( uTimer == oCookies[ iIndex ].uTimer )
	{
		continue;
	}
*/

	if( false == fsound_CookieIsPlaying( oCookies[ iIndex ].oID ) )
	{
		if( FSOUND_ERROR == fsound_CookieStopAndRelease( oCookies[ iIndex ].oID ) )
		{
			fang_DevPrintf("\t\t*** ERROR: %d *********** UNABLE TO RELEASE !!\r\n", __LINE__ );
			return;
		}
		else
		{
			fang_DevPrintf("\t\t*** ERROR: %d *********** RELEASE SUCCESSFUL(WE HOPE) !!\r\n", __LINE__ );
		}

		for( i=0; i < _MAX_SIMULTANEOUS_SOUNDS; ++i )
		{
			if( oButtons[ i ].oCookieID == oCookies[ iIndex ].oID )
			{
				oButtons[ i ].oCookieID = -1;
				break;
			}
		}

		oCookies[ iIndex ].oID = -1;
	}
}

_fsound_Update();

++uTimer;

//**********************************************************************************************************************
// END *****************************************************************************************************************
//**********************************************************************************************************************

	Sleep( 300 );
}

fsound_ModuleShutdown();

/*
Sleep( 1000 );
finput_Uninstall();
finput_ModuleShutdown();
while( 1 );
*/

//u32 count = 0;
//		FASSERT( FINPUT_NO_ERROR == finput_SetForceFeedback( oHandle[ uDevIndex ], ( ( ( count % 2 ) == 0 ) ? 0.0f : 1.0f ), 0 ) );
//		FASSERT( FINPUT_NO_ERROR == finput_SetForceFeedback( oHandle[ uDevIndex ], ( ( ( count % 2 ) == 0 ) ? 0.0f : 1.0f ), 1 ) );
//	++count;

//OutputDebugString( "\r\n!!!!!!!!!!!!!!!!!!!!!!! done !!!!!!!!!!!!!!!!!!!!!!!\r\n");

//**********************************************************************************************************************
//**********************************************************************************************************************
//**********************************************************************************************************************

	floop_InstallGameloop( _InitFcn, _MainLoop, _TermFcn, NULL, 60.0f );
	floop_EnableGovernor( TRUE );
	
}


static BOOL _InitFcn( void *pParameter ) {
	u32 i, nModeCount;
	const FVidDev_t *pVidDev;
	const FVidMode_t *pVidMode;
	FVidWin_t VidWin;

	fvid_Enumerate( FVID_RENDERER_HARDWARE );

	pVidDev = fvid_GetDeviceInfo( 0 );
	nModeCount = fvid_GetModeCount( 0 );

	for( i=0; i<nModeCount; i++ ) {
		pVidMode = fvid_GetModeInfo( 0, i );

		if( pVidMode->nPixelsAcross==640 && pVidMode->nPixelsDown==480 ) {
			if( pVidMode->nColorBits==32 && pVidMode->nDepthBits==24 && pVidMode->nStencilBits==8 ) {
				break;
			}
		}
	}
	if( i == nModeCount ) {
		DEVPRINTF( "xbtest::_InitFcn(): Could not find video mode.\n" );
		return FALSE;
	}

	VidWin.VidDev = *pVidDev;
	VidWin.VidMode = *pVidMode;
	VidWin.nSwapInterval = 0;
	VidWin.fUnitFSAA = 0.0f;
	fvid_CreateWindow( &VidWin );

	_pViewport = fviewport_Create();

	_fRadiansY = 0.0f;

#if 0
	_pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, "guard2" );
	if( _pMesh == NULL ) {
		return FALSE;
	}

	_pMeshInst = new CFMeshInst;
	_pMeshInst->Init( _pMesh );
#endif

	_pRenderTarget = ftex_CreateRenderTarget( 256, 256, 8, "RenderTarget" );
	if( _pRenderTarget == NULL ) {
		return FALSE;
	}

	if( !fresload_Load( FWORLD_RESTYPE, "clearing" ) ) {
		return FALSE;
	}

	return TRUE;
}

static BOOL _MainLoop( BOOL bExitRequest, void *pParameter ) {
	FLight_t Light;

	fvid_Begin();

#if 1
	fviewport_InitPersp( _pViewport, FMATH_QUARTER_PI, 0.1f, 5000.0f );
	fviewport_SetActive( _pViewport );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

#if 0
	_ViewXfm.BuildTranslation( 0.0f, 0.0f, 0.0f );
	_ViewXfm.InitStackWithView();

	frenderer_Push( FRENDERER_MESH, NULL );
	LightFrame = fmesh_StartBlackFrame();
	fmesh_Ambient_Set( 0.2f, 0.2f, 0.2f, 1.0f );

	flight_InitDirLight( &Light, -1.0f, -1.0f, 0.0f );
	fmesh_AddLight( &Light );

	_fRadiansY += 0.01f;
	_pMeshInst->m_Xfm.BuildRotationY( _fRadiansY, 0.0f, 0.0f, 500.0f );
	_pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );

	fmesh_ReleaseLightFrame( LightFrame );
	frenderer_Pop();
#else

	CFXfm XfmRotY, XfmRotX, XfmXlat, Xfm;

	_fRadiansY += 0.01f;
	XfmRotY.BuildRotationY( _fRadiansY, 0.0f, 0.0f, 0.0f );
	XfmRotX.BuildRotationX( 0.45f*FMATH_PI, 0.0f, 0.0f, 0.0f );
	XfmXlat.BuildTranslation( 0.0f, fmath_Sin( (f32)FVid_nFrameCounter * 0.01f )*100.0f + 100.0f, 0.0f );

	Xfm = XfmXlat * XfmRotY * XfmRotX;
	_ViewXfm = Xfm.Invert();
	_ViewXfm.InitStackWithView();

	FWorldNode_t *pWorldNode;
	pWorldNode = fworld_GetNodeIntersectingPoint( FWorld_pWorld->nLeafLevelNum, &FXfm_pView->m_MtxR.m_vPos );
	fworld_Draw( pWorldNode );
#endif

#else
	fviewport_InitPersp( _pViewport, FMATH_QUARTER_PI, 0.1f, 5000.0f, _pRenderTarget );
	fviewport_SetActive( _pViewport );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	_ViewXfm.BuildTranslation( 0.0f, 0.0f, 0.0f );
	_ViewXfm.InitStackWithView();

	frenderer_Push( FRENDERER_MESH, NULL );
	LightFrame = fmesh_StartBlackFrame();
	fmesh_Ambient_Set( 0.05f, 0.05f, 0.05f, 1.0f );

	flight_InitDirLight( &Light, -1.0f, -1.0f, 0.0f );
	fmesh_AddLight( &Light );

	_fRadiansY += 0.01f;
	_pMeshInst->m_Xfm.BuildRotationY( _fRadiansY, 0.0f, 0.0f, 500.0f );
	_pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );

	fmesh_ReleaseLightFrame( LightFrame );
	frenderer_Pop();

	fviewport_InitPersp( _pViewport, FMATH_QUARTER_PI, 0.1f, 5000.0f );
	fviewport_SetActive( _pViewport );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	frenderer_Push( FRENDERER_DRAW, NULL );
	_ModelXfm.BuildRotationY( -0.0f*_fRadiansY, 0.0f, 0.0f, 100.0f );
	_ModelXfm.PushModel();
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_SetTexture( _pRenderTarget );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );

	FDrawVtx_t v1, v2, v3;
	v1.Pos_MS.Set( -100.0f, -100.0f, 0.0f );
	v2.Pos_MS.Set( 0.0f, 100.0f, 0.0f );
	v3.Pos_MS.Set( 100.0f, -100.0f, 0.0f );
	v1.ColorRGBA.OpaqueWhite();
	v2.ColorRGBA.OpaqueWhite();
	v3.ColorRGBA.OpaqueWhite();
	v1.ST.Set( 0.0f, 1.0f );
	v2.ST.Set( 0.5f, 0.0f );
	v3.ST.Set( 1.0f, 1.0f );
	fdraw_Triangle( &v1, &v2, &v3 );
	_ModelXfm.PopModel();
	frenderer_Pop();
#endif

	fvid_End();
	fvid_Swap();

	return TRUE;
}

static void _TermFcn( FLoopTermCode_t nTermCode, void *pParameter ) {
}
