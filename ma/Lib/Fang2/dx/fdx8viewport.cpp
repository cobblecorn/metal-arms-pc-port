//////////////////////////////////////////////////////////////////////////////////////
// fdx8viewport.cpp - Fang viewport module (DX8 version).
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/26/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#include "fdx8.h"
#include "fdx8viewport.h"
#include "fdx8tex.h"
#include "fdx8vid.h"
#include "fdx8tex.h"
#include "fdx8xfm.h"

#include "fsh.h"

#include "fmath.h"


#define _USE_PROJECTION_MATRIX_SCALE	FALSE


FViewport_t *FViewport_pDefaultOrtho;



static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;
static f32 _fD3DMaxVertexW;

static FViewport_t *_pCurrentViewport;
static FViewport_t *_pDefaultViewport;

static FTexDef_t *_pCurrentRendTargTexDef;
static FTexDef_t *_pResetRenderTarget;

static CFMtx44A _CurrentD3DProjMtx;
static D3DVIEWPORT8 _D3DViewport;




static void _InitViewport( FViewport_t *pViewport, FViewportType_e nType, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ,
						   u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef );
static void _ViewportResDestroyed( void *pResMem );
static FViewport_t *_GetNewViewport( void );
static f32 _BuildPerspMtx( CFMtx44A *pMtx, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ );
static f32 _BuildOrthoMtx3D( CFMtx44A *pMtx, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ );
static f32 _BuildOrthoMtx2D( CFMtx44A *pMtx, f32 fWidth, f32 fHeight, f32 fNearZ, f32 fFarZ );
static void _SetRenderTarget( FTexDef_t *pTexDef );
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );



BOOL fdx8viewport_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_bWindowCreated = FALSE;

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_pCurrentViewport = NULL;
	_pDefaultViewport = NULL;

	_bModuleInitialized = TRUE;

	return TRUE;
}

void fdx8viewport_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

FViewport_t *fviewport_Create( void ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	FViewport_t *pViewport = _GetNewViewport();
	if( pViewport == NULL ) {
		return NULL;
	}

	return pViewport;
}

void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	if( pViewport ) {
		if( pTexDef == NULL ) {
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, 0.0f, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} else {
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, 0.0f, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}

void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, 0.0f, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}

void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	if( pViewport ) {
		if( pTexDef == NULL ) {
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, fHalfFOVY, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} else {
			_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, fHalfFOVY, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}

void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_PERSPECTIVE, fHalfFOVX, fHalfFOVY, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}

void fviewport_InitOrtho3D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	if( pViewport ) {
		if( pTexDef == NULL ) {
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO3D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} else {
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO3D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}

void fviewport_InitOrtho3D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO3D, 0.0f, 0.0f, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}

void fviewport_InitOrtho2D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	if( pViewport ) {
		if( pTexDef == NULL ) {
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO2D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, NULL );
		} else {
			_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO2D, 0.0f, 0.0f, fNearZ, fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pTexDef );
		}
	}
}

void fviewport_InitOrtho2D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY,
						  u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );

	_InitViewport( pViewport, FVIEWPORT_TYPE_ORTHO2D, 0.0f, 0.0f, fNearZ, fFarZ, nScreenLeftX, nScreenTopY, nWidth, nHeight, pTexDef );
}

BOOL fviewport_IsInit( FViewport_t *pViewport ) {
	FASSERT( _bWindowCreated );

	if( pViewport ) {
		return pViewport->pViewportIS->bInitialized;
	}

	return FALSE;
}

