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
#include "fpsprite.h"
#include "fanim.h"
#include "dx/fdx8mesh.h"
#include "fshadow.h"

#include <xtl.h>
#include <xgraphics.h>


static FViewport_t *_pViewport;
static CFXfm _ViewXfm;
static CFXfm _ModelXfm;
static f32 _fRadiansY;
static FMesh_t *_pMesh;

static CFWorldMesh *_pWorldMeshDino;
static CFAnimInst *_pAnimInst;
static CFAnimCombiner *_pAnimCombiner;
static FMeshShadow_t _MeshShadow;
static FViewport_t *_pShadowViewport;

static CFWorldPSGroup *_pPSGroup;
static FPSprite_t *_pPS;

static FTexDef_t *_pPSTexDef;



static BOOL _InitFcn( void *pParameter );
static BOOL _MainLoop( BOOL bExitRequest, void *pParameter );
static void _TermFcn( FLoopTermCode_t nTermCode, void *pParameter );


void __cdecl main() {
	fang_Init();
	if( !fang_Startup() ) {
		DEVPRINTF( "fang_Startup() failed.\n" );
		for(;;);
	}

	fresload_SetPath( "d:\\art\\" );

	floop_InstallGameloop( _InitFcn, _MainLoop, _TermFcn, NULL, 60.0f );
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
	VidWin.nSwapInterval = 1;
	VidWin.fUnitFSAA = 0.4f;
	fvid_CreateWindow( &VidWin );

//	floop_EnableGovernor( TRUE );

	_pViewport = fviewport_Create();

	_fRadiansY = 0.0f;

#if 1
//	_pPSTexDef = (FTexDef_t *)fresload_Load( "TXTRparticle" );
	{
		void *pTexture;
		FTexInfo_t *pTexInfo;
		void *pSrcImage, *pDestBuf;
		FTexInfo_t TexInfo;

		pTexture = fresload_LoadRawIntoSysHeap( "TXTRparticle" );
		pTexInfo = (FTexInfo_t *)pTexture;
		pSrcImage = (void *)(pTexInfo + 1);

		pDestBuf = fang_Malloc( 64*64, 0 );

		if( S_OK != XGCompressRect(
						pDestBuf,
						D3DFMT_DXT5,
						(64/4 * 16),
						64,
						64,
						pSrcImage,
						D3DFMT_LIN_A8R8G8B8,
						64 * 4,
						0.0f,		// fAlphaRef ????????
						XGCOMPRESS_NEEDALPHA0 | XGCOMPRESS_NEEDALPHA1
				) ) {
			return FALSE;
		}

		strcpy( TexInfo.szName, "Part" );
		TexInfo.pUserData = NULL;
		TexInfo.nTexFmt = FTEX_FMT_DXT5;
		TexInfo.nPalFmt = FTEX_PALFMT_NONE;
		TexInfo.nFlags = 0;
		TexInfo.nLodCount = 1;
		TexInfo.nTexelsAcross = 64;
		TexInfo.nTexelsDown = 64;

		_pPSTexDef = ftex_CreateTexture( &TexInfo, pDestBuf, NULL );
	}
#endif

	if( !fresload_Load( FWORLD_RESTYPE, "clearing" ) ) {
		return FALSE;
	}

#if 1
	CFVec3 LightDir( -1.0f, -0.8f, -0.5f );

	_pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, "dino" );
	if( _pMesh == NULL ) {
		return FALSE;
	}

	FMeshInit_t MeshInit;
	MeshInit.fCullDist2 = 0.0f;
	MeshInit.Mtx.Identity();
