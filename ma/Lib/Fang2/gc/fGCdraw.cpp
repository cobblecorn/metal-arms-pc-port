//////////////////////////////////////////////////////////////////////////////////////
// fGCdraw.cpp - Fang drawing utility module (GameCube version).
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
#include "fcolor.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fshaders.h"
#include "fclib.h"

#include "fGCdraw.h"
#include "fGCviewport.h"
#include "fGCtex.h"
#include "fGCxfm.h"
#include "fGCvid.h"
#include "fGCvb.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	u32 nFangValue;
	u32 nGCValue;
} _MapTable_t;

typedef struct 
{
	u32 nFangValue;
	u32 nGCType;
	u32 nGCSourceValue;
	u32 nGCDestValue;
	u32 nGCOp;
} _BlendOpMapTable_t;


//////////////////////////////////////////////////////////////////////////////////////
// Local variables:
//////////////////////////////////////////////////////////////////////////////////////

static _MapTable_t _aMap_CullDir[] = 
{
	FDRAW_CULLDIR_CW, 	GX_CULL_FRONT,
	FDRAW_CULLDIR_CCW,	GX_CULL_BACK,		
	FDRAW_CULLDIR_NONE,	GX_CULL_NONE,
};

static _MapTable_t _aMap_DepthTest[] = 
{
	FDRAW_DEPTHTEST_ALWAYS,				GX_ALWAYS,
	FDRAW_DEPTHTEST_NEVER,				GX_NEVER,
	FDRAW_DEPTHTEST_CLOSER,				GX_LESS,
	FDRAW_DEPTHTEST_CLOSER_OR_EQUAL,	GX_LEQUAL,
	FDRAW_DEPTHTEST_EQUAL,				GX_EQUAL,
	FDRAW_DEPTHTEST_NOTEQUAL,			GX_NEQUAL,
	FDRAW_DEPTHTEST_FARTHER,			GX_GREATER,
	FDRAW_DEPTHTEST_FARTHER_OR_EQUAL,	GX_GEQUAL
};

static _MapTable_t _aMap_PrimType[] = {
	FDRAW_PRIMTYPE_POINTS,		GX_POINTS,
	FDRAW_PRIMTYPE_LINELIST,	GX_LINES,
	FDRAW_PRIMTYPE_LINESTRIP,	GX_LINESTRIP,
	FDRAW_PRIMTYPE_TRILIST,		GX_TRIANGLES,
	FDRAW_PRIMTYPE_TRISTRIP,	GX_TRIANGLESTRIP,
	FDRAW_PRIMTYPE_QUADLIST,	GX_QUADS,
};

static _MapTable_t _aMap_AlphaTest[] = {
	FDRAW_ALPHATEST_ALWAYS,		GX_ALWAYS,
	FDRAW_ALPHATEST_NEVER,		GX_NEVER,
	FDRAW_ALPHATEST_LESS,		GX_LESS,
	FDRAW_ALPHATEST_LEQUAL,		GX_LEQUAL,
	FDRAW_ALPHATEST_EQUAL,		GX_EQUAL,
	FDRAW_ALPHATEST_NOTEQUAL,	GX_NEQUAL,
	FDRAW_ALPHATEST_GEQUAL,		GX_GEQUAL,
	FDRAW_ALPHATEST_GREATER,	GX_GREATER,
};