FViewport_t *fviewport_SetActive( FViewport_t *pViewport ) {
	FViewport_t *pPrevViewport;
	CFDX8ViewportIS *pViewportIS;

	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );
	FASSERT( pViewport==NULL || pViewport->pViewportIS->bInitialized );
	FASSERT( _pDefaultViewport );

	if (FViewport_CallbackFunc && pViewport) 
	{
		FViewport_CallbackFunc(pViewport);
		return _pCurrentViewport;
	}

	if( _pCurrentViewport != _pDefaultViewport ) {
		pPrevViewport = _pCurrentViewport;
	} else {
		pPrevViewport = NULL;
	}

	if( pViewport == NULL ) {
		pViewport = _pDefaultViewport;
	}

	if( pViewport!=_pCurrentViewport || pViewport->uViewportKey ) {
		pViewport->uViewportKey = FALSE;

		_pCurrentViewport = pViewport;

		pViewportIS = pViewport->pViewportIS;

		_SetRenderTarget( pViewport->pTexDef );

		_D3DViewport.X = pViewport->nScreenLeftX;
		_D3DViewport.Y = pViewport->nScreenTopY;
		_D3DViewport.Width = pViewport->nWidth;
		_D3DViewport.Height = pViewport->nHeight;
		_D3DViewport.MinZ = 0.0f;
		_D3DViewport.MaxZ = 1.0f;
		FDX8_pDev->SetViewport( &_D3DViewport );

		fang_MemCopy( &_CurrentD3DProjMtx, &pViewportIS->D3DProjMtx, sizeof(CFMtx44A) );
//		FDX8_pDev->SetTransform( D3DTS_PROJECTION, (D3DMATRIX *)&pViewportIS->D3DProjMtx );
		FDX8_SetProjMatrix( &pViewportIS->D3DProjMtx );

		fsh_LoadProjMtx(pViewport->fHalfFOVX*2.0f, pViewport->fHalfFOVY*2.0f);
	}

	return pPrevViewport;
}

FViewport_t *fviewport_GetActive_Absolute( void ) {
	FASSERT( _bWindowCreated );
	return _pCurrentViewport;
}

FViewport_t *fviewport_GetActive( void ) {
	FASSERT( _bWindowCreated );

	if( _pCurrentViewport != _pDefaultViewport ) {
		return _pCurrentViewport;
	} else {
		return NULL;
	}
}

void fviewport_Clear( void ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );
	FASSERT( _pCurrentViewport && _pCurrentViewport->pViewportIS->bInitialized );

	fviewport_Clear( FVIEWPORT_CLEARFLAG_COLOR, 0.0f, 0.0f, 0.0f, 0.0f, 0 );
}

void fviewport_Clear( f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue ) {
	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );
	FASSERT( _pCurrentViewport && _pCurrentViewport->pViewportIS->bInitialized );

	fviewport_Clear( FVIEWPORT_CLEARFLAG_COLOR, fUnitRed, fUnitGreen, fUnitBlue, 0.0f, 0 );
}

void fviewport_Clear( u32 nClearFlag, f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue, f32 fDepthValue, u32 nStencilValue ) {
	DWORD nFlags, nStencil;
	D3DCOLOR D3DColor = 0;
	f32 fDepth;
	u32 nDepthBitCount, nStencilBitCount;

	FASSERT( _bWindowCreated );
	FASSERT( !fdx8vid_bResetting );
	FASSERT( _pCurrentViewport && _pCurrentViewport->pViewportIS->bInitialized );

	if( _pCurrentViewport == NULL ) {
		return;
	}

	nFlags = 0;
	nStencil = 0;
	fDepth = 0.0f;

	if( nClearFlag & FVIEWPORT_CLEARFLAG_COLOR ) {
		// Caller wishes to fill the color buffer...

		nFlags |= D3DCLEAR_TARGET;
		D3DColor = 0x00000000
				 | (u32)(255.0f*fUnitRed) << 16
				 | (u32)(255.0f*fUnitGreen) << 8
				 | (u32)(255.0f*fUnitBlue);

	}

	if( _pCurrentViewport->pTexDef == NULL ) {
		nDepthBitCount = FVid_Mode.nDepthBits;
		nStencilBitCount = FVid_Mode.nStencilBits;
	} else {
		nDepthBitCount = _pCurrentViewport->pTexDef->TexInfo.nRenderTargetDepthBitCount;
		nStencilBitCount = _pCurrentViewport->pTexDef->TexInfo.nRenderTargetStencilBitCount;
	}

	if( nDepthBitCount && (nClearFlag & FVIEWPORT_CLEARFLAG_DEPTH) ) {
		// Caller wishes to fill the depth buffer...

		nFlags |= D3DCLEAR_ZBUFFER;
		fDepth = fDepthValue;
	}

	if( nStencilBitCount && (nClearFlag & FVIEWPORT_CLEARFLAG_STENCIL) ) {
		// Caller wishes to fill the stencil buffer...

		nFlags |= D3DCLEAR_STENCIL;
		nStencil = nStencilValue & ((1<<nStencilBitCount) - 1);
	}

	if( _pCurrentViewport->pViewportIS->bCoversEntireRenderTarget) {
		FDX8_pDev->Clear( 0, NULL, nFlags, D3DColor, fDepth, nStencil );
	} else {
		D3DRECT Rect;

		Rect.x1 = _pCurrentViewport->nScreenLeftX;
		Rect.y1 = _pCurrentViewport->nScreenTopY;
		Rect.x2 = _pCurrentViewport->nScreenRightX;
		Rect.y2 = _pCurrentViewport->nScreenBottomY;

		FDX8_pDev->Clear( 1, &Rect, nFlags, D3DColor, fDepth, nStencil );
	}
}

