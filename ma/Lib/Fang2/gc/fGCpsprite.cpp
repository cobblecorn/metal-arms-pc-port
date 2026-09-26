//////////////////////////////////////////////////////////////////////////////////////
// fGCpsprite.cpp - Fang point sprite module (GameCube version).
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
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fpsprite.h"
#include "fxfm.h"
#include "fclib.h"
#include "frenderer.h"
#include "fperf.h"
#include "fdraw.h"
#include "fshaders.h"

#include "fGC.h"
#include "fGCvid.h"
#include "fGCvb.h"
#include "fGCtex.h"
#include "fGCviewport.h"
#include "fGCxfm.h"


BOOL FPSprite_bEmulated;		// TRUE=Emulated (not directly supported by hardware)
f32 FPSprite_fLargestSize_SS;	// The largest size (horizontal and vertical) in screen space that the hardware supports



#define _MIN_VERTEX_COUNT	32
#define _MAX_VERTEX_COUNT	65535


#define _TEXSTAGE		0


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static u32 _nMaxD3DDrawPrimElements;

static u32 _nVertexCount;
static u32 _nVBCount;

static u32 _hPSpriteVSH;

static CFVec3A *_pavPSPos;
static u32 _nMaximumSpriteCountPerGroup;

static CFMtx44A _ModelViewMtx44;



static BOOL _CreateWhiteTexture( void );
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );


//
//
//
BOOL fpsprite_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_ModelViewMtx44.aa[0][3] = _ModelViewMtx44.aa[1][3] = _ModelViewMtx44.aa[2][3] = 0.0f;
	_ModelViewMtx44.aa[3][3] = 1.0f;

	_nMaximumSpriteCountPerGroup = Fang_ConfigDefs.nMaxParticleEmitterSprites;
	_pavPSPos = (CFVec3A *)fres_Alloc( sizeof( CFVec3A ) * _nMaximumSpriteCountPerGroup );
	if ( !_pavPSPos )
	{
		DEVPRINTF( "fpsprite_ModuleStartup() - Unable to allocate memory to track sprite group positions.\n" );
		return FALSE;
	}

	FPSprite_fLargestSize_SS = 0.0f;
	FPSprite_bEmulated = TRUE;

	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fpsprite_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
