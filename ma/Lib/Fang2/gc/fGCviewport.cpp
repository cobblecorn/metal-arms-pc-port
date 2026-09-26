//////////////////////////////////////////////////////////////////////////////////////
// fGCviewport.cpp - Fang viewport module (GameCube version).
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
// 02/18/02	Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#include "fGC.h"
#include "fGCviewport.h"
#include "fGCtex.h"
#include "fGCvid.h"
#include "fGCtex.h"
#include "fGCxfm.h"
#include "fgcvb.h"

#include "fmath.h"
#include "flinklist.h"

#define _USE_PROJECTION_MATRIX_SCALE		FALSE

// We use a smaller frame buffer for GC to save space for render textures
#define _GAMECUBE_FRAMEBUFFER_WIDTH			(512.f)
#define _GAMECUBE_EXTERNAL_DISPLAY_WIDTH	(640.f)
#define _GAMECUBE_HORIZONTAL_PIXEL_SCALE	(_GAMECUBE_FRAMEBUFFER_WIDTH / _GAMECUBE_EXTERNAL_DISPLAY_WIDTH)

FViewport_t *FViewport_pDefaultOrtho;
f32 FGCViewport_ProjDepth=0;


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;
//static f32 _fD3DMaxVertexW;

//static u32 _nMaxViewports;
//static FGCViewportIS_t *_pViewportMemBlock;
//static FLinkRoot_t _FreePool;
//static FLinkRoot_t _UsedPool;

static FViewport_t *_pCurrentViewport;
static FViewport_t *_pDefaultViewport;

static FTexDef_t *_pCurrentRendTargTexDef;
static FTexDef_t *_pResetRenderTarget;

static Mtx44 _CurrentGCProjMtx;




static void _InitViewport( FViewport_t *pViewport, FViewportType_e nType, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ,
						   u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef );
static FViewport_t *_GetNewViewport( void );
static f32 _BuildPerspMtx( Mtx44 &mxResult, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ );
static f32 _BuildOrthoMtx3D( Mtx44 &mxResult, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ );
static f32 _BuildOrthoMtx2D( Mtx44 &mxResult, f32 fWidth, f32 fHeight, f32 fNearZ, f32 fFarZ );
static void _SetRenderTarget( FTexDef_t *pTexDef );
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );



//
//
//
BOOL fgcviewport_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bWindowCreated = FALSE;

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

//	_nMaxViewports = Fang_ConfigDefs.nViewport_MaxViewports + 1;

//	_pViewportMemBlock = (FGCViewportIS_t *)fang_Malloc( sizeof(FGCViewportIS_t) * _nMaxViewports, 0 );
//	if ( _pViewportMemBlock == NULL ) 
//	{
//		fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );
//		return FALSE;
//	}

//	flinklist_InitRoot( &_FreePool, (s32)FANG_OFFSETOF( FGCViewportIS_t, Link ) );
//	flinklist_InitRoot( &_UsedPool, (s32)FANG_OFFSETOF( FGCViewportIS_t, Link ) );
//	flinklist_InitPool( &_FreePool, _pViewportMemBlock, sizeof(FGCViewportIS_t), _nMaxViewports );

	_pCurrentViewport = NULL;
	_pDefaultViewport = NULL;

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fgcviewport_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

//	fang_Free( _pViewportMemBlock );
	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
FViewport_t* fviewport_Create( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	FViewport_t *pViewport = _GetNewViewport();
	if ( pViewport == NULL )
		return NULL;

	return pViewport;
}

/*
//
//
//
void fviewport_Destroy( FViewport_t *pViewport ) 
{
	FASSERT( _bWindowCreated );

	if ( pViewport ) 
	{
		FGCViewportIS_t *pViewportIS = pViewport->pViewportIS;

		flinklist_Remove( &_UsedPool, pViewportIS );
		flinklist_AddTail( &_FreePool, pViewportIS );

		if( pViewport == _pCurrentViewport ) 
		{
			if( !FGCvid_bResetting ) 
			{
				fviewport_SetActive( NULL );
			}

			_pCurrentViewport = NULL;
		}
	}
}


//
//
//
void fviewport_DestroyAll( void ) 
{
	FGCViewportIS_t *pViewportIS;

	FASSERT( _bWindowCreated );

	while( pViewportIS = (FGCViewportIS_t *)flinklist_GetTail( &_UsedPool ) ) 
	{
		fviewport_Destroy( &pViewportIS->Viewport );
	}
}
*/

//
//
//
void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	if( pViewport ) 
	{
		if( pTexDef == NULL ) 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, 0.0f, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} 
		else 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, 0.0f, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}


//
//
//
void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, 0.0f, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}


//
//
//
void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	if( pViewport ) 
	{
		if( pTexDef == NULL ) 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, fHalfFOVY, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} 
		else 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, fHalfFOVY, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}


//
//
//
void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, fHalfFOVY, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}


//
//
//
void fviewport_InitOrtho3D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	if ( pViewport ) 
	{
		if ( pTexDef == NULL ) 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO3D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} 
		else 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO3D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}


//
//
//
void fviewport_InitOrtho3D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO3D, 0.0f, 0.0f, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}


//
//
//
void fviewport_InitOrtho2D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	if ( pViewport ) 
	{
		if ( pTexDef == NULL ) 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO2D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} 
		else 
		{
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO2D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}


//
//
//
void fviewport_InitOrtho2D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO2D, 0.0f, 0.0f, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}


//
//
//
BOOL fviewport_IsInit( FViewport_t *pViewport ) 
{
	FASSERT( _bWindowCreated );

	if( pViewport ) 
	{
		return !!( pViewport->pViewportIS->nFlags &  FGCVIEWPORT_FLAG_INITIALIZED );
	}

	return FALSE;
}