static _BlendOpMapTable_t _aMap_BlendOp[] = {
	FDRAW_BLENDOP_SRC,						GX_BM_NONE,		GX_BL_ONE,			GX_BL_ZERO,			GX_LO_NOOP,
	FDRAW_BLENDOP_DST,						GX_BM_BLEND,	GX_BL_ZERO,			GX_BL_ONE,			GX_LO_NOOP,
	FDRAW_BLENDOP_ZERO,						GX_BM_LOGIC,	GX_BL_ZERO,			GX_BL_ZERO,			GX_LO_CLEAR,
	FDRAW_BLENDOP_SRC_PLUS_DST,				GX_BM_BLEND,	GX_BL_ONE,			GX_BL_ONE,			GX_LO_NOOP,
	FDRAW_BLENDOP_ALPHA_TIMES_SRC,			GX_BM_BLEND,	GX_BL_SRCALPHA,		GX_BL_ZERO,			GX_LO_NOOP,
	FDRAW_BLENDOP_ALPHA_TIMES_DST,			GX_BM_BLEND,	GX_BL_ZERO,			GX_BL_SRCALPHA,		GX_LO_NOOP,
	FDRAW_BLENDOP_INVALPHA_TIMES_SRC,		GX_BM_BLEND,	GX_BL_INVSRCALPHA,	GX_BL_ZERO,			GX_LO_NOOP,
	FDRAW_BLENDOP_INVALPHA_TIMES_DST,		GX_BM_BLEND,	GX_BL_ZERO,			GX_BL_INVSRCALPHA,	GX_LO_NOOP,
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,	GX_BM_BLEND,	GX_BL_SRCALPHA,		GX_BL_ONE,			GX_LO_NOOP,
	FDRAW_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC,	GX_BM_BLEND,	GX_BL_ONE,			GX_BL_SRCALPHA,		GX_LO_NOOP,
	FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE,	GX_BM_BLEND,	GX_BL_SRCALPHA,		GX_BL_INVSRCALPHA,	GX_LO_NOOP,
	FDRAW_BLENDOP_LERP_WITH_ALPHA_XPARENT,	GX_BM_BLEND,	GX_BL_INVSRCALPHA,	GX_BL_SRCALPHA,		GX_LO_NOOP
};

//test
CFTexInst *_pTestRT;
//


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static u32 _nMaxGCDrawPrimElements;	// Maximum number of primitives we can draw with a single call to DrawPrimitive

static FGCVB_t *_pVB;
static u32 _nVBCount;
static u32 _nVertexCount;
static u32 _nNextFreeVtxIndex;
static u32 _nCurrentVB;

static BOOL _bDisableColorViaDepth;
static BOOL _bDisableColorViaAlpha;
static BOOL _bAdditiveOverride=FALSE;

static u32 _nD3DTexCoordIndex;

static u32 _nFogChangedKey;


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );
static void _ClearPublicCaps( void );
static BOOL _ComputePublicCapsFromGCCaps( void );

static BOOL _CheckCoronaVisible(const CFVec3A& vStart, CFVec3& vEnd);

//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fgcdraw_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_ClearPublicCaps();

	_nFogChangedKey = 0;
	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;
	
	return TRUE;
}


//
//
//
void fgcdraw_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
void fdraw_PrimList( FDrawPrimType_e nPrimType, const FDrawVtx_t *pVtxArray, u32 nVtxCount ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	
	fsh_ExecuteCurrent();
	if (_bAdditiveOverride)
	{
		fgcdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC_PLUS_DST);
	}
	
	fgcxfm_SetGCMatrices();
	
	// Crank up the renderer	
	GXBegin( (GXPrimitive)_aMap_PrimType[ nPrimType ].nGCValue, (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT, nVtxCount );
	{
		for ( int i = 0; i < nVtxCount; i++ )
		{
			// Submit this vertex to the GP
			GXPosition3f32( pVtxArray[i].Pos_MS.x, pVtxArray[i].Pos_MS.y, pVtxArray[i].Pos_MS.z );
			GXColor4u8( (u8)(pVtxArray[i].ColorRGBA.fRed * 255.f),	(u8)(pVtxArray[i].ColorRGBA.fGreen * 255.f), 
						(u8)(pVtxArray[i].ColorRGBA.fBlue * 255.f),	(u8)(pVtxArray[i].ColorRGBA.fAlpha * 255.f) );
			GXTexCoord2f32( pVtxArray[i].ST.x, pVtxArray[i].ST.y );
		}
	}
	GXEnd();
}

void fdraw_SetLineWidth(f32 fWidth)
{
	GXSetLineWidth( (u8)(fWidth*6.0f), GX_TO_ONE );
	
	GXEnableTexOffsets(GX_TEXCOORD0, GX_TRUE, GX_TRUE);
}

//
//
//
void fgcdraw_SetCullDir( FDrawCullDir_e nCullDir ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nCullDir >= 0 && nCullDir < FDRAW_CULLDIR_COUNT );
	
	fgc_SetCullMode( (GXCullMode)_aMap_CullDir[nCullDir].nGCValue );
}