void fdx8viewport_RestoreState( void ) {
	FViewport_t *pCurrentViewport;

	FASSERT( _bWindowCreated );

	pCurrentViewport = _pCurrentViewport;
	_pCurrentViewport = (FViewport_t *)0xffffffff;
	_pCurrentRendTargTexDef = (FTexDef_t *)0xffffffff;
	fviewport_SetActive( pCurrentViewport );
}

const CFMtx44A *fviewport_GetPlatformSpecificProjectionMatrix( const FViewport_t *pViewport ) {
	FASSERT( _bModuleInitialized );

	if( pViewport ) {
		return &pViewport->pViewportIS->D3DProjMtx;
	} else {
		return NULL;
	}
}

f32 fviewport_ComputeOrtho2DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS ) {
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
}

f32 fviewport_ComputeOrtho3DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS ) {
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

	pPoint_SS->x = pPoint_SS->x*pViewport->HalfRes.x;
	pPoint_SS->y = pPoint_SS->y*pViewport->HalfRes.y;

	return Point_VS.z;
}

f32 fviewport_ComputeUnitOrtho3DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS ) {
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

	return Point_VS.z;
}


//
// Returns -1.f if the sphere is behind the camera
//
f32 fviewport_ComputeScreenRadius_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFSphere *pSphere_WS ) 
{
	CFVec3 Point_VS;
	CFMtx44A *pMtx;

	FASSERT( _bModuleInitialized );

	pMtx = &pViewport->pViewportIS->D3DProjMtx;
	Point_VS = pCamMtx->m44.MultPoint( pSphere_WS->m_Pos );

	if( Point_VS.z < -pSphere_WS->m_fRadius )
	{
		return -1.f;
	}

	if( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) 
	{
		if ( Point_VS.z > 0.0001f )
		{
			return (pSphere_WS->m_fRadius*pMtx->aa[0][0]) / (Point_VS.z * pMtx->aa[2][3]);
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
// Returns TRUE if the sphere was on screen, false if it was not
//
BOOL fviewport_ComputeScreenPointAndSize_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFSphere *pSphere_WS, CFSphere *pResult_SS ) 
{
	CFVec3 Point_VS;
	CFMtx44A *pMtx;
	f32 fOOQ;

	FASSERT( _bModuleInitialized );

	pMtx = &pViewport->pViewportIS->D3DProjMtx;
	Point_VS = pCamMtx->m44.MultPoint( pSphere_WS->m_Pos );

	if( pViewport->nType == FVIEWPORT_TYPE_PERSPECTIVE ) 
	{
		pResult_SS->m_Pos.x = Point_VS.x*pMtx->aa[0][0] + Point_VS.z*pMtx->aa[2][0];
		pResult_SS->m_Pos.y = Point_VS.y*pMtx->aa[1][1] + Point_VS.z*pMtx->aa[2][1];
		pResult_SS->m_Pos.z = Point_VS.z;//*pMtx->aa[2][2] + pMtx->aa[3][2];
		pResult_SS->m_fRadius = pSphere_WS->m_fRadius*pMtx->aa[0][0];
		if ( Point_VS.z > 0.0001f )
			fOOQ = 1.0f / (Point_VS.z * pMtx->aa[2][3]);
		else if ( Point_VS.z < -0.0001f )
			fOOQ = -1.0f / (Point_VS.z * pMtx->aa[2][3]);
		else if ( Point_VS.z >= 0)
			fOOQ = 10000.0f;
		else
			fOOQ = -10000.0f;
	} 
	else 
	{
		pResult_SS->m_Pos.x = Point_VS.x*pMtx->aa[0][0] + pMtx->aa[3][0];
		pResult_SS->m_Pos.y = Point_VS.y*pMtx->aa[1][1] + pMtx->aa[3][1];
		pResult_SS->m_Pos.z = Point_VS.z;//*pMtx->aa[2][2] + pMtx->aa[3][2];
		fOOQ = 1.0f / pMtx->aa[3][3];
		pResult_SS->m_fRadius = pSphere_WS->m_fRadius;
	}

	BOOL bPositiveZ = pResult_SS->m_Pos.z > 0.f;

	pResult_SS->m_Pos.x *= fOOQ;
	pResult_SS->m_Pos.y *= fOOQ;
	pResult_SS->m_fRadius *= bPositiveZ ? fOOQ : -fOOQ;

	return bPositiveZ;
}


// Pass in fHalfFOVX=0 for an orthographic viewport (otherwise the viewport is perspective).
// Pass in fHalfFOVY=0 to have _InitViewport() compute the FOVY.
static void _InitViewport( FViewport_t *pViewport, FViewportType_e nType, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ,
						   u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef ) {
	CFDX8ViewportIS *pViewportIS;
	f32 fSinAcross, fCosAcross, fSinDown, fCosDown;
	f32 fLeft, fRight, fTop, fBottom;
	u32 i;

	if( pViewport == NULL ) {
		return;
	}

	pViewportIS = pViewport->pViewportIS;
	pViewportIS->bInitialized = FALSE;

	FASSERT( nType>=0 && nType<FVIEWPORT_TYPE_COUNT );
	FASSERT( ftex_IsPointerValid( pTexDef, TRUE ) );
	FASSERT( nType==FVIEWPORT_TYPE_PERSPECTIVE || fHalfFOVX==0.0f );
	FASSERT( nType!=FVIEWPORT_TYPE_PERSPECTIVE || fHalfFOVX>=FVIEWPORT_MIN_HALFFOVX );
	//FASSERT( fHalfFOVX <= FVIEWPORT_MAX_HALFFOVX );
	FASSERT( fNearZ >= FVIEWPORT_MIN_NEAR_Z );
	FASSERT( (fFarZ - fNearZ) >= FVIEWPORT_MIN_NEAR_FAR_DELTA );
	FASSERT( nScreenLeftX < (pTexDef ? pTexDef->TexInfo.nTexelsAcross : FVid_Mode.nPixelsAcross) );
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

	if( fHalfFOVX ) {
		// Perspective...

		fmath_SinCos( fHalfFOVX, &fSinAcross, &fCosAcross );
		pViewport->fTanHalfFOVX = fSinAcross / fCosAcross;

		fRight = fNearZ * pViewport->fTanHalfFOVX;
		fLeft = -fRight;

		if( fHalfFOVY ) {
			// Vertical FOV is specified...

			pViewport->fHalfFOVY = fHalfFOVY;
			pViewport->fFullFOVY = 2.0f * pViewport->fHalfFOVY;

			fmath_SinCos( pViewport->fHalfFOVY, &fSinDown, &fCosDown );
			pViewport->fTanHalfFOVY = fSinDown / fCosDown;
			fBottom = fNearZ * pViewport->fTanHalfFOVY;
			fTop = -fBottom;
		} else {
			// Vertical FOV is not specified. Compute it ourselves...

			fBottom = fRight * pViewport->fAspectHOW;
			fTop = -fBottom;

			pViewport->fHalfFOVY = fmath_Atan( fBottom, fNearZ );
			pViewport->fFullFOVY = 2.0f * pViewport->fHalfFOVY;

			fmath_SinCos( pViewport->fHalfFOVY, &fSinDown, &fCosDown );
			pViewport->fTanHalfFOVY = fSinDown / fCosDown;
		}
	} else {
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

	for( i=0; i<6; i++ ) {
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

	if( pTexDef == NULL ) {
		pViewportIS->bCoversEntireRenderTarget =
				( pViewport->nScreenLeftX==0 &&
				  pViewport->nScreenRightX==FVid_Mode.nPixelsAcross &&
				  pViewport->nScreenTopY==0 &&
				  pViewport->nScreenBottomY==FVid_Mode.nPixelsDown
				);
	} else {
		pViewportIS->bCoversEntireRenderTarget =
				( pViewport->nScreenLeftX==0 &&
				  pViewport->nScreenRightX==pTexDef->TexInfo.nTexelsAcross &&
				  pViewport->nScreenTopY==0 &&
				  pViewport->nScreenBottomY==pTexDef->TexInfo.nTexelsDown
				);
	}

	switch( nType ) {
	case FVIEWPORT_TYPE_PERSPECTIVE:
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][0].Zero();
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][1].Zero();
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][2].Zero();
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][3].Zero();
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][4].Set( 0.0f, 0.0f, fFarZ );
		pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS][5].Set( 0.0f, 0.0f, fNearZ );

		pViewportIS->fProjScale = _BuildPerspMtx(
										&pViewportIS->D3DProjMtx,
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

		pViewportIS->fProjScale = _BuildOrthoMtx3D(
										&pViewportIS->D3DProjMtx,
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

		pViewportIS->fProjScale = _BuildOrthoMtx2D(
										&pViewportIS->D3DProjMtx,
										pViewport->Res.x,
										pViewport->Res.y,
										pViewport->fNearZ,
										pViewport->fFarZ
									);
		break;

	default:
		FASSERT_NOW;
	}

	pViewportIS->bInitialized = TRUE;
	pViewport->uViewportKey = TRUE;
}