//
//
//
FViewport_t* fviewport_SetActive( FViewport_t *pViewport ) 
{
	FViewport_t *pPrevViewport;
	CFGCViewportIS *pViewportIS;

	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );
	FASSERT( pViewport==NULL || ( !!( pViewport->pViewportIS->nFlags &  FGCVIEWPORT_FLAG_INITIALIZED ) ) );
	FASSERT( _pDefaultViewport );

	if (FViewport_CallbackFunc) 
	{
		FViewport_CallbackFunc(pViewport);
		return (_pCurrentViewport);
	}

	if ( _pCurrentViewport != _pDefaultViewport ) 
	{
		pPrevViewport = _pCurrentViewport;
	} 
	else 
	{
		pPrevViewport = NULL;
	}

	if ( pViewport == NULL ) 
	{
		pViewport = _pDefaultViewport;
	}

	if ( pViewport!=_pCurrentViewport || pViewport->uViewportKey ) 
	{
		pViewport->uViewportKey = FALSE;

		_pCurrentViewport = pViewport;

		pViewportIS = pViewport->pViewportIS;

		_SetRenderTarget( pViewport->pTexDef );
		
		//RAF -- Kind of a hack, but I want to determine if the viewport being loaded
		//is the size of the internal frame buffer (should rarely happen),
		//then forgo the scale factor...
		f32 fHorizScale = _GAMECUBE_HORIZONTAL_PIXEL_SCALE;
		if( pViewportIS->nFlags & FGCVIEWPORT_FLAG_DONT_APPLY_VIEWPORT_SCALE ) {
			fHorizScale = 1.0f;
		}

		GXSetViewport(	pViewport->ScreenCorners.UpperLeft.x * fHorizScale,			// Left-most screen coordinate, in pixels
						pViewport->nScreenTopY,										// Top-most screen coordinate, in pixels
						pViewport->Res.x * fHorizScale,								// Viewport width, in pixels
						pViewport->nHeight,											// Viewport height, in pixels
						0.f,														// Floating point value for near depth scale.  Normalized range is 0.0f - 1.0f
						1.f );														// Floating point value for far depth scale.  Normalized range is 0.0f - 1.0f
		GXSetScissor(	(u32)(pViewport->ScreenCorners.UpperLeft.x * fHorizScale),	// Left-most screen coordinate, in pixels
						pViewport->nScreenTopY,										// Top-most screen coordinate, in pixels
						(u32)(pViewport->Res.x * fHorizScale),						// Viewport width, in pixels
						pViewport->nHeight);										// Viewport height, in pixels
						
		FASSERT( (u32)(pViewport->ScreenCorners.UpperLeft.x * fHorizScale) + (u32)(pViewport->Res.x * fHorizScale) <= 640 );

		fang_MemCopy( &_CurrentGCProjMtx, &pViewportIS->GCProjMtx, sizeof(CFMtx44A) );
		
		// On GameCube, the GXSetProjection() function call ONLY loads
		// [0][0], [0][2], [1][1], [1][2], [2][2], and [2][3] for
		// perspective matrices.  ALL OTHER PROJECTION MATRIX ENTRIES 
		// ARE IMPLIED!!!
		if ( pViewportIS->nMtxType == FGCVIEWPORT_PROJTYPE_PERSP )
			GXSetProjection( _CurrentGCProjMtx, GX_PERSPECTIVE );
		else
			GXSetProjection( _CurrentGCProjMtx, GX_ORTHOGRAPHIC );

		f32 pm[GX_PROJECTION_SZ];
		GXGetProjectionv(pm);
		FGCViewport_ProjDepth = pm[6];
		
		if ( pViewportIS->nMtxType == FGCVIEWPORT_PROJTYPE_PERSP )
		{
			fsh_LoadProjMtx(pViewport->fHalfFOVX*2.0f, pViewport->fHalfFOVY*2.0f);
		}
	}
	

	return pPrevViewport;
}


//
//
//
FViewport_t* fviewport_GetActive( void ) 
{
	FASSERT( _bWindowCreated );

	if( _pCurrentViewport != _pDefaultViewport ) 
	{
		return _pCurrentViewport;
	} 
	else 
	{
		return NULL;
	}
}

FViewport_t *fviewport_GetActive_Absolute( void )
{
	FASSERT( _bWindowCreated );
	return _pCurrentViewport;
}

//
//
//
void fviewport_Clear( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );
	FASSERT( _pCurrentViewport && ( !!( _pCurrentViewport->pViewportIS->nFlags & FGCVIEWPORT_FLAG_INITIALIZED ) ) );

	fviewport_Clear( FVIEWPORT_CLEARFLAG_COLOR, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
}


//
//
//
void fviewport_Clear( f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );
	FASSERT( _pCurrentViewport && ( !!( _pCurrentViewport->pViewportIS->nFlags & FGCVIEWPORT_FLAG_INITIALIZED ) ) );

	fviewport_Clear( FVIEWPORT_CLEARFLAG_COLOR, fUnitRed, fUnitGreen, fUnitBlue, 1.0f, 0 );
}


