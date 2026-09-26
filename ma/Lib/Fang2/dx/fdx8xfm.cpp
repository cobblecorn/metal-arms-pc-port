//////////////////////////////////////////////////////////////////////////////////////
// fdx8xfm.cpp - Fang Transformation module.
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
// 10/05/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#include "fdx8.h"
#include "fdx8xfm.h"
#include "fdx8vid.h"

#include "fdx8vshader_const.h"

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static FINLINE void _ConvertXfmToD3DMtx( D3DMATRIX *pDestD3DMtx, const CFMtx43A *pMtx43A );
static FINLINE void _ConvertXfmToD3DMtx( D3DMATRIX *pDestD3DMtx, const CFMtx43 *pMtx43 );
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );


//
//
//
BOOL fdx8xfm_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bWindowCreated = FALSE;

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fdx8xfm_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
void fdx8xfm_SetRenormalizeNormalsIfNeeded( f32 fMtxScale ) 
{
	f32 fScaleMinusOne;

	fScaleMinusOne = fMtxScale - 1.0f;

	if( FMATH_FABS(fScaleMinusOne) <= 0.1f ) 
	{
		fdx8_SetRenderState_NORMALIZENORMALS( FALSE );
	} 
	else 
	{
		fdx8_SetRenderState_NORMALIZENORMALS( TRUE );
	}
}


//
//
//
void fdx8xfm_SetRenormalizeNormalsIfNeeded( const CFMtx43A *pMtx ) 
{
	f32 fScaleMinusOne2;

	fScaleMinusOne2 = pMtx->m_vRight.MagSq() - 1.0f;

	if( FMATH_FABS(fScaleMinusOne2) <= 0.2f ) 
	{
		fdx8_SetRenderState_NORMALIZENORMALS( FALSE );
	} 
	else 
	{
		fdx8_SetRenderState_NORMALIZENORMALS( TRUE );
	}
}


//
//
//
void fdx8xfm_SetDXMatrices( BOOL bUsingVertexShaders ) 
{
	FASSERT( _bWindowCreated );

	if ( FXfm_pMirrorMtx )
	{
		static CFMtx43A mtxMirrorView( FXfm_pView->m_MtxF );
		mtxMirrorView.Mul( *FXfm_pMirrorMtx );
		FDX8_SetViewMatrix( &mtxMirrorView );
	}
	else
	{
		FDX8_SetViewMatrix( &FXfm_pView->m_MtxF );
	}

	#if FANG_PLATFORM_XB
		XGMATRIX Mtx;
	#else
		D3DXMATRIX Mtx;
	#endif
		
	_ConvertXfmToD3DMtx( &Mtx, &FXfm_pModel->m_MtxF );

	if ( bUsingVertexShaders )
	{
#if FANG_PLATFORM_XB
		XGMATRIX matTemp, matWorldViewProj, matWorldView, World;
		XGMatrixTranspose( &World, &Mtx );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );
		//This is for the vertex shaders
				
		XGMatrixMultiply( &matTemp, &Mtx, &FDX8_CurrentViewMtx );
		XGMatrixTranspose( &matWorldView, &matTemp );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEW_0, &matWorldView(0, 0), 4 );
		XGMatrixMultiply( &matWorldViewProj, &matTemp, &FDX8_CurrentProjMtx );
				
		// Projection to clip space
		XGMatrixTranspose( &matWorldViewProj, &matWorldViewProj );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4 );
#else
		D3DXMATRIX matTemp, matWorldViewProj, matWorldView, World;
		D3DXMatrixTranspose( &World, &Mtx );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );
		//This is for the vertex shaders
				
		D3DXMatrixMultiply( &matTemp, &Mtx, &FDX8_CurrentViewMtx );
		D3DXMatrixTranspose( &matWorldView, &matTemp );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEW_0, &matWorldView(0, 0), 4 );
		D3DXMatrixMultiply( &matWorldViewProj, &matTemp, &FDX8_CurrentProjMtx );
				
		// Projection to clip space
		D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
		FDX8_pDev->SetVertexShaderConstant(CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4);
#endif
	}
	else
	{
		FDX8_pDev->SetTransform( D3DTS_WORLDMATRIX(0), &Mtx );
	}
	//
}


//
//
//
void fdx8xfm_SetModelDXMatrix(  BOOL bUsingVertexShaders ) 
{
	FASSERT( _bWindowCreated );

#if FANG_PLATFORM_XB
	XGMATRIX Mtx;
#else
	D3DXMATRIX Mtx;
#endif

	_ConvertXfmToD3DMtx( &Mtx, &FXfm_pModel->m_MtxF );

	FDX8_pDev->SetTransform( D3DTS_WORLDMATRIX(0), &Mtx );

	if ( bUsingVertexShaders )
	{
#if FANG_PLATFORM_XB
		XGMATRIX World;
		XGMatrixTranspose( &World, &Mtx );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );
#else
		D3DXMATRIX World;
		D3DXMatrixTranspose( &World, &Mtx );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );
#endif
	}
	else
	{
		FDX8_pDev->SetTransform( D3DTS_WORLDMATRIX(0), &Mtx );
	}
}


//
//
//
void fdx8xfm_SetViewDXMatrix( BOOL bUsingVertexShaders ) 
{
//	D3DMATRIX Mtx;

	FASSERT( _bWindowCreated );

	if ( FXfm_pMirrorMtx )
	{
		CFMtx43A mtxMirrorView( FXfm_pView->m_MtxF );
		mtxMirrorView.Mul( *FXfm_pMirrorMtx );
		FDX8_SetViewMatrix( &mtxMirrorView );
	}
	else
	{
		FDX8_SetViewMatrix( &FXfm_pView->m_MtxF );
	}

	if ( bUsingVertexShaders )
	{
		//This is for the vertex shaders
		D3DXMATRIX World;
		FDX8_pDev->GetTransform( D3DTS_WORLD, &World );
		D3DXMatrixTranspose( &World, &World );
		FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0,0), 4 );
	}
}


//
//
//
void fxfm_SetViewAndWorldSpaceModelMatrices( void )
{
	fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &CFMtx43A::m_IdentityMtx, TRUE );
	fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
}


//
//
//
void fdx8xfm_SetCustomDXTextureMatrix( u32 nTexMtxIndex, const CFMtx43 *pMtx43 ) 
{
	D3DXMATRIX Mtx;

	FASSERT( _bWindowCreated );

	_ConvertXfmToD3DMtx( &Mtx, pMtx43 );
	FDX8_pDev->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + nTexMtxIndex), &Mtx );
}


//
// This should only be called when using the shaders
//
void fdx8xfm_SetCustomDXMatrix( D3DTRANSFORMSTATETYPE nD3DMatrixType, const CFMtx43A *pMtx43A, BOOL bUsingVertexShaders ) 
{
	FASSERT( _bWindowCreated );

	#if FANG_PLATFORM_XB
		XGMATRIX Mtx;
	#else
		D3DXMATRIX Mtx;
	#endif

	_ConvertXfmToD3DMtx( &Mtx, pMtx43A );

	if (nD3DMatrixType == D3DTS_WORLDMATRIX(0) && bUsingVertexShaders)
	{
		#if FANG_PLATFORM_XB
			//This is for the vertex shaders
			XGMATRIX matTemp, matWorldViewProj, matWorldView, World;
			XGMatrixTranspose( &World, &Mtx );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );

			XGMatrixMultiply( &matTemp, &Mtx, &FDX8_CurrentViewMtx );
			XGMatrixTranspose( &matWorldView, &matTemp );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEW_0, &matWorldView(0, 0), 4 );
				
			// Projection to clip space
			XGMatrixMultiply( &matWorldViewProj, &matTemp, &FDX8_CurrentProjMtx );
			XGMatrixTranspose( &matWorldViewProj, &matWorldViewProj );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4 );
			//
		#else
			//This is for the vertex shaders
			D3DXMATRIX matTemp, matWorldViewProj, matWorldView, World;
			D3DXMatrixTranspose( &World, &Mtx );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );

			D3DXMatrixMultiply( &matTemp, &Mtx, &FDX8_CurrentViewMtx );
			D3DXMatrixTranspose( &matWorldView, &matTemp );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEW_0, &matWorldView(0, 0), 4 );
				
			// Projection to clip space
			D3DXMatrixMultiply( &matWorldViewProj, &matTemp, &FDX8_CurrentProjMtx );
			D3DXMatrixTranspose( &matWorldViewProj, &matWorldViewProj );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4 );
			//
		#endif
	}
	else
	{
		FDX8_pDev->SetTransform( nD3DMatrixType, &Mtx );
	}
}