static void _ViewportResDestroyed( void *pResMem ) {
	CFDX8ViewportIS *pViewportIS;
	FViewport_t *pViewport;

	FASSERT( _bWindowCreated );

	pViewportIS = (CFDX8ViewportIS *)pResMem;
	pViewport = &pViewportIS->Viewport;

	if( pViewport == _pCurrentViewport ) {
		if( !fdx8vid_bResetting ) {
			fviewport_SetActive( NULL );
		}

		_pCurrentViewport = NULL;
	}
}

static FViewport_t *_GetNewViewport( void ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;

	ResFrame = fres_GetFrame();

	CFDX8ViewportIS *pViewportIS = fnew CFDX8ViewportIS;
	if( pViewportIS == NULL ) {
		goto _ExitNewViewportWithError;
	}

	hRes = fres_Create( NULL, NULL );
	if( hRes == FRES_NULLHANDLE ) {
		goto _ExitNewViewportWithError;
	}

	fres_SetBaseAndCallback( hRes, pViewportIS, _ViewportResDestroyed );

	pViewportIS->Viewport.pViewportIS = pViewportIS;
	pViewportIS->bInitialized = FALSE;
	pViewportIS->Viewport.uViewportKey = TRUE;

	return &pViewportIS->Viewport;

	// Failure:
_ExitNewViewportWithError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
}