//
//
//
void fviewport_Clear( u32 nClearFlag, f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue, f32 fDepthValue, u32 nStencilValue ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );
	FASSERT( _pCurrentViewport && ( !!( _pCurrentViewport->pViewportIS->nFlags & FGCVIEWPORT_FLAG_INITIALIZED ) ) );

	if ( _pCurrentViewport == NULL ) 
		return;

	Mtx44 mxOrtho;
	
	f32 mxProj[GX_PROJECTION_SZ];
	
	// Get current projection matrix for later restoration
	GXGetProjectionv( mxProj );

	// Create an orthogonal matrix to use as the projection matrix  
	MTXOrtho( mxOrtho, 1, -1, -1, 1, 0.f, fDepthValue );
	GXSetProjection( mxOrtho, GX_ORTHOGRAPHIC );
	
	// Set the identity matrix as the model matrix
	fgcxfm_LoadGCPosMatrixImm( FGC_MtxIdentity, GX_PNMTX0 );
	fgcxfm_SetGCCurrentMatrix( GX_PNMTX0 );
	
	// Pixel Processing	
	GXSetBlendMode( GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY );

	// If the requestor wants depth buffer cleared, do it
	if ( nClearFlag & FVIEWPORT_CLEARFLAG_DEPTH ) 
		GXSetZMode( GX_ENABLE, GX_ALWAYS, GX_ENABLE );
	else
		GXSetZMode( GX_DISABLE, GX_ALWAYS, GX_DISABLE );

	// If the requestor wants color buffer cleared, do it
	if ( nClearFlag & FVIEWPORT_CLEARFLAG_COLOR ) 
		GXSetColorUpdate( GX_TRUE );
	else
		GXSetColorUpdate( GX_FALSE );
		
	// Culling mode
	GXSetCullMode( GX_CULL_NONE );
	GXSetAlphaUpdate(TRUE);
	
	fgc_DisableChannels();

	GXSetNumTexGens( 0 );       // # of Tex gens
	GXSetNumTevStages( 1 );     // # of Tev Stage
	GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
	GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );

	// Draw a quad to cover the viewport
	fgc_ClearVtxDesc();
	fgc_SetVtxPosDesc( GX_DIRECT );
	fgc_SetVtxNrmDesc( GX_NONE );
	fgc_SetVtxClrDesc( GX_VA_CLR0, GX_DIRECT );
	fgc_SetVtxClrDesc( GX_VA_CLR1, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX0, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX1, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX2, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX3, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX4, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX5, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX6, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX7, GX_NONE );
	fgc_SetVtxPosFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_F32, 0 );

	f32 fOrthoZFar = -fDepthValue * (1.f - 0.000001f);
	
	// Set clear color
	u32 nColor = 	((u8)(255.f * fUnitRed)   << 24) 	// Red
				  | ((u8)(255.f * fUnitGreen) << 16) 	// Green
				  | ((u8)(255.f * fUnitBlue)  << 8) 	// Blue
				  | 0;									// Alpha
				  
	#define __QUAD_DIMENSIONS	1.f			  
	GXBegin( GX_QUADS, FGCDATA_VARIABLE_VERTEX_FORMAT, 4 );
	{
		GXPosition3f32( __QUAD_DIMENSIONS, __QUAD_DIMENSIONS, fOrthoZFar );
		GXColor1u32( nColor );
		
		GXPosition3f32( -__QUAD_DIMENSIONS, __QUAD_DIMENSIONS, fOrthoZFar );
		GXColor1u32( nColor );
		
		GXPosition3f32( -__QUAD_DIMENSIONS, -__QUAD_DIMENSIONS, fOrthoZFar );
		GXColor1u32( nColor );
		
		GXPosition3f32( __QUAD_DIMENSIONS, -__QUAD_DIMENSIONS, fOrthoZFar );
		GXColor1u32( nColor );
	}
	GXEnd();
	
	#undef __QUAD_DIMENSIONS

	// Restore the original projection matrix
	GXSetProjectionv( mxProj );
	
	// Restore cached states
	GXSetBlendMode( FGC_BlendMode, FGC_SourceFactor, FGC_DestFactor, FGC_BitLogicOp );
	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite );
	GXSetColorUpdate( FGC_bColorBufferUpdate );
	GXSetCullMode( FGC_CullMode );
	GXSetAlphaUpdate(FALSE);
}


//
//
//
void fgcviewport_RestoreState( void ) 
{
	FViewport_t *pCurrentViewport;

	FASSERT( _bWindowCreated );

	pCurrentViewport = _pCurrentViewport;
	_pCurrentViewport = (FViewport_t *)0xffffffff;
	_pCurrentRendTargTexDef = (FTexDef_t *)0xffffffff;
	fviewport_SetActive( pCurrentViewport );
}


//
//
//
const Mtx44* fgcviewport_GetGCMatrix( const FViewport_t *pViewport ) 
{
	FASSERT( _bModuleInitialized );

	if( pViewport ) 
	{
		return &pViewport->pViewportIS->GCProjMtx;
	} 
	else 
	{
		return NULL;
	}
}


//
//
//
f32 fviewport_ComputeOrtho2DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS ) 
{
	FASSERT_NOW;
	return 0.f;
/*
	CFVec3A Point_VS;
	CFMtx44A *pMtx;
	f32 fOOQ;

	FASSERT( _bModuleInitialized );

	pMtx = &pViewport->pViewportIS->D3DProjMtx;
	pCamMtx->MulPoint( Point_VS, *pPoint_WS );

	if( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) {
		pPoint_SS->x = Point_VS.x*pMtx->aa[0][0] + Point_VS.z*pMtx->aa[2][0];
		pPoint_SS->y = Point_VS.y*pMtx->aa[1][1] + Point_VS.z*pMtx->aa[2][1];
		pPoint_SS->z = Point_VS.z*pMtx->aa[2][2] + pMtx->aa[3][2];
		fOOQ = 1.0f / (Point_VS.z * pMtx->aa[2][3]);
	} else {
		pPoint_SS->x = Point_VS.x*pMtx->aa[0][0] + pMtx->aa[3][0];
		pPoint_SS->y = Point_VS.y*pMtx->aa[1][1] + pMtx->aa[3][1];
		pPoint_SS->z = Point_VS.z*pMtx->aa[2][2] + pMtx->aa[3][2];
		fOOQ = 1.0f / pMtx->aa[3][3];
	}

	pPoint_SS->Mul( fOOQ );

	pPoint_SS->x = pViewport->HalfRes.x + pPoint_SS->x*pViewport->HalfRes.x;
	pPoint_SS->y = pViewport->HalfRes.y - pPoint_SS->y*pViewport->HalfRes.y;

	return Point_VS.z;
*/
}


