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

#include <xtl.h>
#include <stdio.h>

void __cdecl main() {
	fang_Init();
	if( !fang_Startup() ) {
		DEVPRINTF( "fang_Startup() failed.\n" );
		for(;;);
	}

	fresload_SetPath( "d:\\art\\" );

//**********************************************************************************************************************

char szDebugText[ 2048 ];
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

u32 uIndex;

//finput_SetForceFeedback( oHandle[ 0 ], 1, 0 );

while( 1 )
{
	sprintf( szDebugText, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\r\n" );
	OutputDebugString( szDebugText );

	for( uDevIndex = 0; uDevIndex < uDev; ++uDevIndex )
	{
		FASSERT( FINPUT_NO_ERROR == finput_GetCurrentControlState( oHandle[ uDevIndex ], &oState[ uDevIndex ] ) );
	}

	for( uDevIndex = 0; uDevIndex < uDev; ++uDevIndex )
	{
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

	Sleep( 1000 );
}

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