// Note:
//
// D3D requires that the Z value range from 0 at nearplane to +1 at farplane, after the division.
// See elements [2][2] and [3][2].
static f32 _BuildPerspMtx( CFMtx44A *pMtx, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ ) {
	f32 fOORightMinusLeft, fOOBottomMinusTop, fOOFarMinusNear, fDoubleNear, fScale;

	#if _USE_PROJECTION_MATRIX_SCALE
		fScale = _fD3DMaxVertexW ? (_fD3DMaxVertexW / fFarZ) : 1.0f;
	#else
		fScale = 1.0f;
	#endif

	fOORightMinusLeft = fScale / (fRightX - fLeftX);
	fOOBottomMinusTop = fScale / (fBottomY - fTopY);
	fOOFarMinusNear = fScale / (fFarZ - fNearZ);
	fDoubleNear = 2.0f * fNearZ;

	pMtx->aa[0][0] = fDoubleNear * fOORightMinusLeft;
	pMtx->aa[0][1] = 0.0f;
	pMtx->aa[0][2] = 0.0f;
	pMtx->aa[0][3] = 0.0f;

	pMtx->aa[1][0] = 0.0f;
	pMtx->aa[1][1] = fDoubleNear * fOOBottomMinusTop;
	pMtx->aa[1][2] = 0.0f;
	pMtx->aa[1][3] = 0.0f;

	pMtx->aa[2][0] = (fRightX + fLeftX) * fOORightMinusLeft;
	pMtx->aa[2][1] = (fBottomY + fTopY) * fOOBottomMinusTop;
	pMtx->aa[2][2] = fFarZ * fOOFarMinusNear;
	pMtx->aa[2][3] = fScale;

	pMtx->aa[3][0] = 0.0f;
	pMtx->aa[3][1] = 0.0f;
	pMtx->aa[3][2] = -fFarZ * fNearZ * fOOFarMinusNear;
	pMtx->aa[3][3] = 0.0f;

	return fScale;
}