//
//
//
f32 fviewport_ComputeOrtho3DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS ) 
{
	CFVec3A Point_VS;
	CFMtx44A *pMtx;
	f32 fOOQ;

	FASSERT( _bModuleInitialized );

	pMtx = (CFMtx44A *)&pViewport->pViewportIS->GCProjMtx;
	pCamMtx->MulPoint( Point_VS, *pPoint_WS );

	if ( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) 
	{
		pPoint_SS->x = Point_VS.x*pMtx->aa[0][0] + Point_VS.z*pMtx->aa[2][0];
		pPoint_SS->y = Point_VS.y*pMtx->aa[1][1] + Point_VS.z*pMtx->aa[2][1];
		pPoint_SS->z = Point_VS.z*(1 - pMtx->aa[2][2]) + pMtx->aa[2][3];
		fOOQ = Point_VS.z * -pMtx->aa[3][2];
		FASSERT( fOOQ != 0.0f );
		fOOQ = fmath_Inv( fOOQ );
	} 
	else 
	{
		pPoint_SS->x = Point_VS.x*pMtx->aa[0][0] + pMtx->aa[3][0];
		pPoint_SS->y = Point_VS.y*pMtx->aa[1][1] + pMtx->aa[3][1];
		pPoint_SS->z = Point_VS.z*(1 - pMtx->aa[2][2]) + pMtx->aa[2][3];
		fOOQ = 1;//fmath_Inv( pMtx->aa[3][3] );
	}

	pPoint_SS->Mul( fOOQ );

	pPoint_SS->x = pPoint_SS->x*pViewport->HalfRes.x;
	pPoint_SS->y = pPoint_SS->y*pViewport->HalfRes.y;

	return Point_VS.z;
}


//
//
//
f32 fviewport_ComputeUnitOrtho3DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS ) 
{
	CFVec3A Point_VS;
	CFMtx44A *pMtx;
	f32 fOOQ;

	FASSERT( _bModuleInitialized );

	pMtx = (CFMtx44A *)&pViewport->pViewportIS->GCProjMtx;
	pCamMtx->MulPoint( Point_VS, *pPoint_WS );

	if ( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) 
	{
		pPoint_SS->x = Point_VS.x * pMtx->aa[0][0] + Point_VS.z * pMtx->aa[2][0];
		pPoint_SS->y = Point_VS.y * pMtx->aa[1][1] + Point_VS.z * pMtx->aa[2][1];
		pPoint_SS->z = Point_VS.z*(1 - pMtx->aa[2][2]) + pMtx->aa[2][3];
		fOOQ = Point_VS.z * -pMtx->aa[3][2];
		FASSERT( fOOQ != 0.0f );
		fOOQ = fmath_Inv( fOOQ );
	} 
	else 
	{
		pPoint_SS->x = Point_VS.x * pMtx->aa[0][0] + pMtx->aa[3][0];
		pPoint_SS->y = Point_VS.y * pMtx->aa[1][1] + pMtx->aa[3][1];
		pPoint_SS->z = Point_VS.z*(1 - pMtx->aa[2][2]) + pMtx->aa[2][3];
		fOOQ = 1;//fmath_Inv( pMtx->aa[3][3] );
	}

	pPoint_SS->Mul( fOOQ );

	return Point_VS.z;
}


//
// Returns TRUE if the sphere was on screen, false if it was not
//
BOOL fviewport_ComputeScreenPointAndSize_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFSphere *pSphere_WS, CFSphere *pResult_SS ) 
{
	CFVec3 Point_VS;
	CFMtx44A *pMtx;
	f32 fOOQ;

	FASSERT( _bModuleInitialized );

	pMtx = (CFMtx44A *)&pViewport->pViewportIS->GCProjMtx;
	Point_VS = pCamMtx->m44.MultPoint( pSphere_WS->m_Pos );

	if ( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) 
	{
		pResult_SS->m_Pos.x = Point_VS.x * pMtx->aa[0][0] + Point_VS.z * pMtx->aa[2][0];
		pResult_SS->m_Pos.y = Point_VS.y * pMtx->aa[1][1] + Point_VS.z * pMtx->aa[2][1];
		pResult_SS->m_Pos.z = Point_VS.z;//*(1 - pMtx->aa[2][2]) + pMtx->aa[2][3];
		pResult_SS->m_fRadius = pSphere_WS->m_fRadius*pMtx->aa[0][0];
		if ( Point_VS.z > 0.0001f )
			fOOQ = -1.0f / (Point_VS.z * pMtx->aa[3][2]);
		else if ( Point_VS.z < -0.0001f )
			fOOQ = 1.0f / (Point_VS.z * pMtx->aa[3][2]);
		else if ( Point_VS.z >= 0)
			fOOQ = 10000.0f;
		else
			fOOQ = -10000.0f;
	} 
	else 
	{
		pResult_SS->m_Pos.x = Point_VS.x * pMtx->aa[0][0] + pMtx->aa[3][0];
		pResult_SS->m_Pos.y = Point_VS.y * pMtx->aa[1][1] + pMtx->aa[3][1];
		pResult_SS->m_Pos.z = Point_VS.z;//*(1 - pMtx->aa[2][2]) + pMtx->aa[2][3];
		fOOQ = 1;//fmath_Inv( pMtx->aa[3][3] );
		pResult_SS->m_fRadius = pSphere_WS->m_fRadius;
	}

	BOOL bPositiveZ = pResult_SS->m_Pos.z > 0.f;

	pResult_SS->m_Pos.x *= fOOQ;
	pResult_SS->m_Pos.y *= fOOQ;
	pResult_SS->m_fRadius *= bPositiveZ ? fOOQ : -fOOQ;

	return bPositiveZ;
}


//
// Computes the radius as a percentage of the veiwport width
// Returns -1.f if the sphere is behind the camera
//
f32 fviewport_ComputeScreenRadius_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFSphere *pSphere_WS ) 
{
	CFVec3 Point_VS;
	CFMtx44A *pMtx;

	FASSERT( _bModuleInitialized );

	pMtx = (CFMtx44A *)&pViewport->pViewportIS->GCProjMtx;
	Point_VS = pCamMtx->m44.MultPoint( pSphere_WS->m_Pos );

	if( Point_VS.z < -pSphere_WS->m_fRadius )
	{
		return -1.f;
	}

	if( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) 
	{
		if ( Point_VS.z > 0.0001f )
		{
			return -(pSphere_WS->m_fRadius*pMtx->aa[0][0]) / (Point_VS.z * pMtx->aa[3][2]);
		}
		else
		{
			return 1.f;
		}
	} 
	else
	{
		return pSphere_WS->m_fRadius;
	}
}