//	MeshInit.Mtx.SetYRotation( 50.0f );
	MeshInit.Mtx.m_vPos.x = 0.0f;
	MeshInit.Mtx.m_vPos.y = 2.0f;
	MeshInit.Mtx.m_vPos.z = -1.0f;
	MeshInit.nFlags = 0;
	MeshInit.pMesh = _pMesh;

	_pWorldMeshDino = new CFWorldMesh( TRUE );
	_pWorldMeshDino->Init( &MeshInit );
	_pWorldMeshDino->m_AmbientRGB.Set( 0.35f );
	_pWorldMeshDino->m_DirLight.m_Dir_WS = LightDir;
	_pWorldMeshDino->m_DirLight.m_fIntensity = 1.0f;
	_pWorldMeshDino->m_DirLight.m_Motif.OpaqueWhite();

	FAnim_t *pAnim = (FAnim_t *)fresload_Load( FANIM_RESNAME, "spino" );
	_pAnimInst = new CFAnimInst;
	_pAnimInst->Create( pAnim );
	_pAnimCombiner = new CFAnimCombiner;
	_pAnimCombiner->CreateSimple( _pAnimInst, _pWorldMeshDino );

	_MeshShadow.LightUnitDir = LightDir.Unit();
	_MeshShadow.fNearPlaneZ = 0.1f;
	_MeshShadow.fFarPlaneZ = 25.0f;
	_MeshShadow.fFullStrengthZ = 20.0f;
	_MeshShadow.pTexDef = ftex_CreateRenderTarget( 512, 512, 8, "Shadow", FALSE, TRUE );
	_MeshShadow.fUnitStrength = 0.4f;

	_pShadowViewport = fviewport_Create();
	fviewport_InitOrtho2D( _pShadowViewport, 0.1f, 1000.0f );
#endif

#if 1
	//xxxxxxxxxxxxx
	_pPS = (FPSprite_t *)fang_Malloc( 100*sizeof(FPSprite_t), 16 );

	_pPSGroup = new CFWorldPSGroup( TRUE );

	_pPSGroup->m_Xfm.BuildTranslation( 0.0f, 0.0f, 0.0f );
	_pPSGroup->m_BoundSphere_MS.Set( CFVec3( 0.0f, 0.0f, 0.0f ), 10.0f );
	_pPSGroup->m_fCullDist2 = FMATH_MAX_FLOAT;
	_pPSGroup->m_nPSFlags = 0;
	_pPSGroup->m_nPSpriteCount = 100;
	_pPSGroup->m_pPSArray = _pPS;
	_pPSGroup->m_TexInst.pTexDef = _pPSTexDef;
	_pPSGroup->m_nBlend = FPSPRITE_BLEND_MODULATE;
	for( i=0; i<100; i++ ) {
		_pPS[i].ColorRGBA.RandomRGBA();
		_pPS[i].fDim_MS = (fmath_RandomFloat()*0.2f + 0.2f);
		_pPS[i].Point_MS.Set( 5.0f*(2.0f*fmath_RandomFloat()-1.0f), 5.0f*(2.0f*fmath_RandomFloat()-1.0f), 5.0f*(2.0f*fmath_RandomFloat()-1.0f) );
	}
	_pPSGroup->UpdateTracker();
#endif

	return TRUE;
}