//
//
//
void fgcdraw_EnableClipping( BOOL bEnable ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	
	fgc_SetClipMode( (GXClipMode)bEnable );
}


//
//
//
void fgcdraw_Color_SetFunc( FDrawColorFunc_e nColorFunc ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nColorFunc >= 0 && nColorFunc < FDRAW_COLORFUNC_COUNT );
	
	fsh_SetShaderType( SHADERTYPE_SURFACE );
	fsh_SetShader( FDraw_ColorFuncToShaderMap[nColorFunc].nShader, FSH_INVALID_LIGHT_SHADER, FSH_INVALID_SPECULAR_SHADER );
}


//
//
//
void fgcdraw_SetTexture( CFTexInst *pTexInst ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	fsh_UseDynamicRegisters();
	fsh_SetSurfaceRegister( 0, (u32)pTexInst );
	fsh_SetSurfaceRegister( 1, 0 );
}


//
//
//
void fgcdraw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nTexCoordXfmMode >= 0 && nTexCoordXfmMode < FDRAW_TEXCOORDXFMMODE_COUNT );

	switch( nTexCoordXfmMode ) 
	{
		case FDRAW_TEXCOORDXFMMODE_MTXONLY:
//			_nD3DTexCoordIndex = 0;
//			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
			break;

		case FDRAW_TEXCOORDXFMMODE_CAMSPACE_NORMAL:
//			_nD3DTexCoordIndex = D3DTSS_TCI_CAMERASPACENORMAL;
//			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
			break;

		case FDRAW_TEXCOORDXFMMODE_CAMSPACE_POS:
//			_nD3DTexCoordIndex = D3DTSS_TCI_CAMERASPACEPOSITION;
//			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
			break;

		case FDRAW_TEXCOORDXFMMODE_CAMSPACE_REFLECTION:
//			_nD3DTexCoordIndex = D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
//			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
			break;

		default:
//			_nD3DTexCoordIndex = 0;
//			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_DISABLE );
			break;
	}
}


//
//
//
void fgcdraw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43 ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	
//	FASSERT_NOW;

//	fdx8xfm_SetCustomDXMatrix( D3DTS_TEXTURE0, pMtx43 );
}


//
//
//
void fgcdraw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if ( bMaskOffRed && bMaskOffGreen && bMaskOffBlue )
	{
		fgc_SetColorBufferUpdate( FALSE );
	}

	return;
}


//
//
//
void fgcdraw_Color_EnableDither( BOOL bEnable ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	
//	fgc_SetDitherMode( bEnable );
}


//
//
//
void fgcdraw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nAlphaTest>=0 && nAlphaTest<FDRAW_ALPHATEST_COUNT );

	fgc_SetAlphaCompare((GXCompare)_aMap_AlphaTest[nAlphaTest].nGCValue, 
						(u8)(fReference * 255.0f), 
						GX_AOP_AND, 
						GX_ALWAYS, 
						0 );
}


//
//
//
void fgcdraw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nBlendOp >= 0 && nBlendOp < FDRAW_BLENDOP_COUNT );

	// Pixel Processing
	fgc_SetBlendOp( (GXBlendMode)_aMap_BlendOp[nBlendOp].nGCType, 
					(GXBlendFactor)_aMap_BlendOp[nBlendOp].nGCSourceValue,
					(GXBlendFactor)_aMap_BlendOp[nBlendOp].nGCDestValue,
					(GXLogicOp)_aMap_BlendOp[nBlendOp].nGCOp );
}


//
//
//
void fgcdraw_Depth_EnableWriting( BOOL bEnable ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( bEnable || FDraw_bCanDisableDepthWrites );
	
	fgc_SetZWriteMode( bEnable );
}


//
//
//
void fgcdraw_Depth_SetTest( FDrawDepthTest_e nDepthTest ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nDepthTest >= 0 && nDepthTest < FDRAW_DEPTHTEST_COUNT );
	
	fgc_SetZCompareMode( (GXCompare)_aMap_DepthTest[nDepthTest].nGCValue );
}


//
//
//
void fgcdraw_Depth_SetBiasLevel( u32 nNewLevel ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nNewLevel <= 16 );
	
	fsh_SetDepthBias(nNewLevel);
}