//
// Pass in fHalfFOVX=0 for an orthographic viewport (otherwise the viewport is perspective).
// Pass in fHalfFOVY=0 to have _InitViewport() compute the FOVY.
//
static void _InitViewport( FViewport_t *pViewport, FViewportType_e nType, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ,
						   u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) 
{
	CFGCViewportIS *pViewportIS;
	f32 fSinAcross, fCosAcross, fSinDown, fCosDown;
	f32 fLeft, fRight, fTop, fBottom;
	u32 i;

	if ( pViewport == NULL ) 
	{
		return;
	}

	pViewportIS = pViewport->pViewportIS;
	pViewportIS->nFlags = 0;

	FASSERT( nType>=0 && nType<FVIEWPORT_TYPE_COUNT );
	FASSERT( ftex_IsPointerValid( pTexDef, TRUE ) );
	FASSERT( nType==FVIEWPORT_TYPE_PERSPECTIVE || fHalfFOVX==0.0f );
	FASSERT( nType!=FVIEWPORT_TYPE_PERSPECTIVE || fHalfFOVX>=FVIEWPORT_MIN_HALFFOVX );
	//FASSERT( fHalfFOVX <= FVIEWPORT_MAX_HALFFOVX );
	FASSERT( fNearZ >= FVIEWPORT_MIN_NEAR_Z );
	FASSERT( (fFarZ - fNearZ) >= FVIEWPORT_MIN_NEAR_FAR_DELTA );
	//FASSERT( nScreenLeftX < (pTexDef ? pTexDef->TexInfo.nTexelsAcross : FVid_Mode.nPixelsAcross) );
	FASSERT( nScreenTopY < (pTexDef ? pTexDef->TexInfo.nTexelsDown : FVid_Mode.nPixelsDown) );
	//FASSERT( nWidth > 0 );
	//FASSERT( nHeight > 0 );
	//FASSERT( nWidth <= (pTexDef ? pTexDef->TexInfo.nTexelsAcross : FVid_Mode.nPixelsAcross) );
	//FASSERT( nHeight <= (pTexDef ? pTexDef->TexInfo.nTexelsDown : FVid_Mode.nPixelsDown) );
	//FASSERT( (nScreenLeftX + nWidth) <= (pTexDef ? pTexDef->TexInfo.nTexelsAcross : FVid_Mode.nPixelsAcross) );
	//FASSERT( (nScreenTopY + nHeight) <= (pTexDef ? pTexDef->TexInfo.nTexelsDown : FVid_Mode.nPixelsDown) );
	FASSERT( pTexDef==NULL || (pTexDef->TexInfo.nFlags & FTEX_FLAG_RENDERTARGET) );

	pViewport->nType = nType;
	pViewport->pTexDef = pTexDef;

	pViewport->fHalfFOVX = fHalfFOVX;
	pViewport->fFullFOVX = 2.0f * fHalfFOVX;

	pViewport->fNearZ = fNearZ;
	pViewport->fFarZ = fFarZ;

	pViewport->nWidth = nWidth;
	pViewport->nHeight = nHeight;
	pViewport->nScreenLeftX = nScreenLeftX;
	pViewport->nScreenTopY = nScreenTopY;
	pViewport->nScreenRightX = nScreenLeftX + nWidth;
	pViewport->nScreenBottomY = nScreenTopY + nHeight;
	pViewport->ScreenCorners.UpperLeft.x = (f32)pViewport->nScreenLeftX;
	pViewport->ScreenCorners.UpperLeft.y = (f32)pViewport->nScreenTopY;
	pViewport->ScreenCorners.LowerRight.x = (f32)pViewport->nScreenRightX;
	pViewport->ScreenCorners.LowerRight.y = (f32)pViewport->nScreenBottomY;
	pViewport->Res.x = (f32)pViewport->nWidth;
	pViewport->Res.y = (f32)pViewport->nHeight;
	pViewport->HalfRes.x = 0.5f * pViewport->Res.x;
	pViewport->HalfRes.y = 0.5f * pViewport->Res.y;
	pViewport->OORes.x = 1.0f / pViewport->Res.x;
	pViewport->OORes.y = 1.0f / pViewport->Res.y;
	pViewport->OOHalfRes.x = 2.0f * pViewport->OORes.x;
	pViewport->OOHalfRes.y = 2.0f * pViewport->OORes.y;
	pViewport->fAspectWOH = pViewport->Res.x / pViewport->Res.y;
	pViewport->fAspectHOW = 1.0f / pViewport->fAspectWOH;
	pViewport->fVPtoWindowPixelRatio = (f32)(pViewport->nWidth * pViewport->nHeight) / (f32)(FVid_Win.VidMode.nPixelsAcross * FVid_Win.VidMode.nPixelsDown);

	if ( fHalfFOVX ) 
	{
		// Perspective...

		fmath_SinCos( fHalfFOVX, &fSinAcross, &fCosAcross );
		pViewport->fTanHalfFOVX = fSinAcross / fCosAcross;

		fRight = fNearZ * pViewport->fTanHalfFOVX;
		fLeft = -fRight;

		if( fHalfFOVY ) 
		{
			// Vertical FOV is specified...

			pViewport->fHalfFOVY = fHalfFOVY;
			pViewport->fFullFOVY = 2.0f * pViewport->fHalfFOVY;

			fmath_SinCos( pViewport->fHalfFOVY, &fSinDown, &fCosDown );
			pViewport->fTanHalfFOVY = fSinDown / fCosDown;
			fBottom = fNearZ * pViewport->fTanHalfFOVY;
			fTop = -fBottom;
		} 
		else 
		{
			// Vertical FOV is not specified. Compute it ourselves...

			fBottom = fRight * pViewport->fAspectHOW;
			fTop = -fBottom;

			pViewport->fHalfFOVY = fmath_Atan( fBottom, fNearZ );
			pViewport->fFullFOVY = 2.0f * pViewport->fHalfFOVY;

			fmath_SinCos( pViewport->fHalfFOVY, &fSinDown, &fCosDown );
			pViewport->fTanHalfFOVY = fSinDown / fCosDown;
		}
	} 
	else 
	{
		// Orthographic...

		fSinAcross = 0.0f;
		fCosAcross = 1.0f;
		pViewport->fTanHalfFOVX = 0.0f;
		pViewport->fTanHalfFOVY = 0.0f;

		fRight = 0.0f;
		fLeft = 0.0f;
		fBottom = 0.0f;
		fTop = 0.0f;

		pViewport->fHalfFOVY = 0.0f;
		pViewport->fFullFOVY = 0.0f;

		fSinDown = 0.0f;
		fCosDown = 1.0f;
	}

	pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][0].Set( -fCosAcross,      0.0f, fSinAcross );	// Right
	pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][1].Set(  fCosAcross,      0.0f, fSinAcross );	// Left
	pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][2].Set(        0.0f,  fCosDown,   fSinDown );	// Bottom
	pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][3].Set(        0.0f, -fCosDown,   fSinDown );	// Top
	pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][4].Set(        0.0f,      0.0f,      -1.0f );	// Far
	pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][5].Set(        0.0f,      0.0f,       1.0f );	// Near

	for ( i = 0; i < 6; i++ ) 
	{
		pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][FVIEWPORT_SPACE_VS][i] = -pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][i];

		pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_WS][i] = pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][i];
		pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][FVIEWPORT_SPACE_WS][i] = pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][FVIEWPORT_SPACE_VS][i];
		pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_MS][i] = pViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][i];
		pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][FVIEWPORT_SPACE_MS][i] = pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][FVIEWPORT_SPACE_VS][i];

		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_WS][i].Zero();
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_MS][i].Zero();
	}

	pViewport->anNormCalcViewKey[FVIEWPORT_DIR_IN] = FXfm_nViewKey - 1;
	pViewport->anNormCalcViewKey[FVIEWPORT_DIR_OUT] = FXfm_nViewKey - 1;
	pViewport->anNormCalcModelViewKey[FVIEWPORT_DIR_IN] = FXfm_nModelViewKey - 1;
	pViewport->anNormCalcModelViewKey[FVIEWPORT_DIR_OUT] = FXfm_nModelViewKey - 1;

	BOOL bCoversEntireRenderTarget;
	if ( pTexDef == NULL ) 
	{
		bCoversEntireRenderTarget =
				( pViewport->nScreenLeftX==0 &&
				  pViewport->nScreenRightX==FVid_Mode.nPixelsAcross &&
				  pViewport->nScreenTopY==0 &&
				  pViewport->nScreenBottomY==FVid_Mode.nPixelsDown
				);
	} 
	else 
	{
		bCoversEntireRenderTarget =
				( pViewport->nScreenLeftX==0 &&
				  pViewport->nScreenRightX==pTexDef->TexInfo.nTexelsAcross &&
				  pViewport->nScreenTopY==0 &&
				  pViewport->nScreenBottomY==pTexDef->TexInfo.nTexelsDown
				);
	}
	if( bCoversEntireRenderTarget ) {
		pViewportIS->nFlags |= FGCVIEWPORT_FLAG_COVERS_ENTIRE_RENDER_TARGET;
	}

	switch( nType ) 
	{
		case FVIEWPORT_TYPE_PERSPECTIVE:
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][0].Zero();
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][1].Zero();
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][2].Zero();
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][3].Zero();
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][4].Set( 0.0f, 0.0f, fFarZ );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][5].Set( 0.0f, 0.0f, fNearZ );

			pViewportIS->nMtxType	= FGCVIEWPORT_PROJTYPE_PERSP;
			pViewportIS->fProjScale = _BuildPerspMtx(
											pViewportIS->GCProjMtx,
											fLeft,
											fRight,
											fTop,
											fBottom,
											pViewport->fNearZ,
											pViewport->fFarZ
										);
			break;

		case FVIEWPORT_TYPE_ORTHO3D:
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][0].Set(  pViewport->HalfRes.x, 0.0f, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][1].Set( -pViewport->HalfRes.x, 0.0f, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][2].Set( 0.0f, -pViewport->HalfRes.y, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][3].Set( 0.0f,  pViewport->HalfRes.y, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][4].Set( 0.0f, 0.0f, fFarZ );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][5].Set( 0.0f, 0.0f, fNearZ );

			pViewportIS->nMtxType	= FGCVIEWPORT_PROJTYPE_ORTHO;
			pViewportIS->fProjScale = _BuildOrthoMtx3D(
											pViewportIS->GCProjMtx,
											-pViewport->HalfRes.x,
											pViewport->HalfRes.x,
											-pViewport->HalfRes.y,
											pViewport->HalfRes.y,
											pViewport->fNearZ,
											pViewport->fFarZ
										);
			break;

		case FVIEWPORT_TYPE_ORTHO2D:
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][0].Set( pViewport->Res.x, 0.0f, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][1].Set( 0.0f, 0.0f, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][2].Set( 0.0f, 0.0f, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][3].Set( 0.0f, pViewport->Res.y, 0.0f );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][4].Set( 0.0f, 0.0f, fFarZ );
			pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][5].Set( 0.0f, 0.0f, fNearZ );

			pViewportIS->nMtxType	= FGCVIEWPORT_PROJTYPE_ORTHO;
			pViewportIS->fProjScale = _BuildOrthoMtx2D(
											pViewportIS->GCProjMtx,
											pViewport->Res.x,
											pViewport->Res.y,
											pViewport->fNearZ,
											pViewport->fFarZ
										);
			break;

		default:
			FASSERT_NOW;
	}

	pViewportIS->nFlags |= FGCVIEWPORT_FLAG_INITIALIZED;
	pViewport->uViewportKey = TRUE;
}