static BOOL _MainLoop( BOOL bExitRequest, void *pParameter ) {
	FLight_t Light;

	_pAnimInst->DeltaTime( FLoop_fPreviousLoopSecs );
	_pAnimCombiner->ComputeMtxPalette();

//	_MeshShadow.LightUnitDir.RotateY( 1.0f );

	fvid_Begin();

	frenderer_Fog_Enable( TRUE );
	frenderer_Fog_SetRange( 10.0f, 200.0f );
	frenderer_Fog_SetMotif( &FColor_MotifWhite );

	fviewport_InitPersp( _pViewport, FMATH_QUARTER_PI, 0.1f, 5000.0f );
	fviewport_SetActive( _pViewport );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	CFXfm XfmRotY, XfmRotX, XfmScale, XfmXlat, Xfm;

	_fRadiansY += 0.01f;
#if 0
//	XfmRotY.BuildRotationY( _fRadiansY, 0.0f, 0.0f, 0.0f );
	XfmRotY.BuildRotationY( 0.0f, 0.0f, 0.0f, 0.0f );
	XfmRotX.BuildRotationX( 0.2f*FMATH_PI, 0.0f, 0.0f, 0.0f );
//	XfmXlat.BuildTranslation( 0.0f, fmath_Sin( (f32)FVid_nFrameCounter * 0.01f )*100.0f + 100.0f, 0.0f );
	XfmXlat.BuildTranslation( -10.0f, 20.0f, -20.0f );
	XfmScale.BuildScale( 1.0f );
#endif

#if 1
	_pPSGroup->m_Xfm.BuildTranslation( fmath_Sin( (f32)FVid_nFrameCounter * 0.01f )*25.0f + 25.0f, 0.0f, fmath_Cos( (f32)FVid_nFrameCounter * 0.01f )*25.0f + 25.0f );
	_pPSGroup->UpdateTracker();
#endif

	#define _CAMRADIUS	10.0f
	CFVec3 LookAtPos = _pWorldMeshDino->m_Xfm.m_MtxF.m_vPos;
	CFVec3 CamPos = LookAtPos + CFVec3( _CAMRADIUS * fmath_Sin( _fRadiansY ), 0.0f, _CAMRADIUS * fmath_Cos( _fRadiansY ) );
CamPos = LookAtPos + CFVec3( 8.0f * fmath_Sin( 0.2f ), 10.0f, 8.0f * fmath_Cos( 0.2f ) );
	_ViewXfm.BuildLookat( CamPos, LookAtPos, CFVec3(0.0f, 1.0f, 0.0f) );
	_ViewXfm.InitStackWithView();

#if 0
	CFVec3 TempVec = CamPos;
	TempVec.x = -TempVec.x;
	TempVec.z = -TempVec.z;
	_MeshShadow.LightDir = LookAtPos - TempVec;
#endif

	frenderer_Push( FRENDERER_SHADOW, NULL );
	fshadow_CreateShadow( _pWorldMeshDino, &_MeshShadow );
	frenderer_Pop();

	FWorldNode_t *pWorldNode;
	pWorldNode = fworld_GetNodeIntersectingPoint( FWorld_pWorld->nLeafLevelNum, &FXfm_pView->m_MtxR.m_vPos );
	fworld_Draw( pWorldNode );

#if 0
	FWorld_pWorld->fFogStartZ = 1.0f;
	FWorld_pWorld->fFogEndZ = 50.0f;
	FWorld_pWorld->FogMotif.OpaqueWhite();
	frenderer_Fog_Enable( TRUE );
	frenderer_Fog_SetRange( 1.0f, 50.0f );
	frenderer_Fog_SetMotif( &FColor_MotifWhite );
#endif

	frenderer_Push( FRENDERER_SHADOW, NULL );
	fshadow_RenderShadowOntoInstance( fworld_GetMeshInst(), &_MeshShadow, FVIEWPORT_PLANESMASK_ALL );
	frenderer_Pop();

#if 1
	frenderer_Push( FRENDERER_DRAW, NULL );
	{
		#define __DIM		100.0f
		#define _OFFSETX	25.0f
		#define _OFFSETY	30.0f
		FDrawVtx_t aVtx[4];
		FViewport_t *pPrevViewport;

		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
		fdraw_SetTexture( _MeshShadow.pTexDef );
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );

		aVtx[0].ColorRGBA.OpaqueWhite();
		aVtx[0].Pos_MS.Set( _OFFSETX, _OFFSETY + __DIM, 1.0f );
		aVtx[0].ST.Set( 0.0f, 1.0f );

		aVtx[1].ColorRGBA.OpaqueWhite();
		aVtx[1].Pos_MS.Set( _OFFSETX, _OFFSETY, 1.0f );
		aVtx[1].ST.Set( 0.0f, 0.0f );

		aVtx[2].ColorRGBA.OpaqueWhite();
		aVtx[2].Pos_MS.Set( _OFFSETX + __DIM, _OFFSETY + __DIM, 1.0f );
		aVtx[2].ST.Set( 1.0f, 1.0f );

		aVtx[3].ColorRGBA.OpaqueWhite();
		aVtx[3].Pos_MS.Set( _OFFSETX + __DIM, _OFFSETY, 1.0f );
		aVtx[3].ST.Set( 1.0f, 0.0f );

		pPrevViewport = fviewport_GetActive();
		fviewport_SetActive( _pShadowViewport );
		CFXfm::InitStack();
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
		fviewport_SetActive( pPrevViewport );
	}
	frenderer_Pop();
#endif

	fvid_End();
	fvid_Swap();

	return TRUE;
}

static void _TermFcn( FLoopTermCode_t nTermCode, void *pParameter ) {
}