//
//
//
void fgcdraw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
							   FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
							   FDrawStencilOp_e nDepthPassOp ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nStencilTest >= 0 && nStencilTest < FDRAW_STENCILTEST_COUNT );
	FASSERT( nStencilFailOp >= 0 && nStencilFailOp < FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthFailOp >= 0 && nDepthFailOp < FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthPassOp >= 0 && nDepthPassOp < FDRAW_STENCILOP_COUNT );

	FASSERT_NOW;
}


//
//
//
BOOL fgcdraw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue ) 
{
	FASSERT( _bWindowCreated );

	if ( !bMaskOffRed && !bMaskOffGreen && !bMaskOffBlue )
	{
		// We obviously can support no color mask
		return TRUE;
	}
	else if ( bMaskOffRed && bMaskOffGreen && bMaskOffBlue )
	{
		// We can support a full color mask
		return TRUE;
	}

	return FALSE;
}


//
//
//
BOOL fgcdraw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp,
									  FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nStencilTest>=0 && nStencilTest<FDRAW_STENCILTEST_COUNT );
	FASSERT( nStencilFailOp>=0 && nStencilFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthFailOp>=0 && nDepthFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthPassOp>=0 && nDepthPassOp<FDRAW_STENCILOP_COUNT );
	
	if ( nStencilTest == FDRAW_STENCILTEST_ALWAYS )
		return TRUE;

	return FALSE;
}