//
//
//
static void _ViewportResDestroyed( void *pResMem ) 
{
	CFGCViewportIS *pViewportIS;
	FViewport_t *pViewport;

	FASSERT( _bWindowCreated );

	pViewportIS = (CFGCViewportIS *)pResMem;
	pViewport = &pViewportIS->Viewport;

	if ( pViewport == _pCurrentViewport ) 
	{
		if ( !FGCvid_bResetting ) 
		{
			fviewport_SetActive( NULL );
		}

		_pCurrentViewport = NULL;
	}
}


//
//
//
static FViewport_t* _GetNewViewport( void ) 
{
	FResFrame_t ResFrame;
	FResHandle_t hRes;

	ResFrame = fres_GetFrame();

	CFGCViewportIS *pViewportIS = fnew CFGCViewportIS;
	if ( pViewportIS == NULL ) 
		goto _ExitNewViewportWithError;

	hRes = fres_Create( NULL, NULL );
	if ( hRes == FRES_NULLHANDLE ) 
		goto _ExitNewViewportWithError;

	fres_SetBaseAndCallback( hRes, pViewportIS, _ViewportResDestroyed );

	pViewportIS->Viewport.pViewportIS = pViewportIS;
	pViewportIS->nFlags = 0;
	pViewportIS->Viewport.uViewportKey = TRUE;

	return &pViewportIS->Viewport;

	// Failure:
_ExitNewViewportWithError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
/*
	FGCViewportIS_t *pViewportIS = (FGCViewportIS_t *)flinklist_RemoveTail( &_FreePool );
	if ( pViewportIS == NULL ) 
		return NULL;

	pViewportIS->Viewport.pViewportIS = pViewportIS;
	pViewportIS->bInitialized = FALSE;
	pViewportIS->Viewport.uViewportKey = TRUE;

	flinklist_AddTail( &_UsedPool, pViewportIS );

	return &pViewportIS->Viewport;
*/
}