// Note:
//
// D3D requires that the Z value range from 0 at nearplane to +1 at farplane, after the division.
// See elements [2][2] and [3][2].
static f32 _BuildOrthoMtx3D( CFMtx44A *pMtx, f32 fLeftX, f32 fRightX, f32 fTopY, f32 fBottomY, f32 fNearZ, f32 fFarZ ) {
	f32 fOOLeftMinusRight, fOOTopMinusBottom, fOOFarMinusNear, fScale;

	#if _USE_PROJECTION_MATRIX_SCALE
		fScale = _fD3DMaxVertexW ? (_fD3DMaxVertexW / fFarZ) : 1.0f;
	#else
		fScale = 1.0f;
	#endif

	fOOLeftMinusRight = fScale / (fLeftX - fRightX);
	fOOTopMinusBottom = fScale / (fTopY - fBottomY);
	fOOFarMinusNear = fScale / (fFarZ - fNearZ);

	pMtx->aa[0][0] = -2.0f * fOOLeftMinusRight;
	pMtx->aa[0][1] = 0.0f;
	pMtx->aa[0][2] = 0.0f;
	pMtx->aa[0][3] = 0.0f;

	pMtx->aa[1][0] = 0.0f;
	pMtx->aa[1][1] = -2.0f * fOOTopMinusBottom;
	pMtx->aa[1][2] = 0.0f;
	pMtx->aa[1][3] = 0.0f;

	pMtx->aa[2][0] = 0.0f;
	pMtx->aa[2][1] = 0.0f;
	pMtx->aa[2][2] = fOOFarMinusNear;
	pMtx->aa[2][3] = 0.0f;

	pMtx->aa[3][0] = (fRightX + fLeftX) * fOOLeftMinusRight;
	pMtx->aa[3][1] = (fBottomY + fTopY) * fOOTopMinusBottom;
	pMtx->aa[3][2] = fNearZ * -fOOFarMinusNear;
	pMtx->aa[3][3] = fScale;

	return fScale;
}

