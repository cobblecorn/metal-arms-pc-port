//////////////////////////////////////////////////////////////////////////////////////
// fdx8psprite.cpp - Fang point sprite module (DX version).
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
// 05/31/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fpsprite.h"
#include "fxfm.h"
#include "fclib.h"
#include "frenderer.h"
#include "fperf.h"
#include "fdraw.h"
#include "fexception.h"
#include "fvis.h"

#include "fdx8.h"
#include "fdx8vid.h"
#include "fdx8vb.h"
#include "fdx8tex.h"
#include "fdx8viewport.h"
#include "fdx8sh.h"
#include "fdx8xfm.h"

// Include compiled shaders:
#if FANG_PLATFORM_WIN
	#include "win\\fdxsh_psprite.cvs"
#else
	#include "xb\\fdxsh_psprite.cvs"
#endif



BOOL FPSprite_bEmulated;		// TRUE=Emulated (not directly supported by hardware)
f32 FPSprite_fLargestSize_SS;	// The largest size (horizontal and vertical) in screen space that the hardware supports



#define _MIN_VERTEX_COUNT	32
#define _MAX_VERTEX_COUNT	65535


#if FANG_PLATFORM_WIN
	#define _TEXSTAGE		0
#else
	#define _TEXSTAGE		3
#endif


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static u32 _nMaxD3DDrawPrimElements;

static u32 _nVertexCount;
static u32 _nVBCount;

static FDX8VB_t *_pHWVB;
static u32 _nCurrentHWVB;
static u32 _nCurrentHWVBVtxIndex;

#if FANG_PLATFORM_WIN
//create additional software emulation VB(s)
static FDX8VB_t *_pEmulationVB;
static u32 _nCurrentEmulationVB;
static u32 _nCurrentEmulationVBVtxIndex;
#endif

static DWORD _hPSpriteVSH;

static CFMtx44A _ModelViewMtx44;



static BOOL _CreateWhiteTexture( void );
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );


static const DWORD _VertexShaderDecl[] = {
	D3DVSD_STREAM( 0 ),					// Stream 0:
	D3DVSD_REG( 0, D3DVSDT_FLOAT3 ),	// Position in world space
	D3DVSD_REG( 1, D3DVSDT_FLOAT1 ),	// Dimension in world units
	D3DVSD_REG( 2, D3DVSDT_FLOAT4 ),	// Color and alpha

	D3DVSD_END()
};



BOOL fpsprite_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	// Compute vertex count per VB...
	_nVertexCount = Fang_ConfigDefs.nPSprite_D3DVBVertexCount;
	if( Fang_ConfigDefs.nPSprite_D3DVBVertexCount < _MIN_VERTEX_COUNT ) {
		// Invalid VB count...
		DEVPRINTF( "fpsprite_ModuleStartup(): Fang_ConfigDefs.nPSprite_D3DVBVertexCount is invalid: %u. Must be at least %u.\n", Fang_ConfigDefs.nPSprite_D3DVBVertexCount, _MIN_VERTEX_COUNT );
		DEVPRINTF( "                          Setting to %u.\n", _MIN_VERTEX_COUNT );
		_nVertexCount = _MIN_VERTEX_COUNT;
	}
	if( Fang_ConfigDefs.nPSprite_D3DVBVertexCount > _MAX_VERTEX_COUNT ) {
		// Invalid VB count...
		DEVPRINTF( "fdx8PSprite_ModuleStartup(): Fang_ConfigDefs.nPSprite_D3DVBVertexCount is invalid: %u. Maximum is %u.\n", Fang_ConfigDefs.nPSprite_D3DVBVertexCount, _MAX_VERTEX_COUNT );
		DEVPRINTF( "                          Setting to %u.\n", _MAX_VERTEX_COUNT );
		_nVertexCount = _MAX_VERTEX_COUNT;
	}

	#if FANG_PLATFORM_XB
		// For Xbox, we create several VBs...
		_nVBCount = Fang_ConfigDefs.nPSprite_XBVBCount;

		if( Fang_ConfigDefs.nPSprite_XBVBCount < 1 ) {
			// Invalid VB count...
			DEVPRINTF( "fpsprite_ModuleStartup(): Fang_ConfigDefs.nPSprite_XBVBCount is invalid: %u. Must be at least 1.\n", Fang_ConfigDefs.nPSprite_XBVBCount );
			DEVPRINTF( "                          Setting to 1.\n" );
			_nVBCount = 1;
		}
	#else
		// For Windows, we create only one VB...
		_nVBCount = 1;
	#endif

	// Create the VB arrays...
	_pHWVB = (FDX8VB_t *)fres_AllocAndZero( _nVBCount * sizeof(FDX8VB_t) );
	if( _pHWVB == NULL ) {
		DEVPRINTF( "fpsprite_ModuleStartup(): Insufficient memory to allocate %u bytes for a HW VB array.\n", _nVBCount * sizeof(FDX8VB_t) );
		return FALSE;
	}

	#if FANG_PLATFORM_WIN
		//create the software VB array pointers as well
		_pEmulationVB = (FDX8VB_t *)fres_AllocAndZero( _nVBCount * sizeof(FDX8VB_t) );
		if( _pEmulationVB == NULL ) {
			DEVPRINTF( "fpsprite_ModuleStartup(): Insufficient memory to allocate %u bytes for a Emulation VB array.\n", _nVBCount * sizeof(FDX8VB_t) );
			return FALSE;
		}
	#endif

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_ModelViewMtx44.aa[0][3] = _ModelViewMtx44.aa[1][3] = _ModelViewMtx44.aa[2][3] = 0.0f;
	_ModelViewMtx44.aa[3][3] = 1.0f;

	FPSprite_fLargestSize_SS = 0.0f;
	FPSprite_bEmulated = TRUE;

	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;

	return TRUE;
}

void fpsprite_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