//
//
//
void fgcdraw_Renderer_Open( void ) 
{
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_SetCullDir( FDRAW_CULLDIR_CCW );
	fgc_SetClipMode( TRUE );
	fgc_SetChannelsForFDraw();
	
	// Set the vertex descriptions
	fgc_ClearVtxDesc();
	fgc_SetVtxPosDesc( GX_DIRECT );
	fgc_SetVtxClrDesc( GX_VA_CLR0, GX_DIRECT );
	fgc_SetVtxSTDesc( GX_VA_TEX0, GX_DIRECT );
	
	// Set the modelview matrix to identity.  No normal
	// matrix is needed since we have no normals for fdraw
	fgcxfm_LoadGCPosMatrixImm( FGC_MtxLeftToRightIdentity, GX_PNMTX0 );
	GXLoadNrmMtxImm( FGC_MtxIdentity, GX_PNMTX0 );
	fgcxfm_SetGCCurrentMatrix( GX_PNMTX0 );
	
	//Reset Indirect Unit
	GXSetNumIndStages( 0 );
	GXSetTevDirect( GX_TEVSTAGE0 );
	GXSetTevDirect( GX_TEVSTAGE1 );
	GXSetTevDirect( GX_TEVSTAGE2 );
	GXSetTevDirect( GX_TEVSTAGE3 );
	
	fsh_Open();
	
	fsh_UseDynamicRegisters();
	fsh_SetSurfaceRegister( 0, NULL );
	fsh_SetSurfaceRegister( 1, 0 );
	
	// Set FDraw vertex format
	fgc_SetVtxPosFormat( (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT, GX_F32, 0 );
	fgc_SetVtxSTFormat( (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT, GX_VA_TEX0, GX_F32, 0 );
}


//
//
//
void fgcdraw_Renderer_Close( void ) 
{
	fsh_Close();
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
			
			if( !_ComputePublicCapsFromGCCaps() ) {
				_ClearPublicCaps();
				_bWindowCreated = FALSE;
				return FALSE;
			}
			
			break;

		case FGCVID_EVENT_WINDOW_DESTROYED:
			_ClearPublicCaps();
			_bWindowCreated = FALSE;

			break;

		case FGCVID_EVENT_PRE_RESET:
			break;

		case FGCVID_EVENT_POST_RESET:
			_nCurrentVB = 0;
			_nNextFreeVtxIndex = 0;
			break;
	}

	return TRUE;
}


//
//
//
static void _ClearPublicCaps( void ) 
{
	FDraw_nCapColorFunc = 0;
	FDraw_nCapAlphaTest = 0;
	FDraw_nCapBlendOp = 0;
	FDraw_nCapDepthTest = 0;

	FDraw_nDepthBitDepth = 0;
	FDraw_nStencilBitDepth = 0;
	FDraw_nStencilBitMask = 0;

	FDraw_bCanDisableDepthWrites = FALSE;
	FDraw_nMaxDepthBiasValue = 0;
}


//
//
//
static BOOL _ComputePublicCapsFromGCCaps( void ) 
{
	// Alpha test...
	FDraw_nCapAlphaTest = FDRAW_CAP_ALPHATEST_ALWAYS 
						| FDRAW_CAP_ALPHATEST_NEVER
						| FDRAW_CAP_ALPHATEST_LESS
						| FDRAW_CAP_ALPHATEST_LEQUAL
						| FDRAW_CAP_ALPHATEST_EQUAL
						| FDRAW_CAP_ALPHATEST_NOTEQUAL
						| FDRAW_CAP_ALPHATEST_GEQUAL
						| FDRAW_CAP_ALPHATEST_GREATER;

	// Alpha blending...
	FDraw_nCapBlendOp =   FDRAW_CAP_BLENDOP_SRC
						| FDRAW_CAP_BLENDOP_DST
						| FDRAW_CAP_BLENDOP_ZERO
						| FDRAW_CAP_BLENDOP_SRC_PLUS_DST
						| FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC
						| FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST
						| FDRAW_CAP_BLENDOP_INVALPHA_TIMES_SRC
						| FDRAW_CAP_BLENDOP_INVALPHA_TIMES_DST
						| FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST
						| FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC
						| FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_OPAQUE
						| FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_XPARENT;

	// Depth buffer test...
	FDraw_nCapDepthTest = FDRAW_CAP_DEPTHTEST_ALWAYS
						| FDRAW_CAP_DEPTHTEST_NEVER
						| FDRAW_CAP_DEPTHTEST_CLOSER
						| FDRAW_CAP_DEPTHTEST_CLOSER_OR_EQUAL
						| FDRAW_CAP_DEPTHTEST_EQUAL
						| FDRAW_CAP_DEPTHTEST_NOTEQUAL
						| FDRAW_CAP_DEPTHTEST_FARTHER
						| FDRAW_CAP_DEPTHTEST_FARTHER_OR_EQUAL;

	// Set stencil info
	FDraw_nDepthBitDepth = FVid_Mode.nDepthBits;
	FDraw_nStencilBitDepth = FVid_Mode.nStencilBits;
	FDraw_nStencilBitMask = (1 << FVid_Mode.nStencilBits) - 1;

	// Currently no support for depth bias						
	FDraw_nMaxDepthBiasValue = 16;

	// GC can disable depth
	FDraw_bCanDisableDepthWrites = TRUE;
	
	// Color function...
	FDraw_nCapColorFunc = 0;
	FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_ALL;
	FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_DECALTEX_AI_INTENSITY;
	
	return TRUE;
}



#include "fworld_coll.h"
#include "flight.h"
#include "floop.h"
CFVec3A _vRayStart, _vRayEnd;
static 	CFCollInfo _CoronaCollInfo;

//
//
//
static BOOL _CoronaTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection )
{
	((CFWorldMesh *)pTracker)->CollideWithMeshTris( &_CoronaCollInfo );
	if ( FColl_nImpactCount )
	{
		return FALSE;
	}
	return TRUE;
}

BOOL _CheckCoronaVisible(const CFVec3A& vStart, CFVec3& vEnd)
{
	_CoronaCollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	_CoronaCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	_CoronaCollInfo.bCullBacksideCollisions = TRUE;
	_CoronaCollInfo.nCollMask = FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT;
	_CoronaCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	_CoronaCollInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	_CoronaCollInfo.Ray.Init( &vStart.v3, &vEnd );
	
	CFTrackerCollideRayInfo CollRayInfo;
	CollRayInfo.StartPoint_WS = _CoronaCollInfo.Ray.vStart_WS;
	CollRayInfo.EndPoint_WS = _CoronaCollInfo.Ray.vEnd_WS;
	CollRayInfo.bComputeIntersection = FALSE;
	CollRayInfo.bIgnoreCollisionFlag = FALSE;
	CollRayInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	CollRayInfo.pCallback = _CoronaTrackersCallback;
	CollRayInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	CollRayInfo.nTrackerSkipCount = 0;
	CollRayInfo.ppTrackerSkipList = NULL;

	fcoll_Clear();
	
	// Check collision against volume geometry
	fworld_CollideWithWorldTris( &_CoronaCollInfo );
	if ( FColl_nImpactCount )
	{
		return FALSE;
	}

	// Check collision against instanced geometry
	fworld_CollideWithTrackers( &CollRayInfo );

	if ( FColl_nImpactCount )
	{
		return FALSE;
	}

	return TRUE;
}
	