//
//
//
void fdx8xfm_SetCustomDXMatrix( D3DTRANSFORMSTATETYPE nD3DMatrixType, const CFMtx44A *pMtx44A, BOOL bUsingVertexShaders ) 
{
	FASSERT( _bWindowCreated );

	if ( nD3DMatrixType == D3DTS_WORLDMATRIX(0) && bUsingVertexShaders )
	{
		#if FANG_PLATFORM_XB
			// This is for the vertex shaders
			XGMATRIX Proj, matTemp, matWorldViewProj, matWorldView, World;
			XGMatrixTranspose( &World, (XGMATRIX *)pMtx44A );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );

			XGMatrixMultiply( &matTemp, (XGMATRIX *)pMtx44A, &FDX8_CurrentViewMtx );
			XGMatrixTranspose( &matWorldView, &matTemp);
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEW_0, &matWorldView(0, 0), 4 );
			XGMatrixMultiply( &matWorldViewProj, &matTemp, &FDX8_CurrentProjMtx );
				
			// Projection to clip space
			XGMatrixTranspose( &matWorldViewProj, &matWorldViewProj );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4 );
			//
		#else
			// This is for the vertex shaders
			D3DXMATRIX Proj, matTemp, matWorldViewProj, matWorldView, World;
			D3DXMatrixTranspose( &World, (D3DXMATRIX *)pMtx44A );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLD_0, &World(0, 0), 4 );

			D3DXMatrixMultiply( &matTemp, (D3DXMATRIX *)pMtx44A, &FDX8_CurrentViewMtx );
			D3DXMatrixTranspose( &matWorldView, &matTemp);
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEW_0, &matWorldView(0, 0), 4 );
			D3DXMatrixMultiply( &matWorldViewProj, &matTemp, &FDX8_CurrentProjMtx );
				
			// Projection to clip space
			D3DXMatrixTranspose( &matWorldViewProj, &matWorldViewProj );
			FDX8_pDev->SetVertexShaderConstant( CV_WORLDVIEWPROJ_0, &matWorldViewProj(0, 0), 4 );
			//
		#endif
	}
	else
	{
		FDX8_pDev->SetTransform( nD3DMatrixType, (D3DMATRIX *)pMtx44A );
	}
}


//
//
//
static FINLINE void _ConvertXfmToD3DMtx( D3DMATRIX *pDestD3DMtx, const CFMtx43 *pMtx43 ) 
{
	pDestD3DMtx->_11 = pMtx43->aa[0][0];
	pDestD3DMtx->_12 = pMtx43->aa[0][1];
	pDestD3DMtx->_13 = pMtx43->aa[0][2];
	pDestD3DMtx->_14 = 0.0f;
	pDestD3DMtx->_21 = pMtx43->aa[1][0];
	pDestD3DMtx->_22 = pMtx43->aa[1][1];
	pDestD3DMtx->_23 = pMtx43->aa[1][2];
	pDestD3DMtx->_24 = 0.0f;
	pDestD3DMtx->_31 = pMtx43->aa[2][0];
	pDestD3DMtx->_32 = pMtx43->aa[2][1];
	pDestD3DMtx->_33 = pMtx43->aa[2][2];
	pDestD3DMtx->_34 = 0.0f;
	pDestD3DMtx->_41 = pMtx43->aa[3][0];
	pDestD3DMtx->_42 = pMtx43->aa[3][1];
	pDestD3DMtx->_43 = pMtx43->aa[3][2];
	pDestD3DMtx->_44 = 1.0f;
}


//
//
//
static FINLINE void _ConvertXfmToD3DMtx( D3DMATRIX *pDestD3DMtx, const CFMtx43A *pMtx43A ) 
{
	pDestD3DMtx->_11 = pMtx43A->aa[0][0];
	pDestD3DMtx->_12 = pMtx43A->aa[0][1];
	pDestD3DMtx->_13 = pMtx43A->aa[0][2];
	pDestD3DMtx->_14 = 0.0f;
	pDestD3DMtx->_21 = pMtx43A->aa[1][0];
	pDestD3DMtx->_22 = pMtx43A->aa[1][1];
	pDestD3DMtx->_23 = pMtx43A->aa[1][2];
	pDestD3DMtx->_24 = 0.0f;
	pDestD3DMtx->_31 = pMtx43A->aa[2][0];
	pDestD3DMtx->_32 = pMtx43A->aa[2][1];
	pDestD3DMtx->_33 = pMtx43A->aa[2][2];
	pDestD3DMtx->_34 = 0.0f;
	pDestD3DMtx->_41 = pMtx43A->aa[3][0];
	pDestD3DMtx->_42 = pMtx43A->aa[3][1];
	pDestD3DMtx->_43 = pMtx43A->aa[3][2];
	pDestD3DMtx->_44 = 1.0f;
}


//
//
//
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FDX8VID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;

			CFXfm::InitStack();
			fdx8xfm_SetDXMatrices( TRUE );
			break;

		case FDX8VID_EVENT_WINDOW_DESTROYED:
			_bWindowCreated = FALSE;
			break;

		case FDX8VID_EVENT_PRE_RESET:
			break;

		case FDX8VID_EVENT_POST_RESET:
			break;
	}

	return TRUE;
}