//
// Note:
//
// D3D requires that the Z value range from 0 at nearplane to +1 at farplane, after the division.
// See elements [2][2] and [3][2].
//
static f32 _BuildPerspMtx( Mtx44 &mxResult, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ ) 
{
	f32 fOORightMinusLeft, fOOBottomMinusTop, fOOFarMinusNear, fDoubleNear;

	// Gamecube is REALLY slow clipping against the far and near planes, so we'll push the far 
	// plane out real far.  Pushing out the far plane does not have much of a detrimental effect
	fFarZ = 50000.f;
	
	// On GameCube, the GXSetProjection() function call ONLY loads
	// [0][0], [0][2], [1][1], [1][2], [2][2], and [2][3].
	// ALL OTHER PROJECTION MATRIX ENTRIES ARE IMPLIED
//	f32 fXScale = 1.09f;
	f32 fXScale = 1.f;
	f32 fYScale = 1.f;	
	f32 fZScale = 1.f;

	fOORightMinusLeft = fXScale / (fRightX - fLeftX);
	fOOBottomMinusTop = fYScale / (fBottomY - fTopY);//(fTopY - fBottomY);
	fOOFarMinusNear = fZScale / (fFarZ - fNearZ);
	fDoubleNear = 2.0f * fNearZ;

	mxResult[0][0] = fDoubleNear * fOORightMinusLeft;
	mxResult[0][1] = 0.0f;
	mxResult[0][2] = (fRightX + fLeftX) * fOORightMinusLeft;
	mxResult[0][3] = 0.0f;

	mxResult[1][0] = 0.0f;
	mxResult[1][1] = fDoubleNear * fOOBottomMinusTop;
	mxResult[1][2] = (fBottomY + fTopY) * fOOBottomMinusTop;
	mxResult[1][3] = 0.0f;

	mxResult[2][0] = 0.0f;
	mxResult[2][1] = 0.0f;
	mxResult[2][2] = -fNearZ * fOOFarMinusNear;
	mxResult[2][3] = -fFarZ * fNearZ * fOOFarMinusNear;

	mxResult[3][0] = 0.0f;
	mxResult[3][1] = 0.0f;
	mxResult[3][2] = -fZScale;	// This entry is implied to be -1 on GC
	mxResult[3][3] = 0.0f;

	return fZScale;
}


//
// Note:
//
// D3D requires that the Z value range from 0 at nearplane to +1 at farplane, after the division.
// See elements [2][2] and [3][2].
//
static f32 _BuildOrthoMtx3D( Mtx44 &mxResult, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ ) 
{
	f32 fOOLeftMinusRight, fOOTopMinusBottom, fOOFarMinusNear;

	#if _USE_PROJECTION_MATRIX_SCALE
//		fScale = _fD3DMaxVertexW ? (_fD3DMaxVertexW / fFarZ) : 1.0f;
	#else
//		fScale = 1.0f;
	#endif

//	f32 fXScale = 1.09f;
	f32 fXScale = 1.f;	
	f32 fYScale = 1.f;	
	f32 fZScale = 1.f;
	
	fOOLeftMinusRight = fXScale / (fLeftX - fRightX);
	fOOTopMinusBottom = fYScale / (fTopY - fBottomY);
	fOOFarMinusNear = fZScale / (fFarZ - fNearZ);

	mxResult[0][0] = -2.0f * fOOLeftMinusRight;
	mxResult[0][1] = 0.0f;
	mxResult[0][2] = 0.0f;
	mxResult[0][3] = (fRightX + fLeftX) * fOOLeftMinusRight;

	mxResult[1][0] = 0.0f;
	mxResult[1][1] = -2.0f * fOOTopMinusBottom;
	mxResult[1][2] = 0.0f;
	mxResult[1][3] = (fBottomY + fTopY) * fOOTopMinusBottom;

	mxResult[2][0] = 0.0f;
	mxResult[2][1] = 0.0f;
	mxResult[2][2] = -fOOFarMinusNear;
	mxResult[2][3] = -fFarZ * fOOFarMinusNear;

	mxResult[3][0] = 0.0f;
	mxResult[3][1] = 0.0f;
	mxResult[3][2] = 0.0f;
	mxResult[3][3] = fZScale;

	return fZScale;
}