#define _CORONA_VIS_FLAG 0x40

void fdraw_RenderScrQuad(const CFMtx43A *pCamera, CFVec3 *pLoc, CFTexInst *pTex, float fScale)
{
	float fDot=1.0f;
	CFVec3 quad[4], XAxis, YAxis, ZAxis, Pos, vec;
	CFColorRGBA color;
	
	ZAxis.x = pCamera->aa[0][2]; ZAxis.y = pCamera->aa[1][2]; ZAxis.z = pCamera->aa[2][2];

	if (pTex)
	{
		fgcdraw_SetTexture(pTex);
	}
	
	fgcdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
	fgcdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC);

	color.fRed = 1.0f;
	color.fGreen = 1.0f;
	color.fBlue = 1.0f;
	color.fAlpha = 1.0f;

	XAxis.x = pCamera->aa[0][0]; XAxis.y = pCamera->aa[1][0]; XAxis.z = pCamera->aa[2][0];
	YAxis.x = pCamera->aa[0][1]; YAxis.y = pCamera->aa[1][1]; YAxis.z = pCamera->aa[2][1];
	
	vec.x = -1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[0].x = vec.Dot(XAxis)*fScale + pLoc->x;
	quad[0].y = vec.Dot(YAxis)*fScale + pLoc->y;
	quad[0].z = vec.Dot(ZAxis)*fScale + pLoc->z;

	vec.x = +1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[1].x = vec.Dot(XAxis)*fScale + pLoc->x; 
	quad[1].y = vec.Dot(YAxis)*fScale + pLoc->y; 
	quad[1].z = vec.Dot(ZAxis)*fScale + pLoc->z; 

	vec.x = +1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[2].x = vec.Dot(XAxis)*fScale + pLoc->x; 
	quad[2].y = vec.Dot(YAxis)*fScale + pLoc->y; 
	quad[2].z = vec.Dot(ZAxis)*fScale + pLoc->z; 

	vec.x = -1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[3].x = vec.Dot(XAxis)*fScale + pLoc->x; 
	quad[3].y = vec.Dot(YAxis)*fScale + pLoc->y; 
	quad[3].z = vec.Dot(ZAxis)*fScale + pLoc->z; 

	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_EnableWriting( FALSE );
	
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);
}