// Note:
//
// D3D requires that the Z value range from 0 at nearplane to +1 at farplane, after the division.
// See elements [2][2] and [3][2].
static f32 _BuildOrthoMtx2D( CFMtx44A *pMtx, f32 fWidth, f32 fHeight, f32 fNearZ, f32 fFarZ ) {
	f32 fOOWidth, fOOHeight, fOOFarMinusNear, fScale;

	#if _USE_PROJECTION_MATRIX_SCALE
		fScale = _fD3DMaxVertexW ? (_fD3DMaxVertexW / fFarZ) : 1.0f;
	#else
		fScale = 1.0f;
	#endif

	fOOWidth = fScale / fWidth;
	fOOHeight = fScale / fHeight;
	fOOFarMinusNear = fScale / (fFarZ - fNearZ);

	pMtx->aa[0][0] = 2.0f * fOOWidth;
	pMtx->aa[0][1] = 0.0f;
	pMtx->aa[0][2] = 0.0f;
	pMtx->aa[0][3] = 0.0f;

	pMtx->aa[1][0] = 0.0f;
	pMtx->aa[1][1] = -2.0f * fOOHeight;
	pMtx->aa[1][2] = 0.0f;
	pMtx->aa[1][3] = 0.0f;

	pMtx->aa[2][0] = 0.0f;
	pMtx->aa[2][1] = 0.0f;
	pMtx->aa[2][2] = fOOFarMinusNear;
	pMtx->aa[2][3] = 0.0f;

	pMtx->aa[3][0] = -fScale;
	pMtx->aa[3][1] = fScale;
	pMtx->aa[3][2] = fNearZ * -fOOFarMinusNear;
	pMtx->aa[3][3] = fScale;

	return fScale;
}

static void _SetRenderTarget( FTexDef_t *pTexDef ) {
	if( pTexDef == _pCurrentRendTargTexDef ) {
		return;
	}

	if( pTexDef == NULL ) {
		// Set render target to back buffer...

		FDX8_pDev->SetRenderTarget( FDX8_pSurfBack, FDX8_pSurfDepthStencil );
	} else {
		// Set render target to texture...

		IDirect3DSurface8 *pD3DSurface;

		pTexDef->pTexData->pD3DTexture->GetSurfaceLevel( 0, &pD3DSurface );
		FDX8_pDev->SetRenderTarget( pD3DSurface, pTexDef->pTexData->pD3DDepthStencil );
		pD3DSurface->Release();
	}

	_pCurrentRendTargTexDef = pTexDef;
}

static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) {
	case FDX8VID_EVENT_WINDOW_CREATED:
		_bWindowCreated = TRUE;

		_fD3DMaxVertexW = FDX8_Caps.MaxVertexW;

		fang_MemCopy( &_CurrentD3DProjMtx, &FDX8_MatrixIdentity, sizeof(CFMtx44A) );
//		FDX8_pDev->SetTransform( D3DTS_PROJECTION, (D3DMATRIX *)&_CurrentD3DProjMtx );
		FDX8_SetProjMatrix( &_CurrentD3DProjMtx );

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
								0, 0, 
								FViewport_pDefaultOrtho->nWidth, FViewport_pDefaultOrtho->nHeight, 
								NULL );

		_pDefaultViewport = fviewport_Create();
		FASSERT( _pDefaultViewport );
		_pDefaultViewport->pUserData = NULL;
		fviewport_InitPersp( _pDefaultViewport, FVIEWPORT_DEFAULT_HALFFOVX, FVIEWPORT_DEFAULT_NEARZ, FVIEWPORT_DEFAULT_FARZ );
		fviewport_SetActive( _pDefaultViewport );

		break;

	case FDX8VID_EVENT_WINDOW_DESTROYED:
		_SetRenderTarget( NULL );
		_bWindowCreated = FALSE;
		break;

	case FDX8VID_EVENT_PRE_RESET:
		_pResetRenderTarget = _pCurrentRendTargTexDef;
		_SetRenderTarget( NULL );
		break;

	case FDX8VID_EVENT_POST_RESET:
		FDX8_pDev->SetViewport( &_D3DViewport );
//		FDX8_pDev->SetTransform( D3DTS_PROJECTION, (D3DMATRIX *)&_CurrentD3DProjMtx );
		FDX8_SetProjMatrix( &_CurrentD3DProjMtx );
		_SetRenderTarget( _pResetRenderTarget );
		break;
	}

	return TRUE;
}