FViewportPlanesMask_t CFPSpriteGroup::Render( BOOL bPerformFrustumTest, FViewportPlanesMask_t nCrossesPlanesMask, CFMtx43A *pMirrorMtx ) {
	FDX8VB_t *pVB;
	FViewport_t *pViewport;
	FPSprite_t *pDstVtxBuf;
	const FPSprite_t *pSrcVtxBuf;
	u32 nVtxCount, nDrawVtxCount, nMaxDrawVtxCount, nRenderBufIndex;
	const CFMtx44A *pProjMtx44;
	CFMtx44A ProjViewMtx44, ViewMtx44;
	f32 fOOViewportFarMinusNearZ, fUnitFogStartZ, fUnitFogEndZ, afPointScaleAndFogFactors[4];
	CFVec3A TempVec3A;

	FASSERT( _bWindowCreated );
	FASSERT( (u32)m_pBase == FMATH_BYTE_ALIGN_UP( (u32)m_pBase, FPSPRITE_BYTE_ALIGNMENT ) );
	FASSERT( m_nRenderCount <= m_nMaxCount );
	FASSERT( m_nRenderStartIndex <= m_nMaxCount );

	// exception occurs in d3d drawing function
	fexception_DisableFloatingPointExceptions();

	if( m_nPSFlags & FPSPRITE_FLAG_DONT_DRAW ) {
		// This group flagged to not draw...
		return nCrossesPlanesMask;
	}

	if( m_nRenderCount == 0 ) {
		// No sprites to draw...
		return nCrossesPlanesMask;
	}

	// Get viewport...
	pViewport= fviewport_GetActive();
	if( pViewport == NULL ) {
		// No viewport...
		return nCrossesPlanesMask;
	}

//	m_Xfm.PushModel();

	// Perform frustum test, if required...
	if( bPerformFrustumTest ) {
		// Test group bounding sphere against frustum...

		TempVec3A.Sub( FXfm_pView->m_MtxR.m_vPos, m_Xfm.m_MtxF.m_vPos );
		if( m_fCullDist!=0.0f && TempVec3A.MagSq() > m_fCullDist * m_fCullDist ) {
			// Group is too far to render...
			return nCrossesPlanesMask;
		}

		if( nCrossesPlanesMask == -1 ) {
			// Group is outside frustum...
//			CFXfm::PopModel();
			return -1;
		}

		nCrossesPlanesMask = fviewport_TestSphere_MS( pViewport, &m_BoundSphere_MS, nCrossesPlanesMask );
		if( nCrossesPlanesMask == -1 ) {
			// Group is outside frustum...
//			CFXfm::PopModel();
			return -1;
		}
	}

	BOOL bUseEmulatedPath = FPSprite_bEmulated;

	if( !bUseEmulatedPath && ( m_fEmulationDistance != 0.0f ) ) {
		//first, we want to see if we should even bother to use the emulated path
		//if we are closer than m_fEmulationDistance, then we want to switch to the emulated
		//render path to avoid the 64x64 screen space sprite pixel limitation size.

		//We are going to use the bounding sphere position as our test, 
		//becuase the m_Xfm is always the identity!  
		//Also note that because the m_XFM is always the identity,
		//the m_BoundSphere_MS is acutally in WS coords...
		TempVec3A.Sub( m_BoundSphere_MS.m_Pos, FXfm_pView->m_MtxR.m_vPos );
		if( TempVec3A.MagSq() < m_fEmulationDistance * m_fEmulationDistance ) {
			bUseEmulatedPath = TRUE;
		}
	}

	m_Xfm.PushModel();

	if( bUseEmulatedPath ) {
		// Emulate point sprites...
		return _RenderEmulatedGroup( bPerformFrustumTest, nCrossesPlanesMask );
	}


	//if we are here, we are using the HW path

	// Compute matrix...
	_ModelViewMtx44.m_vX = FXfm_pModelView->m_MtxF.m_vX.v4a;
	_ModelViewMtx44.m_vY = FXfm_pModelView->m_MtxF.m_vY.v4a;
	_ModelViewMtx44.m_vZ = FXfm_pModelView->m_MtxF.m_vZ.v4a;
	_ModelViewMtx44.m_vPos = FXfm_pModelView->m_MtxF.m_vPos.v4a;
	_ModelViewMtx44.aa[3][3] = 1.0f;
	pProjMtx44 = fviewport_GetPlatformSpecificProjectionMatrix( pViewport );
	ProjViewMtx44.Mul( *pProjMtx44, _ModelViewMtx44 );
	ProjViewMtx44.Transpose();

	// Configure depth buffer write mode...
	fdx8_SetRenderState_ZWRITEENABLE( !!(m_nPSFlags & FPSPRITE_FLAG_ENABLE_DEPTH_WRITES) );

	//set up any HW rendering specific renderstates
	fdx8_SetRenderState_POINTSPRITEENABLE( TRUE );
	fdx8_SetRenderState_POINTSCALEENABLE( TRUE );

	// Set blend mode...
	switch( m_nBlend ) {
	case FPSPRITE_BLEND_MODULATE:
		fdx8_SetRenderState_SRCBLEND( D3DBLEND_SRCALPHA );
		fdx8_SetRenderState_DESTBLEND( D3DBLEND_INVSRCALPHA );
		break;

	case FPSPRITE_BLEND_ADD:
		fdx8_SetRenderState_SRCBLEND( D3DBLEND_ONE );
		fdx8_SetRenderState_DESTBLEND( D3DBLEND_ONE );
		break;

	default:
		FASSERT_NOW;
	}

	// Set up texture...
	if( m_TexInst.GetTexDef() ) {
		// Texture provided...
		fdx8_SetRenderState_POINTSPRITEENABLE( TRUE );
		fdx8tex_SetTexture( _TEXSTAGE, &m_TexInst, _TEXSTAGE );

		fdx8_SetTextureState_COLOROP( _TEXSTAGE, D3DTOP_MODULATE );
		fdx8_SetTextureState_COLORARG1( _TEXSTAGE, D3DTA_TEXTURE  );
		fdx8_SetTextureState_ALPHAOP( _TEXSTAGE, D3DTOP_MODULATE );
		fdx8_SetTextureState_ALPHAARG1( _TEXSTAGE, D3DTA_TEXTURE  );
	} else {
		// No texture provided...
		fdx8_SetRenderState_POINTSPRITEENABLE( FALSE );
		fdx8tex_SetTexture( _TEXSTAGE, NULL, _TEXSTAGE );

		fdx8_SetTextureState_COLOROP( _TEXSTAGE, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_COLORARG1( _TEXSTAGE, D3DTA_DIFFUSE  );
		fdx8_SetTextureState_ALPHAOP( _TEXSTAGE, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_ALPHAARG1( _TEXSTAGE, D3DTA_DIFFUSE  );
	}

	// Set fill mode...
	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	// Compute point scale factor...
	if( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) {
		// Point size is in model space units...
		afPointScaleAndFogFactors[0] = pProjMtx44->aa[0][0] * pViewport->Res.x * 0.5f;
		afPointScaleAndFogFactors[1] = 0.0f;
	} else {
		// Point size is in screen pixels...
		afPointScaleAndFogFactors[0] = 0.0f;
		afPointScaleAndFogFactors[1] = 1.0f;
	}

	// Set up fog..
	if( !FRenderer_bFogEnabled || (m_nPSFlags & FPSPRITE_FLAG_NO_FOG) ) {
		// Fog disabled...

		//fdx8_SetRenderState_FOGENABLE( FALSE );

		afPointScaleAndFogFactors[2] = 0.0f;
		afPointScaleAndFogFactors[3] = 1.0f;
	} else {
		// Fog enabled...

		frenderer_Fog_ComputeColor();
		//fdx8_SetRenderState_FOGENABLE( TRUE );
		fdx8_SetRenderState_FOGCOLOR( 0x0 );//FRenderer_FogD3DColor );

		fOOViewportFarMinusNearZ = 1.0f / (pViewport->fFarZ - pViewport->fNearZ);
		fUnitFogStartZ = (FRenderer_fFogStartZ - pViewport->fNearZ) * fOOViewportFarMinusNearZ;
		fUnitFogEndZ = (FRenderer_fFogEndZ - pViewport->fNearZ) * fOOViewportFarMinusNearZ;

		afPointScaleAndFogFactors[2] = -1.0f / (pViewport->pViewportIS->fProjScale * pViewport->fFarZ * (fUnitFogEndZ - fUnitFogStartZ));
		afPointScaleAndFogFactors[3] = fUnitFogEndZ / (fUnitFogEndZ - fUnitFogStartZ);
	}

	FDX8_pDev->SetPixelShader(0);

	// Store vertex shader constants...
	FDX8_pDev->SetVertexShaderConstant( 0, &ProjViewMtx44, sizeof(ProjViewMtx44)>>4 );
	FDX8_pDev->SetVertexShaderConstant( sizeof(ProjViewMtx44)>>4, &afPointScaleAndFogFactors, 1 );

	#if FANG_PLATFORM_XB
		// Select the starting VB for Xbox...
		if( _nCurrentHWVBVtxIndex != _nVertexCount ) {
			if( sizeof(FPSprite_t) & 0x1f ) {
				// Vertex size isn't a multiple of 32 bytes.
				// In order to append to dynamic vertex buffers, Xbox
				// requires us to start the next transfer into the
				// vertex buffer on a 32-byte boundary.
			}

			pVB = &_pHWVB[ _nCurrentHWVB ];
			fdx8vb_Select( pVB );
		}
	#else
		// Select the VB for Windows...
		FASSERT( _nCurrentHWVB == 0 );
		pVB = _pHWVB;
		fdx8vb_Select( pVB );
	#endif

	nVtxCount = m_nRenderCount;
	nRenderBufIndex = m_nRenderStartIndex;
	pSrcVtxBuf = m_pBase + nRenderBufIndex;

	while( nVtxCount ) {
		if( _nCurrentHWVBVtxIndex == _nVertexCount ) {
			// Reached the end of the current VB...
			_nCurrentHWVBVtxIndex = 0;

			#if FANG_PLATFORM_XB
				// Select the next VB for Xbox...

				if( ++_nCurrentHWVB >= _nVBCount ) {
					_nCurrentHWVB = 0;
				}

				pVB = &_pHWVB[ _nCurrentHWVB ];

				fdx8vb_BlockUntilNotBusy( pVB );
				fdx8vb_Select( pVB );

				_nCurrentHWVBVtxIndex = 0;
			#endif
		}

		// Compute the maximum number of vertices that can fit into the unused portion of our VB...
		nMaxDrawVtxCount = _nVertexCount - _nCurrentHWVBVtxIndex;
		FMATH_CLAMPMAX( nMaxDrawVtxCount, _nMaxD3DDrawPrimElements );

		// Compute the number of vertices we will draw this loop...
		nDrawVtxCount = FMATH_MIN( nVtxCount, nMaxDrawVtxCount );

		// Handle ring buffer wrapping...
		if( (nRenderBufIndex + nDrawVtxCount) > m_nMaxCount ) {
			nDrawVtxCount = m_nMaxCount - nRenderBufIndex;

			if( nDrawVtxCount == 0 ) {
				nRenderBufIndex = 0;
				pSrcVtxBuf = m_pBase;
				nDrawVtxCount = FMATH_MIN( nVtxCount, nMaxDrawVtxCount );
			}
		}

		// Copy the point sprite vertices into our VB...
		pDstVtxBuf = (FPSprite_t *)fdx8vb_Lock( pVB, _nCurrentHWVBVtxIndex, nDrawVtxCount );
		fang_MemCopy( pDstVtxBuf, pSrcVtxBuf, nDrawVtxCount*sizeof(FPSprite_t) );
		fdx8vb_Unlock( pVB );

		FDX8_pDev->DrawPrimitive( D3DPT_POINTLIST, _nCurrentHWVBVtxIndex, nDrawVtxCount );
		
		// Adjust pointers...
		nVtxCount -= nDrawVtxCount;
		pSrcVtxBuf += nDrawVtxCount;
		nRenderBufIndex += nDrawVtxCount;
		_nCurrentHWVBVtxIndex += nDrawVtxCount;
	}

	if( FRenderer_bDrawBoundInfo ) {
		// Draw group bounding sphere...
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_FacetedWireSphere( &m_BoundSphere_MS.m_Pos, m_BoundSphere_MS.m_fRadius, 2, 2 );
		fdraw_ModelSpaceAxis( 0.5f * m_BoundSphere_MS.m_fRadius );
		frenderer_Pop();
	}

	CFXfm::PopModel();

	#if FPERF_ENABLE
		FPerf_nPSGroupDrawnCount++;
		FPerf_nPSpritesDrawnCount += m_nRenderCount;
	#endif

	fexception_EnableFloatingPointExceptions();

	return nCrossesPlanesMask;
}

#if FANG_PLATFORM_WIN
FViewportPlanesMask_t CFPSpriteGroup::_RenderEmulatedGroup( BOOL bPerformFrustumTest, FViewportPlanesMask_t nCrossesPlanesMask ) {
	FDX8VB_t *pVB;
	FViewport_t *pViewport;
	FDX8VB_C1T1_t *pDstVtxBuf;
	CFVec3 Pos_VS;
	const FPSprite_t *pSrcVtxBuf;
	u32 i, nD3DColor, nQuadCount, nProcessQuadCount, nDrawQuadCount, nProcessVtxCount, nMaxDrawVtxCount, nMaxDrawQuadCount, nRenderBufIndex;
	f32 fScaleMultiplier, fHalfDim_VS, fLeftX, fRightX, fTopY, fBottomY;
	CFVec3A TempVec3A;


	// Get viewport...
	pViewport= fviewport_GetActive();

	// Configure depth buffer write mode...
	fdx8_SetRenderState_ZWRITEENABLE( !!(m_nPSFlags & FPSPRITE_FLAG_ENABLE_DEPTH_WRITES) );

	//set up any emulation specific renderstates
	fdx8_SetRenderState_POINTSPRITEENABLE( FALSE );
	fdx8_SetRenderState_POINTSCALEENABLE( FALSE );

	// Set blend mode...
	switch( m_nBlend ) {
	case FPSPRITE_BLEND_MODULATE:
		fdx8_SetRenderState_SRCBLEND( D3DBLEND_SRCALPHA );
		fdx8_SetRenderState_DESTBLEND( D3DBLEND_INVSRCALPHA );
		break;

	case FPSPRITE_BLEND_ADD:
		fdx8_SetRenderState_SRCBLEND( D3DBLEND_ONE );
		fdx8_SetRenderState_DESTBLEND( D3DBLEND_ONE );
		break;

	default:
		FASSERT_NOW;
	}

	// Set up texture...
	if( m_TexInst.GetTexDef() ) {
		// Texture provided...
		fdx8tex_SetTexture( _TEXSTAGE, &m_TexInst, _TEXSTAGE );

		fdx8_SetTextureState_COLOROP( _TEXSTAGE, D3DTOP_MODULATE );
		fdx8_SetTextureState_COLORARG1( _TEXSTAGE, D3DTA_TEXTURE  );
		fdx8_SetTextureState_COLORARG2( _TEXSTAGE, D3DTA_DIFFUSE  );
		fdx8_SetTextureState_ALPHAOP( _TEXSTAGE, D3DTOP_MODULATE );
		fdx8_SetTextureState_ALPHAARG1( _TEXSTAGE, D3DTA_TEXTURE  );
		fdx8_SetTextureState_ALPHAARG2( _TEXSTAGE, D3DTA_DIFFUSE  );
	} else {
		// No texture provided...
		fdx8tex_SetTexture( _TEXSTAGE, NULL, _TEXSTAGE );

		fdx8_SetTextureState_COLOROP( _TEXSTAGE, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_COLORARG1( _TEXSTAGE, D3DTA_DIFFUSE  );
		fdx8_SetTextureState_ALPHAOP( _TEXSTAGE, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_ALPHAARG1( _TEXSTAGE, D3DTA_DIFFUSE  );
	}

	// Set fill mode...
	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	// Set up fog..
	if( !FRenderer_bFogEnabled || (m_nPSFlags & FPSPRITE_FLAG_NO_FOG) ) {
		// Fog disabled...

		//fdx8_SetRenderState_FOGENABLE( FALSE );
	} else {
		// Fog enabled...

		frenderer_Fog_ComputeColor();
		//fdx8_SetRenderState_FOGENABLE( TRUE );
		//fdx8_SetRenderState_FOGCOLOR( FRenderer_FogD3DColor );
	}

	// Set D3D model & view matrices...
	FDX8_pDev->SetTransform( D3DTS_WORLDMATRIX(0), &FDX8_MatrixIdentity );
	FDX8_SetViewMatrixIdentity();
//	FDX8_pDev->SetTransform( D3DTS_VIEW, &FDX8_MatrixIdentity );

	// Compute model-space to view-space scale factor, and point size multiplier...
	if( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) {
		// Point size is in model space units...
		fScaleMultiplier = 0.5f * FXfm_pModelView->m_fScaleF;
	} else {
		// Point size is in screen pixels...
		fScaleMultiplier = 0.5f * FXfm_pModelView->m_fScaleF * pViewport->OOHalfRes.x;
	}

	// Select the VB for Windows...
	FASSERT( _nCurrentEmulationVB == 0 );
	pVB = _pEmulationVB;
	fdx8vb_Select( pVB );

	nQuadCount = m_nRenderCount;
	nRenderBufIndex = m_nRenderStartIndex;
	pSrcVtxBuf = m_pBase + nRenderBufIndex;

	while( nQuadCount ) {
		// Compute the maximum number of quads that can fit into the unused portion of our VB...
		nMaxDrawVtxCount = _nVertexCount - _nCurrentEmulationVBVtxIndex;
		FMATH_CLAMPMAX( nMaxDrawVtxCount, _nMaxD3DDrawPrimElements );
		nMaxDrawQuadCount = nMaxDrawVtxCount / 6;
		FASSERT( nMaxDrawQuadCount );

		// Compute the number of quads we will draw this loop...
		nProcessQuadCount = FMATH_MIN( nQuadCount, nMaxDrawQuadCount );
		nProcessVtxCount = nProcessQuadCount * 6;

		// Handle ring buffer wrapping...
		if( (nRenderBufIndex + nProcessQuadCount) > m_nMaxCount ) {
			nProcessQuadCount = m_nMaxCount - nRenderBufIndex;

			if( nProcessQuadCount == 0 ) {
				nRenderBufIndex = 0;
				pSrcVtxBuf = m_pBase;
				nProcessQuadCount = FMATH_MIN( nQuadCount, nMaxDrawQuadCount );
				nProcessVtxCount = nProcessQuadCount * 6;
			}
		}

		nDrawQuadCount = 0;
		pDstVtxBuf = (FDX8VB_C1T1_t *)fdx8vb_Lock( pVB, _nCurrentEmulationVBVtxIndex, nProcessVtxCount );
		for( i=0; i<nProcessQuadCount; i++, pSrcVtxBuf++ ) {
			// Transform PS position from model-space to view-space...
			FXfm_pModelView->TransformPointF( Pos_VS, pSrcVtxBuf->Point_MS );

			if( Pos_VS.z < pViewport->fNearZ ) {
				// This PS is behind the camera, so skip it...
				continue;
			}

			// Compute half-dimension of PS in view space...
			if( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) {
				// Point size is in model space units...
				fHalfDim_VS = pSrcVtxBuf->fDim_MS * fScaleMultiplier;
			} else {
				// Point size is in screen pixels...
				fHalfDim_VS = pSrcVtxBuf->fDim_MS * fScaleMultiplier * Pos_VS.z;
			}

			// Compute sprite edges...
			fLeftX = Pos_VS.x - fHalfDim_VS;
			fRightX = Pos_VS.x + fHalfDim_VS;
			fTopY = Pos_VS.y - fHalfDim_VS;
			fBottomY = Pos_VS.y + fHalfDim_VS;

			// Compute D3D color...
			nD3DColor = FDX8_RGBA_TO_DXCOLOR( pSrcVtxBuf->ColorRGBA );

			// Triangle #1: Lower-left vertex:
			pDstVtxBuf[0].nDiffuseRGBA = nD3DColor;
			pDstVtxBuf[0].fS0 = 0.0f;
			pDstVtxBuf[0].fT0 = 0.0f;
			pDstVtxBuf[0].fPosX = fLeftX;
			pDstVtxBuf[0].fPosY = fBottomY;
			pDstVtxBuf[0].fPosZ = Pos_VS.z;

			// Triangle #1: Upper-left vertex:
			pDstVtxBuf[1].nDiffuseRGBA = nD3DColor;
			pDstVtxBuf[1].fS0 = 0.0f;
			pDstVtxBuf[1].fT0 = 1.0f;
			pDstVtxBuf[1].fPosX = fLeftX;
			pDstVtxBuf[1].fPosY = fTopY;
			pDstVtxBuf[1].fPosZ = Pos_VS.z;

			// Triangle #1: Lower-right vertex:
			pDstVtxBuf[2].nDiffuseRGBA = nD3DColor;
			pDstVtxBuf[2].fS0 = 1.0f;
			pDstVtxBuf[2].fT0 = 0.0f;
			pDstVtxBuf[2].fPosX = fRightX;
			pDstVtxBuf[2].fPosY = fBottomY;
			pDstVtxBuf[2].fPosZ = Pos_VS.z;

			// Triangle #2: Upper-left vertex:
			pDstVtxBuf[3].nDiffuseRGBA = nD3DColor;
			pDstVtxBuf[3].fS0 = 0.0f;
			pDstVtxBuf[3].fT0 = 1.0f;
			pDstVtxBuf[3].fPosX = fLeftX;
			pDstVtxBuf[3].fPosY = fTopY;
			pDstVtxBuf[3].fPosZ = Pos_VS.z;

			// Triangle #2: Lower-right vertex:
			pDstVtxBuf[4].nDiffuseRGBA = nD3DColor;
			pDstVtxBuf[4].fS0 = 1.0f;
			pDstVtxBuf[4].fT0 = 0.0f;
			pDstVtxBuf[4].fPosX = fRightX;
			pDstVtxBuf[4].fPosY = fBottomY;
			pDstVtxBuf[4].fPosZ = Pos_VS.z;

			// Triangle #2: Upper-right vertex:
			pDstVtxBuf[5].nDiffuseRGBA = nD3DColor;
			pDstVtxBuf[5].fS0 = 1.0f;
			pDstVtxBuf[5].fT0 = 1.0f;
			pDstVtxBuf[5].fPosX = fRightX;
			pDstVtxBuf[5].fPosY = fTopY;
			pDstVtxBuf[5].fPosZ = Pos_VS.z;

			// Next quad...
			nDrawQuadCount++;
			pDstVtxBuf += 6;
		}
		fdx8vb_Unlock( pVB );

		// Draw the point sprites...
		if( nDrawQuadCount > 0 ) {
			FDX8_pDev->DrawPrimitive( D3DPT_TRIANGLELIST, _nCurrentEmulationVBVtxIndex, nDrawQuadCount<<1 );
		}

		// Adjust pointers...
		nQuadCount -= nProcessQuadCount;
		nRenderBufIndex += nProcessQuadCount;
		_nCurrentEmulationVBVtxIndex += (nDrawQuadCount * 6);
		if( (_nVertexCount - _nCurrentEmulationVBVtxIndex) < 6 ) {
			// Reached the end of this VB...
			_nCurrentEmulationVBVtxIndex = 0;
		}
	}

	if( FRenderer_bDrawBoundInfo ) {
		// Draw group bounding sphere...
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_FacetedWireSphere( &m_BoundSphere_MS.m_Pos, m_BoundSphere_MS.m_fRadius, 2, 2 );
		fdraw_ModelSpaceAxis( 0.5f * m_BoundSphere_MS.m_fRadius );
		frenderer_Pop();
	}

	CFXfm::PopModel();

	// The emulated quads are in view space. Restore the camera before the
	// sorted list draws world-space translucencies (for example waterfalls).
	fdx8xfm_SetViewDXMatrix( TRUE );

	#if FPERF_ENABLE
		FPerf_nPSGroupDrawnCount++;
		FPerf_nPSpritesDrawnCount += m_nRenderCount;
	#endif

	return nCrossesPlanesMask;
}
#endif



#if FANG_PLATFORM_XB
FViewportPlanesMask_t CFPSpriteGroup::_RenderEmulatedGroup( BOOL bPerformFrustumTest, FViewportPlanesMask_t nCrossesPlanesMask ) 
{
	#define __DWORDS_PER_VERTEX			6
	#define __MAX_VERTICES_PER_ARRAY	340   // We can submit a max of 2047 DWORDS of data at a time

	FViewport_t *pViewport;
	CFVec3 Pos_VS;
	const FPSprite_t *pSrcVtxBuf;
	u32 i, nQuadCount, nRenderBufIndex;
	f32 fScaleMultiplier, fHalfDim_VS, fLeftX, fRightX, fTopY, fBottomY;
	CFVec3A TempVec3A;

	// Get the viewport...
	pViewport= fviewport_GetActive();

	// Configure depth buffer write mode...
	fdx8_SetRenderState_ZWRITEENABLE( !!(m_nPSFlags & FPSPRITE_FLAG_ENABLE_DEPTH_WRITES) );

	//set up any emulation specific renderstates
	fdx8_SetRenderState_POINTSPRITEENABLE( FALSE );
	fdx8_SetRenderState_POINTSCALEENABLE( FALSE );

	// Set blend mode...
	switch( m_nBlend ) 
	{
		case FPSPRITE_BLEND_MODULATE:
			fdx8_SetRenderState_SRCBLEND( D3DBLEND_SRCALPHA );
			fdx8_SetRenderState_DESTBLEND( D3DBLEND_INVSRCALPHA );
			break;

		case FPSPRITE_BLEND_ADD:
			fdx8_SetRenderState_SRCBLEND( D3DBLEND_ONE );
			fdx8_SetRenderState_DESTBLEND( D3DBLEND_ONE );
			break;

		default:
			FASSERT_NOW;
	}

	// Set up texture...
	if( m_TexInst.GetTexDef() ) 
	{
		// Texture provided...
		fdx8tex_SetTexture( 0/*_TEXSTAGE*/, &m_TexInst, 0/*_TEXSTAGE*/ );

		fdx8_SetTextureState_COLOROP( 0/*_TEXSTAGE*/, D3DTOP_MODULATE );
		fdx8_SetTextureState_COLORARG1( 0/*_TEXSTAGE*/, D3DTA_TEXTURE  );
		fdx8_SetTextureState_COLORARG2( 0/*_TEXSTAGE*/, D3DTA_DIFFUSE  );
		fdx8_SetTextureState_ALPHAOP( 0/*_TEXSTAGE*/, D3DTOP_MODULATE );
		fdx8_SetTextureState_ALPHAARG1( 0/*_TEXSTAGE*/, D3DTA_TEXTURE  );
		fdx8_SetTextureState_ALPHAARG2( 0/*_TEXSTAGE*/, D3DTA_DIFFUSE  );
	} 
	else 
	{
		// No texture provided...
		fdx8tex_SetTexture( 0/*_TEXSTAGE*/, NULL, _TEXSTAGE );

		fdx8_SetTextureState_COLOROP( 0/*_TEXSTAGE*/, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_COLORARG1( 0/*_TEXSTAGE*/, D3DTA_DIFFUSE  );
		fdx8_SetTextureState_ALPHAOP( 0/*_TEXSTAGE*/, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_ALPHAARG1( 0/*_TEXSTAGE*/, D3DTA_DIFFUSE  );
	}

	// Set fill mode...
	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	// Set up fog..
	if( !FRenderer_bFogEnabled || (m_nPSFlags & FPSPRITE_FLAG_NO_FOG) ) 
	{
		// Fog disabled...

		//fdx8_SetRenderState_FOGENABLE( FALSE );
	} 
	else 
	{
		// Fog enabled...

		frenderer_Fog_ComputeColor();
		//fdx8_SetRenderState_FOGENABLE( TRUE );
		//fdx8_SetRenderState_FOGCOLOR( FRenderer_FogD3DColor );
	}

	// Set D3D model & view matrices...
	FDX8_pDev->SetTransform( D3DTS_WORLDMATRIX(0), &FDX8_MatrixIdentity );
//	FDX8_pDev->SetTransform( D3DTS_VIEW, &FDX8_MatrixIdentity );
	FDX8_SetViewMatrixIdentity();

	// Needed because the xbox still needs to know how to interperet the data
	// thats comes down the xbox pipe...  Also, becuase the shader pipeline caches
	// which vertex buffer is currenlty in use, we should clear the cached vertex
	// buffer so that the next shader (or the next hardware psprite group) that gets
	// rasterized does not assume the hardware vertex buffer is still set up in the
	// hardware pipeline as the current vertex buffer.
	FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );
	fdx8sh_SetVertexType( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );
	fdx8vb_UncacheSelected();

	// Compute model-space to view-space scale factor, and point size multiplier...
	if( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) 
	{
		// Point size is in model space units...
		fScaleMultiplier = 0.5f * FXfm_pModelView->m_fScaleF;
	} 
	else 
	{
		// Point size is in screen pixels...
		fScaleMultiplier = 0.5f * FXfm_pModelView->m_fScaleF * pViewport->OOHalfRes.x;
	}

	nQuadCount = m_nRenderCount;
	nRenderBufIndex = m_nRenderStartIndex;
	pSrcVtxBuf = m_pBase + nRenderBufIndex;

	u16 nVtxCount = nQuadCount * 4;

	u32 uPushSizeInDWORDs = (__DWORDS_PER_VERTEX * nVtxCount);

	u16 uArrayCount, uCurrentArrayVertCount = __MAX_VERTICES_PER_ARRAY;
	if ( nVtxCount <= __MAX_VERTICES_PER_ARRAY )
	{
		uArrayCount = 1;
	}
	else
	{
		uArrayCount = (nVtxCount + __MAX_VERTICES_PER_ARRAY) / __MAX_VERTICES_PER_ARRAY;
	}

	FASSERT( uArrayCount > 0 );

	// Gain direct access to the pushbuffer. Note that we have 
	// to reserve overhead for the encoding parameters.
	DWORD *pPush, *pArrayStart = NULL;;
	if ( FDX8_pDev->BeginPush( uPushSizeInDWORDs + 4 + uArrayCount, &pPush ) == S_OK )
	{
		// Push the macro that start things off
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
		
		// Specify the primitive type of the vertices that follow
		*pPush++ = D3DPT_QUADLIST;

		u32 nD3DColor;
		for ( i = 0; i < nQuadCount; i++, pSrcVtxBuf++ ) 
		{
			if ( uCurrentArrayVertCount + 4 > __MAX_VERTICES_PER_ARRAY )
			{
				// If we just filled in an array, fill out the encoding
				if ( pArrayStart )
				{
					*pArrayStart = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG|D3DPUSH_INLINE_ARRAY, (uCurrentArrayVertCount * __DWORDS_PER_VERTEX) );
				}

				// Record the start of this array so we can later fill in the encoding for the array
				pArrayStart = pPush;
				pPush++;

				// Reset the vertex count
				uCurrentArrayVertCount = 0;
			}

			// Transform PS position from model-space to view-space...
			FXfm_pModelView->TransformPointF( Pos_VS, pSrcVtxBuf->Point_MS );

			if( Pos_VS.z < pViewport->fNearZ ) 
			{
				// This PS is behind the camera, so skip it...
				continue;
			}

			// Compute half-dimension of PS in view space...
			if( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) 
			{
				// Point size is in model space units...
				fHalfDim_VS = pSrcVtxBuf->fDim_MS * fScaleMultiplier;
			} 
			else 
			{
				// Point size is in screen pixels...
				fHalfDim_VS = pSrcVtxBuf->fDim_MS * fScaleMultiplier * Pos_VS.z;
			}

			// Compute sprite edges...
			fLeftX = Pos_VS.x - fHalfDim_VS;
			fRightX = Pos_VS.x + fHalfDim_VS;
			fTopY = Pos_VS.y - fHalfDim_VS;
			fBottomY = Pos_VS.y + fHalfDim_VS;

			nD3DColor =  ((u8)(pSrcVtxBuf->ColorRGBA.fAlpha * 255)<<24) | ((u8)(pSrcVtxBuf->ColorRGBA.fRed * 255)<<16) | ((u8)(pSrcVtxBuf->ColorRGBA.fGreen * 255)<<8) | ((u8)(pSrcVtxBuf->ColorRGBA.fBlue * 255));

			// VERTEX 0 //

			// Write the position
			*((f32*)pPush) = fLeftX;
			pPush++;
			*((f32*)pPush) = fBottomY;
			pPush++;
			*((f32*)pPush) = Pos_VS.z;
			pPush++;

			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = 0.f;
			pPush++;
			*((f32*)pPush) = 0.f;
			pPush++;

			// VERTEX 1 //

			// Write the position
			*((f32*)pPush) = fLeftX;
			pPush++;
			*((f32*)pPush) = fTopY;
			pPush++;
			*((f32*)pPush) = Pos_VS.z;
			pPush++;
	 
			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = 0.f;
			pPush++;
			*((f32*)pPush) = 1.f;
			pPush++;

			// VERTEX 2 //

			// Write the position
			*((f32*)pPush) = fRightX;
			pPush++;
			*((f32*)pPush) = fTopY;
			pPush++;
			*((f32*)pPush) = Pos_VS.z;
			pPush++;

			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = 1.f;
			pPush++;
			*((f32*)pPush) = 1.f;
			pPush++;

			// VERTEX 3 //

			// Write the position
			*((f32*)pPush) = fRightX;
			pPush++;
			*((f32*)pPush) = fBottomY;
			pPush++;
			*((f32*)pPush) = Pos_VS.z;
			pPush++;

			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = 1.f;
			pPush++;
			*((f32*)pPush) = 0.f;
			pPush++;

			// Increment the vertex count
			uCurrentArrayVertCount += 4;
		}

		// Fill in the count for the final array
		FASSERT( pArrayStart );
		*pArrayStart = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG|D3DPUSH_INLINE_ARRAY, (uCurrentArrayVertCount * __DWORDS_PER_VERTEX) );

		// Push the macros that finish things off
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
		*pPush++ = 0;

		FDX8_pDev->EndPush( pPush );
	}
	#if !FANG_PRODUCTION_BUILD
	else
	{
		DEVPRINTF( "PUSH_START FAILED!!!\n" );
	}
	#endif

	if( FRenderer_bDrawBoundInfo ) 
	{
		// Draw group bounding sphere...
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_FacetedWireSphere( &m_BoundSphere_MS.m_Pos, m_BoundSphere_MS.m_fRadius, 2, 2 );
		fdraw_ModelSpaceAxis( 0.5f * m_BoundSphere_MS.m_fRadius );
		frenderer_Pop();
	}

	CFXfm::PopModel();

	// Return the appropriate view matrix
	fdx8xfm_SetViewDXMatrix( TRUE );

	#if FPERF_ENABLE
		FPerf_nPSGroupDrawnCount++;
		FPerf_nPSpritesDrawnCount += m_nRenderCount;
	#endif

	return nCrossesPlanesMask;

	#undef __DWORDS_PER_VERTEX
	#undef __MAX_VERTICES_PER_ARRAY
}
#endif

//
//
//
void fpsprite_Renderer_Open( void ) 
{
	FASSERT( _bModuleInitialized );

	// Init render states...
	fdx8_SetRenderState_ALPHABLENDENABLE( TRUE );
	fdx8_SetRenderState_ALPHATESTENABLE( TRUE );
	fdx8_SetRenderState_ALPHAREF( 5 );
	fdx8_SetRenderState_ALPHAFUNC( D3DCMP_GREATEREQUAL );
	fdx8_SetRenderState_SHADEMODE( D3DSHADE_GOURAUD );
	fdx8_SetRenderState_SPECULARENABLE( FALSE );
	fdx8_SetRenderState_LIGHTING( FALSE );
	fdx8_SetRenderState_LOCALVIEWER( TRUE );
	fdx8_SetRenderState_NORMALIZENORMALS( FALSE );
	fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
	fdx8_SetRenderState_BLENDOP( D3DBLENDOP_ADD );
	//fdx8_SetRenderState_FOGENABLE( FALSE );
	//fdx8_SetRenderState_FOGTABLEMODE( D3DFOG_NONE );
	//fdx8_SetRenderState_RANGEFOGENABLE( FALSE );
	fdx8_SetRenderState_ZBIAS( 0 );
	fdx8_SetRenderState_CULLMODE( D3DCULL_NONE );
	fdx8_SetRenderState_ZFUNC( D3DCMP_LESSEQUAL );
	if( FDX8_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE ) 
	{
		fdx8_SetRenderState_COLORWRITEENABLE( D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );
	}
	fdx8_SetRenderState_STENCILENABLE( FALSE );
	fdx8_SetRenderState_STENCILFUNC( D3DCMP_ALWAYS );
	fdx8_SetRenderState_STENCILFAIL( D3DSTENCILOP_KEEP );
	fdx8_SetRenderState_STENCILZFAIL( D3DSTENCILOP_KEEP );
	fdx8_SetRenderState_STENCILPASS( D3DSTENCILOP_KEEP );
	fdx8_SetRenderState_STENCILREF( 0 );
	fdx8_SetRenderState_STENCILMASK( 0xffffffff );
	fdx8_SetRenderState_STENCILWRITEMASK( 0xffffffff );

	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_FOGVERTEXMODE( D3DFOG_LINEAR );
		fdx8_SetRenderState_CLIPPING( TRUE );

		//clamp the max point sprite size to 64 for WYSIWYG XBOX emulation
		//(Geforce4 + hardware supports pixel sprite sizes above 64x64 which
		// is in general nice, but not nice for authoring sprite artwork for the XBOX )
		f32 fSixtyFour = 64.0f;
		fdx8_SetRenderState_POINTSIZE_MAX( *((u32*)&fSixtyFour ) );
	#endif

	// Init texture stage states...
	for( u32 i=0; i<FDX8_nStageCount; i++ ) 
	{
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(i, D3DTTFF_DISABLE);//COUNT2);
		fdx8_SetTextureState_COLOROP( i, D3DTOP_DISABLE );
		fdx8_SetTextureState_ALPHAOP( i, D3DTOP_DISABLE );
		fdx8tex_SetTexture( i, NULL, i );
	}
	fdx8_SetTextureState_COLORARG2( _TEXSTAGE, D3DTA_DIFFUSE  );
	fdx8_SetTextureState_ALPHAARG2( _TEXSTAGE, D3DTA_DIFFUSE  );

	FDX8_pDev->SetPixelShader( 0 );
	fdx8sh_SetVertexType( 0 );  // Clear the shader vertex type cache
}

void fpsprite_Renderer_Close( void ) 
{
	FASSERT( _bModuleInitialized );
}

u32 fpsprite_Renderer_GetStateSize( void ) {
	FASSERT( _bModuleInitialized );
	return 0;
}

void fpsprite_Renderer_GetState( void *pDestState ) {
	FASSERT( _bModuleInitialized );
}

void fpsprite_Renderer_SetState( const void *pState ) {
	FASSERT( _bModuleInitialized );
}

void fpsprite_Renderer_SetDefaultState( void ) {
	FASSERT( _bModuleInitialized );
}


static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) {
	DWORD nUsage;
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) {
	case FDX8VID_EVENT_WINDOW_CREATED:
		_bWindowCreated = TRUE;
		FPSprite_bEmulated = TRUE;

		if( FDX8_Caps.MaxPointSize > 1.0f ) {
			// Device supports point sizes...
			FPSprite_bEmulated = FALSE;
		}

		if( FPSprite_bEmulated ) { //we are going software only
			// Create vertex buffers...
#if FANG_PLATFORM_WIN
			for( i=0; i<_nVBCount; i++ ) {
				if( !fdx8vb_CreateFVF( &_pEmulationVB[i], _nVertexCount, FDX8VB_TYPE_C1T1, TRUE, FALSE ) ) {
					// Unable to create VB...
					DEVPRINTF( "fdx8psprite.cpp::_WindowCreatedCallback(): Could not create VB.\n" );
					_bWindowCreated = FALSE;
					return FALSE;
				}
			}
#endif
		} else {
			// Create vertex buffers...
			for( i=0; i<_nVBCount; i++ ) {
				if( !fdx8vb_CreateVSH( &_pHWVB[i], _nVertexCount, sizeof(FPSprite_t), TRUE, TRUE ) ) {
					// Unable to create VB...
					DEVPRINTF( "fdx8psprite.cpp::_WindowCreatedCallback(): Could not create VB.\n" );
					_bWindowCreated = FALSE;
					return FALSE;
				}
			}

			// Create vertex shader...
			#if FANG_PLATFORM_XB
				nUsage = 0;
			#else
				nUsage = FDX8_bHardwareVertexShaders ? 0 : D3DUSAGE_SOFTWAREPROCESSING;
			#endif
			if( FAILED( FDX8_pDev->CreateVertexShader( _VertexShaderDecl, dwFdxsh_pspriteVertexShader, &_hPSpriteVSH, nUsage ) ) ) {
				// Unable to compile shader...
				DEVPRINTF( "fdx8psprite.cpp::_WindowCreatedCallback(): Could not compile vertex shader.\n" );
				_bWindowCreated = FALSE;
				return FALSE;
			}

			// Attach VBs to our shader...
			for( i=0; i<_nVBCount; i++ ) {
				fdx8vb_AttachToVertexShader( &_pHWVB[i], _hPSpriteVSH );
			}


#if FANG_PLATFORM_WIN
			//if we are running under windows, create additional software VBS for the software pipe.
			// Create vertex buffers...
			for( i=0; i<_nVBCount; i++ ) {
				if( !fdx8vb_CreateFVF( &_pEmulationVB[i], _nVertexCount, FDX8VB_TYPE_C1T1, TRUE, FALSE ) ) {
					// Unable to create VB...
					DEVPRINTF( "fdx8psprite.cpp::_WindowCreatedCallback(): Could not create VB.\n" );
					_bWindowCreated = FALSE;
					return FALSE;
				}
			}
			_nCurrentEmulationVB = 0;
			_nCurrentEmulationVBVtxIndex = 0;
#endif
			_nCurrentHWVB = 0;
			_nCurrentHWVBVtxIndex = 0;
		}

		// Compute the maximum number of elements we can draw with DrawPrimitive()...
		_nMaxD3DDrawPrimElements = FDX8_Caps.MaxPrimitiveCount;

		// Compute the maximum point size dimension...
		FPSprite_fLargestSize_SS = FPSprite_bEmulated ? 0.0f : FDX8_Caps.MaxPointSize;

		break;

	case FDX8VID_EVENT_WINDOW_DESTROYED:
		if( !FPSprite_bEmulated ) {
			FDX8_pDev->DeleteVertexShader( _hPSpriteVSH );
		}

		_bWindowCreated = FALSE;

		break;

	case FDX8VID_EVENT_PRE_RESET:
		break;

	case FDX8VID_EVENT_POST_RESET:
		_nCurrentHWVB = 0;
		_nCurrentHWVBVtxIndex = 0;
#if FANG_PLATFORM_WIN
		_nCurrentEmulationVB = 0;
		_nCurrentEmulationVBVtxIndex = 0;
#endif
		break;
	}

	return TRUE;
}