void fdraw_RenderCorona(const CFMtx43A *pCamera, CFLight *pLight, float fCamDistSq, u32 nFlags, u32 nPhase)
{
	float fScaleDelta;
	float fDot=1.0f;
	CFVec3 quad[4], XAxis, YAxis, ZAxis, Pos, vec;
	CFColorRGBA color;
	float fScale;

	ZAxis.x = pCamera->aa[0][2]; ZAxis.y = pCamera->aa[1][2]; ZAxis.z = pCamera->aa[2][2];

	if ( nPhase == (pLight->m_nRayCastPhase&(_CORONA_VIS_FLAG-1)) )
	{
		if (_CheckCoronaVisible(pCamera->m_vPos, pLight->m_spInfluence_WS.m_Pos) ) 
		{
			if (pLight->m_nFlags & FLIGHT_FLAG_HASDIR)
			{
				fDot = pLight->m_mtxOrientation_WS.m_vFront.x*ZAxis.x + pLight->m_mtxOrientation_WS.m_vFront.y*ZAxis.y + pLight->m_mtxOrientation_WS.m_vFront.z*ZAxis.z;
			}
			if (fDot > 0.0f)
			{
				pLight->m_nRayCastPhase |= _CORONA_VIS_FLAG;
				pLight->m_fFadeDelta = +3.0f;
			}
			else
			{
				pLight->m_nRayCastPhase &= ~_CORONA_VIS_FLAG;
				pLight->m_fFadeDelta = -3.0f;
			}
		}
		else
		{
			pLight->m_nRayCastPhase &= ~_CORONA_VIS_FLAG;
			pLight->m_fFadeDelta = -3.0f;
		}
	}

	pLight->m_fFade += pLight->m_fFadeDelta*FLoop_fPreviousLoopSecs;
	if (pLight->m_fFade < 0.0f) pLight->m_fFade = 0.0f;
	if (pLight->m_fFade > 1.0f) pLight->m_fFade = 1.0f;

	if ( !(pLight->m_nRayCastPhase&_CORONA_VIS_FLAG) && pLight->m_fFade == 0.0f)
	{
		return;
	}

	fScaleDelta = pLight->m_fFade*2.0f;
	if (fScaleDelta > 1.0f) fScaleDelta = 1.0f;

	fScaleDelta *= pLight->m_fCoronaScale;
	
	BOOL bUseDefault=FALSE;

	if (pLight->m_pCoronaTex)
	{
		fgcdraw_SetTexture(pLight->m_pCoronaTex);
	}
	else
	{
		fgcdraw_SetTexture(pLight->m_pCoronaTex);
		bUseDefault = TRUE;
	}
	fgcdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
			
	if (bUseDefault)
	{	
		//GXLoadTexObj( (GXTexObj*)fsh_GetAttenMap(), GX_TEXMAP0 );
		fgcdraw_SetTexture((CFTexInst*)fsh_GetAttenMap());
	}

	pLight->ComputeColor();
	color.fRed = pLight->m_ScaledColor.fRed*pLight->m_fFade*0.5f;
	color.fGreen = pLight->m_ScaledColor.fGreen*pLight->m_fFade*0.5f;
	color.fBlue = pLight->m_ScaledColor.fBlue*pLight->m_fFade*0.5f;
	color.fAlpha = pLight->m_ScaledColor.fAlpha;
	
	f32 fDistScale;
	if (pLight->m_nFlags&FLIGHT_FLAG_CORONA_WORLDSPACE)
	{
		fDistScale = 0.0005f;
	}
	else
	{
		fDistScale = 0.00015f;
	}
	f32 fInvDist = (fCamDistSq * fDistScale);

	if (fInvDist > 1.0f) fInvDist = 1.0f;
	color.fRed *= fInvDist;
	color.fGreen *= fInvDist;
	color.fBlue *= fInvDist;
	
	XAxis.x = pCamera->aa[0][0]; XAxis.y = pCamera->aa[1][0]; XAxis.z = pCamera->aa[2][0];
	YAxis.x = pCamera->aa[0][1]; YAxis.y = pCamera->aa[1][1]; YAxis.z = pCamera->aa[2][1];
	
	if ( !(pLight->m_nFlags&FLIGHT_FLAG_CORONA_WORLDSPACE) || pLight->m_fDeltaScale == 1.0f )
	{
		fScale = fCamDistSq * 0.00075f * fScaleDelta;
		if (fScale < 1.0f) fScale = 1.0f;

		if (fScale > 15.0f)
		{
			fDistScale = (fScale - 15.0f)*0.20f;
			if (fDistScale > 1.0f) { fDistScale = 1.0f; }
			fDistScale = 1.0f - fDistScale;

			color.fRed *= fDistScale;
			color.fGreen *= fDistScale;
			color.fBlue *= fDistScale;
		}
	}
	else
	{
		if (pLight->m_fDeltaScale != 0.0f)
		{
			fScale = (1.0f - pLight->m_fDeltaScale)*fScaleDelta + (pLight->m_fDeltaScale)*(fCamDistSq * 0.00075f * fScaleDelta);
		}
		else
		{
			fScale = fScaleDelta;
		}
	}
	
	vec.x = -1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[0].x = vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x;
	quad[0].y = vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y;
	quad[0].z = vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z;

	vec.x = +1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[1].x = vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x; 
	quad[1].y = vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y; 
	quad[1].z = vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z; 

	vec.x = +1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[2].x = vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x; 
	quad[2].y = vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y; 
	quad[2].z = vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z; 

	vec.x = -1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[3].x = vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x; 
	quad[3].y = vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y; 
	quad[3].z = vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z; 

	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_EnableWriting( FALSE );
	
	_bAdditiveOverride = TRUE;
	
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);
	
	_bAdditiveOverride = FALSE;
}