FViewportPlanesMask_t CFPSpriteGroup::Render( BOOL bPerformFrustumTest, FViewportPlanesMask_t nCrossesPlanesMask, CFMtx43A *pMirrorMtx ) 
{
	FViewport_t *pViewport;
	const FPSprite_t *pSrcVtxBuf;
	u32 nRenderBufIndex;
	CFMtx44A ProjViewMtx44, ViewMtx44;
	f32 fScaleMultiplier;

	FASSERT( _bWindowCreated );
	FASSERT( m_nRenderCount <= m_nMaxCount );
	FASSERT( m_nRenderStartIndex <= m_nMaxCount );

	if ( m_nPSFlags & FPSPRITE_FLAG_DONT_DRAW ) 
	{
		// This group flagged to not draw...
		return nCrossesPlanesMask;
	}

	if ( m_nRenderCount == 0 ) 
	{
		// No sprites to draw...
		return nCrossesPlanesMask;
	}

	// Get viewport...
	pViewport= fviewport_GetActive();
	if ( pViewport == NULL ) 
	{
		// No viewport...
		return nCrossesPlanesMask;
	}

	m_Xfm.PushModel();

	// Perform frustum test, if required...
	if( bPerformFrustumTest ) 
	{
		// Test group bounding sphere against frustum...
		CFVec3A vDiff, vPos( m_Xfm.m_MtxF.m_vPos );
		if ( pMirrorMtx )
		{
			pMirrorMtx->MulPoint( vPos );
		}
			
		if ( m_fCullDist!=0.0f && vDiff.Sub( FXfm_pView->m_MtxR.m_vPos, vPos).MagSq() > m_fCullDist * m_fCullDist ) 
		{
			// Group is too far to render...
			return nCrossesPlanesMask;
		}

		if ( nCrossesPlanesMask == -1 ) 
		{
			// Group is outside frustum...
			m_Xfm.PopModel();
			return -1;
		}

		nCrossesPlanesMask = fviewport_TestSphere_MS( pViewport, &m_BoundSphere_MS, nCrossesPlanesMask );
		if ( nCrossesPlanesMask == -1 ) 
		{
			// Group is outside frustum...
			m_Xfm.PopModel();
			return -1;
		}
	}

	fsh_UseDynamicRegisters();
	fsh_SetSurfaceRegister( 0, (u32)&m_TexInst );
	fsh_SetSurfaceRegister( 1, 0 );
	fsh_SetShaderType( SHADERTYPE_SURFACE );
	fsh_SetShader( FSHADERS_DIFFUSETEX_AIAT, FSH_INVALID_LIGHT_SHADER, FSH_INVALID_SPECULAR_SHADER );
	fsh_ExecuteCurrent();
	
	// Configure depth buffer write mode...
	fgc_SetZWriteMode( !!(m_nPSFlags & FPSPRITE_FLAG_ENABLE_DEPTH_WRITES) );

	// Set blend mode...
	switch( m_nBlend ) 
	{
		case FPSPRITE_BLEND_MODULATE:
			fgc_SetBlendOp( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP );
			break;

		case FPSPRITE_BLEND_ADD:
			fgc_SetBlendOp( GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP );
			break;

		default:
			FASSERT_NOW;
	}

	// Compute model-space to view-space scale factor, and point size multiplier...
	if ( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) 
	{
		// Point size is in model space units...
		fScaleMultiplier = 0.4f * FXfm_pModelView->m_fScaleF;
	} 
	else 
	{
		// Point size is in screen pixels...
		fScaleMultiplier = 0.4f * FXfm_pModelView->m_fScaleF * pViewport->OOHalfRes.x;
	}

#if 0
	// Set up fog..
	if ( !FRenderer_bFogEnabled || (m_nPSFlags & FPSPRITE_FLAG_NO_FOG) ) 
	{
		// Fog disabled...

		fdx8_SetRenderState_FOGENABLE( FALSE );

		afPointScaleAndFogFactors[2] = 0.0f;
		afPointScaleAndFogFactors[3] = 1.0f;
	} 
	else 
	{
		// Fog enabled...

		frenderer_Fog_ComputeColor();
		FASSERT_NOW;
		fdx8_SetRenderState_FOGENABLE( TRUE );
		fdx8_SetRenderState_FOGCOLOR( FRenderer_FogD3DColor );

		fOOViewportFarMinusNearZ = 1.0f / (pViewport->fFarZ - pViewport->fNearZ);
		fUnitFogStartZ = (FRenderer_fFogStartZ - pViewport->fNearZ) * fOOViewportFarMinusNearZ;
		fUnitFogEndZ = (FRenderer_fFogEndZ - pViewport->fNearZ) * fOOViewportFarMinusNearZ;

		afPointScaleAndFogFactors[2] = -1.0f / (pViewport->pViewportIS->fProjScale * pViewport->fFarZ * (fUnitFogEndZ - fUnitFogStartZ));
		afPointScaleAndFogFactors[3] = fUnitFogEndZ / (fUnitFogEndZ - fUnitFogStartZ);
	}
#endif

	u32 nQuadCount = m_nRenderCount;
	nRenderBufIndex = m_nRenderStartIndex;
	pSrcVtxBuf = m_pBase + nRenderBufIndex;

	fgcxfm_SetGCMatricesIdentity();

	// Setup the modelview matrix
	CFMtx43A mtxModelView;
	if ( pMirrorMtx )
	{
		mtxModelView.Mul( FXfm_pView->m_MtxF, *pMirrorMtx );
		mtxModelView.Mul( FXfm_pModel->m_MtxF );
	}
	else
	{
		mtxModelView.Set( FXfm_pModelView->m_MtxF );
	}
	
	FASSERT( _nMaximumSpriteCountPerGroup >= nQuadCount );

	// Determine the z position and how many actual quads we need to render	
	u16 i, nQuadCountToRender = 0;
	CFVec3A *paPSTransPos = _pavPSPos;
	for ( i = 0; i < nQuadCount; pSrcVtxBuf++, i++, paPSTransPos++ )
	{
		// Handle ring buffer wrapping
		if ( i + m_nRenderStartIndex >= m_nMaxCount )
		{
			pSrcVtxBuf = m_pBase;			
		}
		
		// Transform PS position from model-space to view-space...
		paPSTransPos->Set( pSrcVtxBuf->Point_MS );
		mtxModelView.MulPoint( *paPSTransPos );

		// If this PS is behind the camera, skip it...
		if ( paPSTransPos->z < pViewport->fNearZ ) 
		{
			continue;
		}
		
		// Increment our count of actual quads to render
		nQuadCountToRender++;
	}
	
	// Crank up the renderer	
	paPSTransPos = _pavPSPos;
	pSrcVtxBuf = m_pBase + nRenderBufIndex;
	f32 fHalfDim_VS, fLeftX, fRightX, fTopY, fBottomY;
	GXBegin( GX_QUADS, (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT, nQuadCountToRender * 4 );
	{
		for ( i = 0; i < nQuadCount; pSrcVtxBuf++, i++, paPSTransPos++ )
		{
			// Handle ring buffer wrapping
			if ( i + m_nRenderStartIndex >= m_nMaxCount )
			{
				pSrcVtxBuf = m_pBase;			
			}
			 
			// If this PS is behind the camera, skip it...
			if ( paPSTransPos->z < pViewport->fNearZ ) 
			{
				continue;
			}

			// Compute half-dimension of PS in view space...
			if ( !(m_nPSFlags & FPSPRITE_FLAG_SIZE_IN_PIXELS) ) 
			{
				// Point size is in model space units...
				fHalfDim_VS = pSrcVtxBuf->fDim_MS * fScaleMultiplier;
			} 
			else 
			{
				// Point size is in screen pixels...
				fHalfDim_VS = pSrcVtxBuf->fDim_MS * fScaleMultiplier * paPSTransPos->z;
			}

			// Compute sprite edges...
			fLeftX = paPSTransPos->x - fHalfDim_VS;
			fRightX = paPSTransPos->x + fHalfDim_VS;
			fTopY = paPSTransPos->y - fHalfDim_VS;
			fBottomY = paPSTransPos->y + fHalfDim_VS;

			// Submit this quad to the GP
			u32 nColor =  (u32)(pSrcVtxBuf->ColorRGBA.fRed * 255.f) << 24
						| (u32)(pSrcVtxBuf->ColorRGBA.fGreen * 255.f) << 16 
						| (u32)(pSrcVtxBuf->ColorRGBA.fBlue * 255.f) << 8
						| (u32)(pSrcVtxBuf->ColorRGBA.fAlpha * 255.f);
						
			// Upper left corner
			GXPosition3f32( fLeftX, fTopY, paPSTransPos->z );
			GXColor1u32( nColor );
			GXTexCoord2f32( 0.f, 1.f );

			// Upper right corner
			GXPosition3f32( fRightX, fTopY, paPSTransPos->z );
			GXColor1u32( nColor );
			GXTexCoord2f32( 1.f, 1.f );

			// Lower left corner
			GXPosition3f32( fRightX, fBottomY, paPSTransPos->z );
			GXColor1u32( nColor );
			GXTexCoord2f32( 1.f, 0.f );

			// Lower right corner
			GXPosition3f32( fLeftX, fBottomY, paPSTransPos->z );
			GXColor1u32( nColor );
			GXTexCoord2f32( 0.f, 0.f );
		}
	}
	GXEnd();

	if ( FRenderer_bDrawBoundInfo ) 
	{
		// Draw group bounding sphere...
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_FacetedWireSphere( &m_BoundSphere_MS.m_Pos, m_BoundSphere_MS.m_fRadius, 2, 2 );
		fdraw_ModelSpaceAxis( 0.5f * m_BoundSphere_MS.m_fRadius );
		frenderer_Pop();
	}

	m_Xfm.PopModel();
	
	#if FPERF_ENABLE
		FPerf_nPSGroupDrawnCount++;
		FPerf_nPSpritesDrawnCount += m_nRenderCount;
	#endif

	return nCrossesPlanesMask;
}


//
//
//
void fpsprite_Renderer_Open( void ) 
{
	FASSERT( _bModuleInitialized );
	
	fgc_SetZMode( TRUE, GX_LEQUAL, TRUE );
	fgc_SetCullMode( GX_CULL_NONE );
	fgc_SetClipMode( GX_CLIP_ENABLE );
	fgc_SetChannelsForPointSprites();
	
	// Set rendering mode for single texture
	GXSetNumTexGens( 1 );
	GXSetNumTevStages( 1 );
	GXSetTevOp( GX_TEVSTAGE0, GX_MODULATE );
	GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0 );
	GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY );
	
	// Set the modelview and normal matrix to identity
	fgcxfm_LoadGCPosMatrixImm( FGC_MtxLeftToRightIdentity, GX_PNMTX0 );
	fgcxfm_SetGCCurrentMatrix( GX_PNMTX0 );

	// Set the vertex descriptions
	fgc_ClearVtxDesc();
	fgc_SetVtxPosDesc( GX_DIRECT );
	fgc_SetVtxNrmDesc( GX_NONE );
	fgc_SetVtxClrDesc( GX_VA_CLR0, GX_DIRECT );
	fgc_SetVtxClrDesc( GX_VA_CLR1, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX0, GX_DIRECT );
	fgc_SetVtxSTDesc( GX_VA_TEX1, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX2, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX3, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX4, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX5, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX6, GX_NONE );
	fgc_SetVtxSTDesc( GX_VA_TEX7, GX_NONE );
	
	// Set FDraw vertex format
	fgc_SetVtxPosFormat( (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT, GX_F32, 0 );
	fgc_SetVtxSTFormat( (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT, GX_VA_TEX0, GX_F32, 0 );
}


//
//
//
void fpsprite_Renderer_Close( void ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
u32 fpsprite_Renderer_GetStateSize( void ) 
{
	FASSERT( _bModuleInitialized );
	return 0;
}


//
//
//
void fpsprite_Renderer_GetState( void *pDestState ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
void fpsprite_Renderer_SetState( const void *pState ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
void fpsprite_Renderer_SetDefaultState( void ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent >= 0 && nEvent < FGCVID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FGCVID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;
			FPSprite_bEmulated = TRUE;
			break;

		case FGCVID_EVENT_WINDOW_DESTROYED:
			_bWindowCreated = FALSE;

			break;

		case FGCVID_EVENT_PRE_RESET:
			break;

		case FGCVID_EVENT_POST_RESET:
			break;
	}

	return TRUE;
}