//
// Note:
//
// D3D requires that the Z value range from 0 at nearplane to +1 at farplane, after the division.
// See elements [2][2] and [3][2].
//
static f32 _BuildOrthoMtx2D( Mtx44 &mxResult, f32 fWidth, f32 fHeight, f32 fNearZ, f32 fFarZ ) 
{
	f32 fOOWidth, fOOHeight, fOOFarMinusNear;

	f32 fXScale = 1.f;	
	f32 fYScale = 1.f;	
	f32 fZScale = 1.f;
	
	fOOWidth = fXScale / fWidth;
	fOOHeight = fYScale / fHeight;
	fOOFarMinusNear = fZScale / (fFarZ - fNearZ);

	mxResult[0][0] = 2.0f * fOOWidth;
	mxResult[0][1] = 0.0f;
	mxResult[0][2] = 0.0f;
	mxResult[0][3] = -fXScale;

	mxResult[1][0] = 0.0f;
	mxResult[1][1] = -2.0f * fOOHeight;
	mxResult[1][2] = 0.0f;
	mxResult[1][3] = fYScale;

	mxResult[2][0] = 0.0f;
	mxResult[2][1] = 0.0f;
	mxResult[2][2] = -fOOFarMinusNear;
	mxResult[2][3] = -fFarZ * fOOFarMinusNear;

	mxResult[3][0] = 0.0f;
	mxResult[3][1] = 0.0f;
	mxResult[3][2] = 0.0f;
	mxResult[3][3] = fZScale;

	return fZScale;
}


//
//
//
static void _SetRenderTarget( FTexDef_t *pTexDef ) 
{
	if( pTexDef == _pCurrentRendTargTexDef ) 
		return;
		
	FASSERT_NOW;
/*
	if( pTexDef == NULL ) 
	{
		// Set render target to back buffer...

		FDX8_pDev->SetRenderTarget( FDX8_pSurfBack, FDX8_pSurfDepthStencil );
	} 
	else 
	{
		// Set render target to texture...

		IDirect3DSurface8 *pD3DSurface;

		pTexDef->pTexData->pD3DTexture->GetSurfaceLevel( 0, &pD3DSurface );
		FDX8_pDev->SetRenderTarget( pD3DSurface, pTexDef->pTexData->pD3DDepthStencil );
		pD3DSurface->Release();
	}
*/
	_pCurrentRendTargTexDef = pTexDef;
}


//
//
//
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FGCVID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FGCVID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;

			fang_MemCopy( &_CurrentGCProjMtx, &FGC_MtxIdentity, sizeof(Mtx44) );
			GXSetProjection( _CurrentGCProjMtx, GX_ORTHOGRAPHIC );

			_pCurrentRendTargTexDef = NULL;

			FViewport_pDefaultOrtho = fviewport_Create();
			FASSERT( FViewport_pDefaultOrtho );
			FViewport_pDefaultOrtho->nWidth = FVid_Win.VidMode.nPixelsAcross;
			FViewport_pDefaultOrtho->nHeight = FVid_Win.VidMode.nPixelsDown;
			FViewport_pDefaultOrtho->nScreenLeftX = 0;
			FViewport_pDefaultOrtho->nScreenTopY = 0;
			FViewport_pDefaultOrtho->pUserData = NULL;
			FViewport_pDefaultOrtho->nScreenRightX = FVid_Win.VidMode.nPixelsAcross;
			FViewport_pDefaultOrtho->nScreenBottomY = FVid_Win.VidMode.nPixelsDown;
			fviewport_InitOrtho2D( FViewport_pDefaultOrtho, 
									0.1f, 2.f, 
									0.f, 0.f, 
									FViewport_pDefaultOrtho->nWidth, FViewport_pDefaultOrtho->nHeight, 
									NULL );

			_pDefaultViewport = fviewport_Create();
			FASSERT( _pDefaultViewport );
			_pDefaultViewport->nWidth = FVid_Win.VidMode.nPixelsAcross;
			_pDefaultViewport->nHeight = FVid_Win.VidMode.nPixelsDown;
			_pDefaultViewport->nScreenLeftX = 0;
			_pDefaultViewport->nScreenTopY = 0;
			_pDefaultViewport->pUserData = NULL;
			_pDefaultViewport->nScreenRightX = FVid_Win.VidMode.nPixelsAcross;
			_pDefaultViewport->nScreenBottomY = FVid_Win.VidMode.nPixelsDown;
			fviewport_InitPersp( _pDefaultViewport, FVIEWPORT_DEFAULT_HALFFOVX, FVIEWPORT_DEFAULT_NEARZ, FVIEWPORT_DEFAULT_FARZ );
			fviewport_SetActive( _pDefaultViewport );
			
			

			break;

		case FGCVID_EVENT_WINDOW_DESTROYED:
			_SetRenderTarget( NULL );
			_bWindowCreated = FALSE;
			break;

		case FGCVID_EVENT_PRE_RESET:
			_pResetRenderTarget = _pCurrentRendTargTexDef;
			_SetRenderTarget( NULL );
			break;

		case FGCVID_EVENT_POST_RESET:
			if ( _pCurrentViewport )
				fviewport_SetActive( _pCurrentViewport );
			else
				fviewport_SetActive( _pDefaultViewport );
				
			_SetRenderTarget( _pResetRenderTarget );
			break;
	}

	return TRUE;
}


