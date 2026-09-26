//////////////////////////////////////////////////////////////////////////////////////
// fdx8draw.cpp - Fang drawing utility module (DX version).
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
// 10/10/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcolor.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fclib.h"

#include "fdx8draw.h"
#include "fdx8viewport.h"
#include "fdx8tex.h"
#include "fdx8xfm.h"
#include "fdx8vid.h"
#include "fdx8vb.h"

#include "fdx8sh.h"

#define _DXFVF_ELEMENTS			(D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0))
#define _MIN_VERTEX_COUNT		32
#define _MAX_VERTEX_COUNT		65535

#if FANG_PLATFORM_XB
#include "CompiledPShaderfxbIntensity.h"
#else
#include "CompiledPShaderfdx8Intensity.h"
#endif

static u32 _nPShader_Intensity=0;


typedef enum {
	_COLORMASKTYPE_RENDERSTATE,
	_COLORMASKTYPE_DEPTH,
	_COLORMASKTYPE_ALPHA,

	_COLORMASKTYPE_NONE
} _ColorMaskType_e;

#if FANG_PLATFORM_WIN
	typedef struct {
		u32 nStart;		// Number of vertices describing the first in the series
		u32 nRepeat;	// Number of vertices describing each in the series, after the first
		u32 nBack;		// Number of vertices we need to back up if we split the series
	} _PrimVtxCountInfo_t;
#endif


typedef struct {
	u32 nD3DValue;
	u32 nFangValue;
} _MapTable_t;


typedef struct {
	_MapTable_t *pMapTable;
	u32 nElementCount;
} _MapTableList_t;


static _MapTable_t _aMap_StencilOp_D3DSTENCILCAPS[] = {
	D3DSTENCILCAPS_KEEP,			FDRAW_STENCILOP_KEEP,
	D3DSTENCILCAPS_ZERO,			FDRAW_STENCILOP_ZERO,
	D3DSTENCILCAPS_REPLACE,			FDRAW_STENCILOP_REPLACE,
	D3DSTENCILCAPS_INCRSAT,			FDRAW_STENCILOP_INC_CLAMP,
	D3DSTENCILCAPS_DECRSAT,			FDRAW_STENCILOP_DEC_CLAMP,
	D3DSTENCILCAPS_INVERT,			FDRAW_STENCILOP_INVERT,
};

static _MapTable_t _aMap_CullDir_D3DCULL[] = {
	D3DCULL_CW,						FDRAW_CULLDIR_CW,
	D3DCULL_CCW,					FDRAW_CULLDIR_CCW,
	D3DCULL_NONE,					FDRAW_CULLDIR_NONE,
};

static _MapTable_t _aMap_WrapEnable_D3DTEXTUREADDRESS[] = {
	D3DTADDRESS_CLAMP,				FALSE,
	D3DTADDRESS_WRAP,				TRUE,
};

static _MapTable_t _aMap_TexInterp_D3DTEXTUREFILTERTYPE[] = {
	D3DTEXF_POINT,					FALSE,
	D3DTEXF_LINEAR,					TRUE,
};

static _MapTable_t _aMap_AlphaTest_D3DCMPFUNC[] = {
	D3DCMP_ALWAYS,					FDRAW_ALPHATEST_ALWAYS,
	D3DCMP_NEVER,					FDRAW_ALPHATEST_NEVER,
	D3DCMP_LESS,					FDRAW_ALPHATEST_LESS,
	D3DCMP_LESSEQUAL,				FDRAW_ALPHATEST_LEQUAL,
	D3DCMP_EQUAL,					FDRAW_ALPHATEST_EQUAL,
	D3DCMP_NOTEQUAL,				FDRAW_ALPHATEST_NOTEQUAL,
	D3DCMP_GREATEREQUAL,			FDRAW_ALPHATEST_GEQUAL,
	D3DCMP_GREATER,					FDRAW_ALPHATEST_GREATER,
};

static _MapTable_t _aMap_ColorFunc_COLOROP[] = {
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECAL_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECALTEX_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECALTEX_AT,
	D3DTOP_MODULATE,				FDRAW_COLORFUNC_DIFFUSETEX_AI,
	D3DTOP_MODULATE,				FDRAW_COLORFUNC_DIFFUSETEX_AT,
	D3DTOP_MODULATE,				FDRAW_COLORFUNC_DIFFUSETEX_AIAT,
	D3DTOP_MODULATECOLOR_ADDALPHA,	FDRAW_COLORFUNC_SPECULARTEX_AT,
	D3DTOP_ADD,						FDRAW_COLORFUNC_ADD,
	D3DTOP_BLENDDIFFUSEALPHA,		FDRAW_COLORFUNC_BLEND_AIPLUSAT,
	D3DTOP_MODULATEALPHA_ADDCOLOR,	FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,
};

static _MapTable_t _aMap_ColorFunc_COLORARG1[] = {
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECAL_AI,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DECALTEX_AI,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DECALTEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AIAT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_SPECULARTEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_ADD,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_BLEND_AIPLUSAT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,
};

static _MapTable_t _aMap_ColorFunc_COLORARG2[] = {
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECAL_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DIFFUSETEX_AI,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DIFFUSETEX_AT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DIFFUSETEX_AIAT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_SPECULARTEX_AT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_ADD,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_BLEND_AIPLUSAT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,
};

static _MapTable_t _aMap_ColorFunc_ALPHAOP[] = {
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECAL_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECALTEX_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECALTEX_AT,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DIFFUSETEX_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DIFFUSETEX_AT,
	D3DTOP_MODULATE,				FDRAW_COLORFUNC_DIFFUSETEX_AIAT,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_SPECULARTEX_AT,
	D3DTOP_ADD,						FDRAW_COLORFUNC_ADD,
	D3DTOP_ADD,						FDRAW_COLORFUNC_BLEND_AIPLUSAT,
	D3DTOP_SELECTARG2,				FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,
	D3DTOP_SELECTARG1,				FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,
};

static _MapTable_t _aMap_ColorFunc_ALPHAARG1[] = {
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECAL_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AI,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DECALTEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AI,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DIFFUSETEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AIAT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_SPECULARTEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_ADD,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_BLEND_AIPLUSAT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,
	D3DTA_TEXTURE,				FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,
};

static _MapTable_t _aMap_ColorFunc_ALPHAARG2[] = {
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECAL_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DIFFUSETEX_AT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_DIFFUSETEX_AIAT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_SPECULARTEX_AT,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_ADD,
	D3DTA_TEXTURE,					FDRAW_COLORFUNC_BLEND_AIPLUSAT,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,
	D3DTA_DIFFUSE,					FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,
};

static _MapTable_t _aMap_BlendOp_D3DBLENDSRC[] = {
	D3DBLEND_ONE,					FDRAW_BLENDOP_SRC,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_DST,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_ZERO,
	D3DBLEND_ONE,					FDRAW_BLENDOP_SRC_PLUS_DST,
	D3DBLEND_SRCALPHA,				FDRAW_BLENDOP_ALPHA_TIMES_SRC,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_ALPHA_TIMES_DST,
	D3DBLEND_INVSRCALPHA,			FDRAW_BLENDOP_INVALPHA_TIMES_SRC,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_INVALPHA_TIMES_DST,
	D3DBLEND_SRCALPHA,				FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,
	D3DBLEND_ONE,					FDRAW_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC,
	D3DBLEND_SRCALPHA,				FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE,
	D3DBLEND_INVSRCALPHA,			FDRAW_BLENDOP_LERP_WITH_ALPHA_XPARENT,
};

static _MapTable_t _aMap_BlendOp_D3DBLENDDST[] = {
	D3DBLEND_ZERO,					FDRAW_BLENDOP_SRC,
	D3DBLEND_ONE,					FDRAW_BLENDOP_DST,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_ZERO,
	D3DBLEND_ONE,					FDRAW_BLENDOP_SRC_PLUS_DST,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_ALPHA_TIMES_SRC,
	D3DBLEND_SRCALPHA,				FDRAW_BLENDOP_ALPHA_TIMES_DST,
	D3DBLEND_ZERO,					FDRAW_BLENDOP_INVALPHA_TIMES_SRC,
	D3DBLEND_INVSRCALPHA,			FDRAW_BLENDOP_INVALPHA_TIMES_DST,
	D3DBLEND_ONE,					FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,
	D3DBLEND_SRCALPHA,				FDRAW_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC,
	D3DBLEND_INVSRCALPHA,			FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE,
	D3DBLEND_SRCALPHA,				FDRAW_BLENDOP_LERP_WITH_ALPHA_XPARENT,
};

static _MapTable_t _aMap_DepthTest_D3DCMPFUNC[] = {
	D3DCMP_ALWAYS,					FDRAW_DEPTHTEST_ALWAYS,
	D3DCMP_NEVER,					FDRAW_DEPTHTEST_NEVER,
	D3DCMP_LESS,					FDRAW_DEPTHTEST_CLOSER,
	D3DCMP_LESSEQUAL,				FDRAW_DEPTHTEST_CLOSER_OR_EQUAL,
	D3DCMP_EQUAL,					FDRAW_DEPTHTEST_EQUAL,
	D3DCMP_NOTEQUAL,				FDRAW_DEPTHTEST_NOTEQUAL,
	D3DCMP_GREATER,					FDRAW_DEPTHTEST_FARTHER,
	D3DCMP_GREATEREQUAL,			FDRAW_DEPTHTEST_FARTHER_OR_EQUAL,
};

static _MapTable_t _aMap_StencilTest_D3DSTENCILFUNC[] = {
	D3DCMP_ALWAYS,					FDRAW_STENCILTEST_ALWAYS,
	D3DCMP_NEVER,					FDRAW_STENCILTEST_NEVER,
	D3DCMP_LESS,					FDRAW_STENCILTEST_LESS,
	D3DCMP_LESSEQUAL,				FDRAW_STENCILTEST_LEQUAL,
	D3DCMP_EQUAL,					FDRAW_STENCILTEST_EQUAL,
	D3DCMP_NOTEQUAL,				FDRAW_STENCILTEST_NOTEQUAL,
	D3DCMP_GREATEREQUAL,			FDRAW_STENCILTEST_GEQUAL,
	D3DCMP_GREATER,					FDRAW_STENCILTEST_GREATER,
};

static _MapTable_t _aMap_StencilTest_D3DSTENCILOP[] = {
	D3DSTENCILOP_KEEP,				FDRAW_STENCILOP_KEEP,
	D3DSTENCILOP_ZERO,				FDRAW_STENCILOP_ZERO,
	D3DSTENCILOP_REPLACE,			FDRAW_STENCILOP_REPLACE,
	D3DSTENCILOP_INCRSAT,			FDRAW_STENCILOP_INC_CLAMP,
	D3DSTENCILOP_DECRSAT,			FDRAW_STENCILOP_DEC_CLAMP,
	D3DSTENCILOP_INVERT,			FDRAW_STENCILOP_INVERT,
};

static const _MapTableList_t _MapTableList[] = {
	_aMap_StencilOp_D3DSTENCILCAPS,			FDRAW_STENCILOP_COUNT,
	_aMap_CullDir_D3DCULL,					FDRAW_CULLDIR_COUNT,
	_aMap_WrapEnable_D3DTEXTUREADDRESS,		2,
	_aMap_TexInterp_D3DTEXTUREFILTERTYPE,	2,
	_aMap_AlphaTest_D3DCMPFUNC,				FDRAW_ALPHATEST_COUNT,
	_aMap_ColorFunc_COLOROP,				FDRAW_COLORFUNC_COUNT,
	_aMap_ColorFunc_COLORARG1,				FDRAW_COLORFUNC_COUNT,
	_aMap_ColorFunc_COLORARG2,				FDRAW_COLORFUNC_COUNT,
	_aMap_ColorFunc_ALPHAOP,				FDRAW_COLORFUNC_COUNT,
	_aMap_ColorFunc_ALPHAARG1,				FDRAW_COLORFUNC_COUNT,
	_aMap_ColorFunc_ALPHAARG2,				FDRAW_COLORFUNC_COUNT,
	_aMap_BlendOp_D3DBLENDSRC,				FDRAW_BLENDOP_COUNT,
	_aMap_BlendOp_D3DBLENDDST,				FDRAW_BLENDOP_COUNT,
	_aMap_DepthTest_D3DCMPFUNC,				FDRAW_DEPTHTEST_COUNT,
	_aMap_StencilTest_D3DSTENCILFUNC,		FDRAW_STENCILTEST_COUNT,
	_aMap_StencilTest_D3DSTENCILOP,			FDRAW_STENCILOP_COUNT,

	NULL,									0
};


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static u32 _nMaxD3DDrawPrimElements;	// Maximum number of primitives we can draw with a single call to DrawPrimitive
static DWORD _hD3DStateBlock;

static _ColorMaskType_e _nColorMaskType;
static BOOL _bDisableColorViaDepth;
static BOOL _bDisableColorViaAlpha;

static u32 _nD3DTexCoordIndex;

static u32 _nFogChangedKey;

#if FANG_PLATFORM_WIN
	static FDX8VB_t *_pVB;
	static u32 _nVBCount;
	static u32 _nVertexCount;
	static u32 _nNextFreeVtxIndex;
	static u32 _nCurrentVB;

	static const _PrimVtxCountInfo_t _aPrimVtxCountInfo[FDRAW_PRIMTYPE_COUNT] = 
	{
	//  nStart	nRepeat	nBack
		1,		1,		0,	// FDRAW_PRIMTYPE_POINTS
		2,		2,		0,	// FDRAW_PRIMTYPE_LINELIST
		2,		1,		1,	// FDRAW_PRIMTYPE_LINESTRIP
		3,		3,		0,	// FDRAW_PRIMTYPE_TRILIST
		3,		1,		2,	// FDRAW_PRIMTYPE_TRISTRIP
		4,		1,		0,	// FDRAW_PRIMTYPE_QUADLIST
	};
#endif




static void _FillVertexBufferAndDrawPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *pSrcVtxArray, u32 nStartIndex, u32 nVtxCount );
static void _DrawPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *paSrcVertices, u32 nVtxCount );
static void _DrawIndexedPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *paSrcVertices, const u16 *paSrcIndices, u32 nVtxCount );
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );
#if FANG_PLATFORM_WIN
	static BOOL _CreateVertexBuffers( void );
	static BOOL _DeleteVertexBuffers(void);
#endif
static void _ClearPublicCaps( void );
static BOOL _ComputePublicCapsFromD3DCaps( void );
static void _FixupMapTables( void );
static int _SortMapTableCallback( const void *pArg1, const void *pArg2 );




BOOL fdx8draw_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	#if FANG_PLATFORM_WIN
		// Compute vertex count per VB...
		_nVertexCount = Fang_ConfigDefs.nDraw_D3DVBVertexCount;
		if( Fang_ConfigDefs.nDraw_D3DVBVertexCount < _MIN_VERTEX_COUNT ) 
		{
			// Invalid VB count...
			DEVPRINTF( "fdx8Draw_ModuleStartup(): Fang_ConfigDefs.nDraw_D3DVBVertexCount is invalid: %u. Must be at least %u.\n", Fang_ConfigDefs.nDraw_D3DVBVertexCount, _MIN_VERTEX_COUNT );
			DEVPRINTF( "                          Setting to %u.\n", _MIN_VERTEX_COUNT );
			_nVertexCount = _MIN_VERTEX_COUNT;
		}
		if( Fang_ConfigDefs.nDraw_D3DVBVertexCount > _MAX_VERTEX_COUNT ) 
		{
			// Invalid VB count...
			DEVPRINTF( "fdx8Draw_ModuleStartup(): Fang_ConfigDefs.nDraw_D3DVBVertexCount is invalid: %u. Maximum is %u.\n", Fang_ConfigDefs.nDraw_D3DVBVertexCount, _MAX_VERTEX_COUNT );
			DEVPRINTF( "                          Setting to %u.\n", _MAX_VERTEX_COUNT );
			_nVertexCount = _MAX_VERTEX_COUNT;
		}

		// For Windows, we create only one VB...
		_nVBCount = 1;

		// Create the VB array...
		_pVB = (FDX8VB_t *)fres_AllocAndZero( _nVBCount * sizeof(FDX8VB_t) );
		if( _pVB == NULL ) {
			DEVPRINTF( "fdx8Draw_ModuleStartup(): Insufficient memory to allocate %u bytes for VB array.\n", _nVBCount * sizeof(FDX8VB_t) );
			return FALSE;
		}
	#endif

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_ClearPublicCaps();
	_FixupMapTables();

	_nFogChangedKey = 0;
	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;

	return TRUE;
}

void fdx8draw_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

void fdraw_PrimList( FDrawPrimType_e nPrimType, const FDrawVtx_t *paVertices, u32 nVtxCount ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	_DrawPrim( nPrimType, paVertices, nVtxCount );
}

void fdraw_IndexedPrimList( FDrawPrimType_e nPrimType, const FDrawVtx_t *paVertices, const u16 *paIndices, u32 nIndexCount ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	_DrawIndexedPrim( nPrimType, paVertices, paIndices, nIndexCount );
}

void fdraw_SetLineWidth(f32 fWidth)
{
#if FANG_PLATFORM_XB
	FDX8_pDev->SetRenderState(D3DRS_EDGEANTIALIAS, !!(fWidth>1.0f));
	FDX8_pDev->SetRenderState(D3DRS_LINEWIDTH, *((DWORD*)(&fWidth)));
#endif
}

void fdx8draw_SetCullDir( FDrawCullDir_e nCullDir ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nCullDir>=0 && nCullDir<FDRAW_CULLDIR_COUNT );
	fdx8_SetRenderState_CULLMODE( _aMap_CullDir_D3DCULL[nCullDir].nD3DValue );
}

void fdx8draw_EnableClipping( BOOL bEnable ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_CLIPPING( bEnable );
	#endif
}

static FDrawColorFunc_e _nCurrentColorFunc;

void fdx8draw_Color_SetFunc( FDrawColorFunc_e nColorFunc ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nColorFunc>=0 && nColorFunc<FDRAW_COLORFUNC_COUNT );
	FASSERT( (1<<nColorFunc) & FDraw_nCapColorFunc );

	_nCurrentColorFunc = nColorFunc;

	if (nColorFunc == FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY)
	{
		FASSERT( _nPShader_Intensity );

		if ( _nPShader_Intensity )
		{
			FDX8_pDev->SetPixelShader(_nPShader_Intensity);
		}
		else
		{
			DEVPRINTF("fdx8draw: Intensity shader is invalid! Reverting to FDRAW_COLORFUNC_DECALTEX_AI.\n");
	
			FDX8_pDev->SetPixelShader(0);

			fdx8_SetTextureState_COLOROP( 0, (D3DTEXTUREOP)_aMap_ColorFunc_COLOROP[FDRAW_COLORFUNC_DECALTEX_AI].nD3DValue );
			fdx8_SetTextureState_COLORARG1( 0, _aMap_ColorFunc_COLORARG1[FDRAW_COLORFUNC_DECALTEX_AI].nD3DValue );
			fdx8_SetTextureState_COLORARG2( 0, _aMap_ColorFunc_COLORARG2[FDRAW_COLORFUNC_DECALTEX_AI].nD3DValue );
			fdx8_SetTextureState_ALPHAOP( 0, (D3DTEXTUREOP)_aMap_ColorFunc_ALPHAOP[FDRAW_COLORFUNC_DECALTEX_AI].nD3DValue );
			fdx8_SetTextureState_ALPHAARG1( 0, _aMap_ColorFunc_ALPHAARG1[FDRAW_COLORFUNC_DECALTEX_AI].nD3DValue );
			fdx8_SetTextureState_ALPHAARG2( 0, _aMap_ColorFunc_ALPHAARG2[FDRAW_COLORFUNC_DECALTEX_AI].nD3DValue );

			_nCurrentColorFunc = FDRAW_COLORFUNC_DECALTEX_AI;
		}
	}
	else
	{
		FDX8_pDev->SetPixelShader(0);

		fdx8_SetTextureState_COLOROP( 0, (D3DTEXTUREOP)_aMap_ColorFunc_COLOROP[nColorFunc].nD3DValue );
		fdx8_SetTextureState_COLORARG1( 0, _aMap_ColorFunc_COLORARG1[nColorFunc].nD3DValue );
		fdx8_SetTextureState_COLORARG2( 0, _aMap_ColorFunc_COLORARG2[nColorFunc].nD3DValue );
		fdx8_SetTextureState_ALPHAOP( 0, (D3DTEXTUREOP)_aMap_ColorFunc_ALPHAOP[nColorFunc].nD3DValue );
		fdx8_SetTextureState_ALPHAARG1( 0, _aMap_ColorFunc_ALPHAARG1[nColorFunc].nD3DValue );
		fdx8_SetTextureState_ALPHAARG2( 0, _aMap_ColorFunc_ALPHAARG2[nColorFunc].nD3DValue );
	}
}

void fdx8draw_SetTexture( CFTexInst *pTexInst ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	fdx8tex_SetTexture( 0, pTexInst, _nD3DTexCoordIndex );
}

void fdx8draw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nTexCoordXfmMode>=0 && nTexCoordXfmMode<FDRAW_TEXCOORDXFMMODE_COUNT );

	switch( nTexCoordXfmMode ) {
	case FDRAW_TEXCOORDXFMMODE_MTXONLY:
		_nD3DTexCoordIndex = 0;
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
		break;

	case FDRAW_TEXCOORDXFMMODE_CAMSPACE_NORMAL:
		_nD3DTexCoordIndex = D3DTSS_TCI_CAMERASPACENORMAL;
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
		break;

	case FDRAW_TEXCOORDXFMMODE_CAMSPACE_POS:
		_nD3DTexCoordIndex = D3DTSS_TCI_CAMERASPACEPOSITION;
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
		break;

	case FDRAW_TEXCOORDXFMMODE_CAMSPACE_REFLECTION:
		_nD3DTexCoordIndex = D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_COUNT2 );
		break;

	default:
		_nD3DTexCoordIndex = 0;
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_DISABLE );
		break;
	}
}

void fdx8draw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43A ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	fdx8xfm_SetCustomDXMatrix( D3DTS_TEXTURE0, pMtx43A, FALSE );
}

void fdx8draw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue ) {
	DWORD nD3DColorMaskFlags;
	BOOL bDisable;

	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( _nColorMaskType == _COLORMASKTYPE_NONE ) {
		return;
	}

	bMaskOffRed = !!bMaskOffRed;
	bMaskOffGreen = !!bMaskOffGreen;
	bMaskOffBlue = !!bMaskOffBlue;

	if( _nColorMaskType==_COLORMASKTYPE_DEPTH || _nColorMaskType==_COLORMASKTYPE_ALPHA ) {
		bDisable = (bMaskOffRed & bMaskOffGreen & bMaskOffBlue);
		bMaskOffRed = bDisable;
		bMaskOffGreen = bDisable;
		bMaskOffBlue = bDisable;
	}

	switch( _nColorMaskType ) {
	case _COLORMASKTYPE_RENDERSTATE:
		// We can mask writes to any combination of color buffer planes...

		nD3DColorMaskFlags = (D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);

		if( bMaskOffRed ) {
			nD3DColorMaskFlags &= ~D3DCOLORWRITEENABLE_RED;
		}
		if( bMaskOffGreen ) {
			nD3DColorMaskFlags &= ~D3DCOLORWRITEENABLE_GREEN;
		}
		if( bMaskOffBlue ) {
			nD3DColorMaskFlags &= ~D3DCOLORWRITEENABLE_BLUE;
		}

		fdx8_SetRenderState_COLORWRITEENABLE( nD3DColorMaskFlags );

		break;

	case _COLORMASKTYPE_DEPTH:
		// We can only mask all planes. Use the depth test...

		if( bDisable ) {
			if( fdraw_Depth_GetTest() != FDRAW_DEPTHTEST_NEVER ) {
				fdx8_SetRenderState_ZFUNC( _aMap_DepthTest_D3DCMPFUNC[FDRAW_DEPTHTEST_NEVER].nD3DValue );
			}
		} else {
			fdx8_SetRenderState_ZFUNC( _aMap_DepthTest_D3DCMPFUNC[ fdraw_Depth_GetTest() ].nD3DValue );
		}

		_bDisableColorViaDepth = bDisable;

		break;

	case _COLORMASKTYPE_ALPHA:
		// We can only mask all planes. Use the alpha test...

		if( bDisable ) {
			if( fdraw_Alpha_GetTest() != FDRAW_ALPHATEST_NEVER ) {
				fdx8_SetRenderState_ALPHAFUNC( _aMap_AlphaTest_D3DCMPFUNC[FDRAW_ALPHATEST_NEVER].nD3DValue );
			}
		} else {
			fdx8_SetRenderState_ALPHAFUNC( _aMap_AlphaTest_D3DCMPFUNC[ fdraw_Alpha_GetTest() ].nD3DValue );
		}

		_bDisableColorViaAlpha = bDisable;

		break;
	}
}

void fdx8draw_Color_EnableDither( BOOL bEnable ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	fdx8_SetRenderState_DITHERENABLE( bEnable );
}

void fdx8draw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nAlphaTest>=0 && nAlphaTest<FDRAW_ALPHATEST_COUNT );
	FASSERT( (1<<nAlphaTest) & FDraw_nCapAlphaTest );

	if( !_bDisableColorViaAlpha ) {
		fdx8_SetRenderState_ALPHAFUNC( _aMap_AlphaTest_D3DCMPFUNC[nAlphaTest].nD3DValue );
	}

	fdx8_SetRenderState_ALPHAREF( (DWORD)(fReference * 255.0f) );
}

void fdx8draw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nBlendOp>=0 && nBlendOp<FDRAW_BLENDOP_COUNT );
	FASSERT( (1<<nBlendOp) & FDraw_nCapBlendOp );

	fdx8_SetRenderState_SRCBLEND( (D3DBLEND)_aMap_BlendOp_D3DBLENDSRC[nBlendOp].nD3DValue );
	fdx8_SetRenderState_DESTBLEND( (D3DBLEND)_aMap_BlendOp_D3DBLENDDST[nBlendOp].nD3DValue );
}

void fdx8draw_Depth_EnableWriting( BOOL bEnable ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( bEnable || FDraw_bCanDisableDepthWrites );
	fdx8_SetRenderState_ZWRITEENABLE( bEnable );
}

void fdx8draw_Depth_SetTest( FDrawDepthTest_e nDepthTest ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nDepthTest>=0 && nDepthTest<FDRAW_DEPTHTEST_COUNT );
	FASSERT( (1<<nDepthTest) & FDraw_nCapDepthTest );

	if( !_bDisableColorViaDepth ) {
		fdx8_SetRenderState_ZFUNC( _aMap_DepthTest_D3DCMPFUNC[nDepthTest].nD3DValue );
	}
}

void fdx8draw_Depth_SetBiasLevel( u32 nNewLevel ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nNewLevel <= 16 );
	fdx8_SetRenderState_ZBIAS( nNewLevel );
}

void fdx8draw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
							   FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
							   FDrawStencilOp_e nDepthPassOp ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nStencilTest>=0 && nStencilTest<FDRAW_STENCILTEST_COUNT );
	FASSERT( nStencilFailOp>=0 && nStencilFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthFailOp>=0 && nDepthFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthPassOp>=0 && nDepthPassOp<FDRAW_STENCILOP_COUNT );

	if( FDraw_nStencilBitDepth == 0 ) {
		return;
	}

	if( !(FDX8_Caps.StencilCaps & _aMap_StencilOp_D3DSTENCILCAPS[nStencilFailOp].nD3DValue) ) {
		return;
	}

	if( !(FDX8_Caps.StencilCaps & _aMap_StencilOp_D3DSTENCILCAPS[nDepthFailOp].nD3DValue) ) {
		return;
	}

	if( !(FDX8_Caps.StencilCaps & _aMap_StencilOp_D3DSTENCILCAPS[nDepthPassOp].nD3DValue) ) {
		return;
	}

	if( (nStencilTest==FDRAW_STENCILTEST_ALWAYS) & (nDepthFailOp==FDRAW_STENCILOP_KEEP) & (nDepthPassOp==FDRAW_STENCILOP_KEEP) ) {
		// Disable the stencil buffer...
		fdx8_SetRenderState_STENCILENABLE( FALSE );
	} else {
		// Enable the stencil buffer...
		fdx8_SetRenderState_STENCILENABLE( TRUE );
		fdx8_SetRenderState_STENCILFUNC( (D3DCMPFUNC)_aMap_StencilTest_D3DSTENCILFUNC[nStencilTest].nD3DValue );
		fdx8_SetRenderState_STENCILFAIL( (D3DSTENCILOP)_aMap_StencilTest_D3DSTENCILFUNC[nStencilFailOp].nD3DValue );
		fdx8_SetRenderState_STENCILZFAIL( (D3DSTENCILOP)_aMap_StencilTest_D3DSTENCILFUNC[nDepthFailOp].nD3DValue );
		fdx8_SetRenderState_STENCILPASS( (D3DSTENCILOP)_aMap_StencilTest_D3DSTENCILFUNC[nDepthPassOp].nD3DValue );
		fdx8_SetRenderState_STENCILREF( nReference );
		fdx8_SetRenderState_STENCILMASK( nTestMask );
		fdx8_SetRenderState_STENCILWRITEMASK( nWriteMask );
	}
}

BOOL fdx8draw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	switch( _nColorMaskType ) {
	case _COLORMASKTYPE_RENDERSTATE:
		// We can mask writes to any combination of color buffer planes...
		return TRUE;

	case _COLORMASKTYPE_DEPTH:
	case _COLORMASKTYPE_ALPHA:
		// We can only mask all planes...
		return ( !bMaskOffRed==!bMaskOffGreen && !bMaskOffGreen==!bMaskOffBlue );

	case _COLORMASKTYPE_NONE:
		// Can't mask writes to color buffer...
		return FALSE;

	default:
		FASSERT_NOW;
	}

	return FALSE;
}

BOOL fdx8draw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp,
									  FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp ) {
	FASSERT( _bWindowCreated );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nStencilTest>=0 && nStencilTest<FDRAW_STENCILTEST_COUNT );
	FASSERT( nStencilFailOp>=0 && nStencilFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthFailOp>=0 && nDepthFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthPassOp>=0 && nDepthPassOp<FDRAW_STENCILOP_COUNT );

	if( FDraw_nStencilBitDepth ) {
		if( FDX8_Caps.StencilCaps & _aMap_StencilOp_D3DSTENCILCAPS[nStencilFailOp].nD3DValue ) {
			if( FDX8_Caps.StencilCaps & _aMap_StencilOp_D3DSTENCILCAPS[nDepthFailOp].nD3DValue ) {
				if( FDX8_Caps.StencilCaps & _aMap_StencilOp_D3DSTENCILCAPS[nDepthPassOp].nD3DValue ) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

#if FANG_PLATFORM_WIN
	static void _FillVertexBufferAndDrawPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *pSrcVtxArray, u32 nStartIndex, u32 nVtxCount ) 
	{
		FDX8VB_t *pVB;
		FDX8VB_C1T1_t *pD3DVtx;
		u32 i, nPrimCount;
		static const D3DPRIMITIVETYPE __anPrimType[FDRAW_PRIMTYPE_COUNT] = 
		{
			D3DPT_POINTLIST,		// FDRAW_PRIMTYPE_POINTS
			D3DPT_LINELIST,			// FDRAW_PRIMTYPE_LINELIST
			D3DPT_LINESTRIP,		// FDRAW_PRIMTYPE_LINESTRIP
			D3DPT_TRIANGLELIST,		// FDRAW_PRIMTYPE_TRILIST
			D3DPT_TRIANGLESTRIP,	// FDRAW_PRIMTYPE_TRISTRIP
//			D3DPT_TRIANGLEFAN,		// FDRAW_PRIMTYPE_TRIFAN
		};

		// Always use the same VB for Windows...
		FASSERT( _nCurrentVB == 0 );
		pVB = _pVB;

		// Copy vertex data into vertex buffer...
		pD3DVtx = (FDX8VB_C1T1_t *)fdx8vb_Lock( pVB, nStartIndex, nVtxCount );

		if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
		{
			const FDrawVtx_t *pEndVertex = &pSrcVtxArray[nVtxCount];
			while( pSrcVtxArray < pEndVertex ) 
			{
				pD3DVtx->fPosX = pSrcVtxArray[0].Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray[0].Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray[0].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray[0].ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray[0].ST.x;
				pD3DVtx->fT0 = pSrcVtxArray[0].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = pSrcVtxArray[1].Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray[1].Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray[1].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray[1].ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray[1].ST.x;
				pD3DVtx->fT0 = pSrcVtxArray[1].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = pSrcVtxArray[3].Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray[3].Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray[3].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray[3].ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray[3].ST.x;
				pD3DVtx->fT0 = pSrcVtxArray[3].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = pSrcVtxArray[1].Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray[1].Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray[1].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray[1].ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray[1].ST.x;
				pD3DVtx->fT0 = pSrcVtxArray[1].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = pSrcVtxArray[2].Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray[2].Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray[2].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray[2].ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray[2].ST.x;
				pD3DVtx->fT0 = pSrcVtxArray[2].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = pSrcVtxArray[3].Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray[3].Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray[3].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray[3].ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray[3].ST.x;
				pD3DVtx->fT0 = pSrcVtxArray[3].ST.y;
				pD3DVtx++;

				pSrcVtxArray += 4;
			}

			fdx8vb_Unlock( pVB );

			// Compute the number of primitives from the number of vertices, and draw...
			FDX8_pDev->DrawPrimitive( D3DPT_TRIANGLELIST, nStartIndex, (UINT)(nVtxCount * 0.5f) );
		}
		else
		{
			for( i = 0; i < nVtxCount; i++ ) 
			{
				pD3DVtx->fPosX = pSrcVtxArray->Pos_MS.x;
				pD3DVtx->fPosY = pSrcVtxArray->Pos_MS.y;
				pD3DVtx->fPosZ = pSrcVtxArray->Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pSrcVtxArray->ColorRGBA );
				pD3DVtx->fS0 = pSrcVtxArray->ST.x;
				pD3DVtx->fT0 = pSrcVtxArray->ST.y;

				pSrcVtxArray++;
				pD3DVtx++;
			}

			fdx8vb_Unlock( pVB );

			// Compute the number of primitives from the number of vertices, and draw...
			nPrimCount = 1 + (nVtxCount - _aPrimVtxCountInfo[nPrimType].nStart)/_aPrimVtxCountInfo[nPrimType].nRepeat;
			FDX8_pDev->DrawPrimitive( __anPrimType[nPrimType], nStartIndex, nPrimCount );
		}
	}

	static void _FillVertexBufferAndDrawIndexedPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *paSrcVertices, const u16 *paSrcIndices, u32 nStartIndex, u32 nIndexCount ) 
	{
		FDX8VB_t *pVB;
		FDX8VB_C1T1_t *pD3DVtx;
		u32 i, nPrimCount;
		static const D3DPRIMITIVETYPE __anPrimType[FDRAW_PRIMTYPE_COUNT] = 
		{
			D3DPT_POINTLIST,		// FDRAW_PRIMTYPE_POINTS
			D3DPT_LINELIST,			// FDRAW_PRIMTYPE_LINELIST
			D3DPT_LINESTRIP,		// FDRAW_PRIMTYPE_LINESTRIP
			D3DPT_TRIANGLELIST,		// FDRAW_PRIMTYPE_TRILIST
			D3DPT_TRIANGLESTRIP,	// FDRAW_PRIMTYPE_TRISTRIP
//			D3DPT_TRIANGLEFAN,		// FDRAW_PRIMTYPE_TRIFAN
		};

		// Always use the same VB for Windows...
		FASSERT( _nCurrentVB == 0 );
		pVB = _pVB;

		// Copy vertex data into vertex buffer...
		pD3DVtx = (FDX8VB_C1T1_t *)fdx8vb_Lock( pVB, nStartIndex, nIndexCount );

		const FDrawVtx_t *pCurrVert;
		if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
		{
			const u16 *paEndIndex = &paSrcIndices[nIndexCount];
			while( paSrcIndices < paEndIndex ) 
			{
				u16 nVert0 = *paSrcIndices; paSrcIndices++;
				u16 nVert1 = *paSrcIndices; paSrcIndices++;
				u16 nVert2 = *paSrcIndices; paSrcIndices++;
				u16 nVert3 = *paSrcIndices; paSrcIndices++;

				pD3DVtx->fPosX = paSrcVertices[nVert0].Pos_MS.x;
				pD3DVtx->fPosY = paSrcVertices[nVert0].Pos_MS.y;
				pD3DVtx->fPosZ = paSrcVertices[nVert0].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( paSrcVertices[nVert0].ColorRGBA );
				pD3DVtx->fS0 = paSrcVertices[nVert0].ST.x;
				pD3DVtx->fT0 = paSrcVertices[nVert0].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = paSrcVertices[nVert1].Pos_MS.x;
				pD3DVtx->fPosY = paSrcVertices[nVert1].Pos_MS.y;
				pD3DVtx->fPosZ = paSrcVertices[nVert1].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( paSrcVertices[nVert1].ColorRGBA );
				pD3DVtx->fS0 = paSrcVertices[nVert1].ST.x;
				pD3DVtx->fT0 = paSrcVertices[nVert1].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = paSrcVertices[nVert3].Pos_MS.x;
				pD3DVtx->fPosY = paSrcVertices[nVert3].Pos_MS.y;
				pD3DVtx->fPosZ = paSrcVertices[nVert3].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( paSrcVertices[nVert3].ColorRGBA );
				pD3DVtx->fS0 = paSrcVertices[nVert3].ST.x;
				pD3DVtx->fT0 = paSrcVertices[nVert3].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = paSrcVertices[nVert1].Pos_MS.x;
				pD3DVtx->fPosY = paSrcVertices[nVert1].Pos_MS.y;
				pD3DVtx->fPosZ = paSrcVertices[nVert1].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( paSrcVertices[nVert1].ColorRGBA );
				pD3DVtx->fS0 = paSrcVertices[nVert1].ST.x;
				pD3DVtx->fT0 = paSrcVertices[nVert1].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = paSrcVertices[nVert2].Pos_MS.x;
				pD3DVtx->fPosY = paSrcVertices[nVert2].Pos_MS.y;
				pD3DVtx->fPosZ = paSrcVertices[nVert2].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( paSrcVertices[nVert2].ColorRGBA );
				pD3DVtx->fS0 = paSrcVertices[nVert2].ST.x;
				pD3DVtx->fT0 = paSrcVertices[nVert2].ST.y;
				pD3DVtx++;

				pD3DVtx->fPosX = paSrcVertices[nVert3].Pos_MS.x;
				pD3DVtx->fPosY = paSrcVertices[nVert3].Pos_MS.y;
				pD3DVtx->fPosZ = paSrcVertices[nVert3].Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( paSrcVertices[nVert3].ColorRGBA );
				pD3DVtx->fS0 = paSrcVertices[nVert3].ST.x;
				pD3DVtx->fT0 = paSrcVertices[nVert3].ST.y;
				pD3DVtx++;
			}

			fdx8vb_Unlock( pVB );

			// Compute the number of primitives from the number of vertices, and draw...
			FDX8_pDev->DrawPrimitive( D3DPT_TRIANGLELIST, nStartIndex, (UINT)(nIndexCount * 0.5f) );
		}
		else
		{
			for( i = 0; i < nIndexCount; i++, paSrcIndices++, pD3DVtx++ ) 
			{
				pCurrVert = &paSrcVertices[ *paSrcIndices ];
				pD3DVtx->fPosX = pCurrVert->Pos_MS.x;
				pD3DVtx->fPosY = pCurrVert->Pos_MS.y;
				pD3DVtx->fPosZ = pCurrVert->Pos_MS.z;
				pD3DVtx->nDiffuseRGBA = FDX8_RGBA_TO_DXCOLOR( pCurrVert->ColorRGBA );
				pD3DVtx->fS0 = pCurrVert->ST.x;
				pD3DVtx->fT0 = pCurrVert->ST.y;
			}

			fdx8vb_Unlock( pVB );

			// Compute the number of primitives from the number of vertices, and draw...
			nPrimCount = 1 + (nIndexCount - _aPrimVtxCountInfo[nPrimType].nStart)/_aPrimVtxCountInfo[nPrimType].nRepeat;
			FDX8_pDev->DrawPrimitive( __anPrimType[nPrimType], nStartIndex, nPrimCount );
		}
	}
#endif


//
//
//
static void _DrawIndexedPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *paSrcVertices, const u16 *paSrcIndices, u32 nIndexCount ) 
{
	if ( nIndexCount == 0 ) 
	{
		// Exit if no vertices...
		return;
	}

	// Set fog...
	if ( FRenderer_bFogEnabled && _nFogChangedKey != FRenderer_nFogChangedKey ) 
	{
		// Fog values changed since last time this function was called...

		_nFogChangedKey = FRenderer_nFogChangedKey;

		frenderer_Fog_ComputeColor();
	}

	// Set fill mode...
	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	fdx8xfm_SetDXMatrices( FALSE );

#if FANG_PLATFORM_XB
	static const D3DPRIMITIVETYPE __anPrimType[FDRAW_PRIMTYPE_COUNT] = {
		D3DPT_POINTLIST,		// FDRAW_PRIMTYPE_POINTS
		D3DPT_LINELIST,			// FDRAW_PRIMTYPE_LINELIST
		D3DPT_LINESTRIP,		// FDRAW_PRIMTYPE_LINESTRIP
		D3DPT_TRIANGLELIST,		// FDRAW_PRIMTYPE_TRILIST
		D3DPT_TRIANGLESTRIP,	// FDRAW_PRIMTYPE_TRISTRIP
//		D3DPT_TRIANGLEFAN,		// FDRAW_PRIMTYPE_TRIFAN
		D3DPT_QUADLIST,			// FDRAW_PRIMTYPE_QUADLIST
	};

	if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
	{
		FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	}
	else
	{
		FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );
	}

	u32 i, uBytesPerVertex, uMaxVertCountPerArray;
	if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
	{
		uBytesPerVertex = (sizeof(f32) * 3) + sizeof(u32);
		uMaxVertCountPerArray = 508; // We can submit a max of 2047 DWORDS of data at a time
	}
	else
	{
		uBytesPerVertex = (sizeof(f32) * 3) + sizeof(u32) + (sizeof(f32) * 2);
		uMaxVertCountPerArray = 340; // We can submit a max of 2047 DWORDS of data at a time
	}

	const FDrawVtx_t *pCurrVert;
	if ( nIndexCount <= uMaxVertCountPerArray )
	{
		u32 uPushSizeInDWORDs = (uBytesPerVertex * nIndexCount) >> 2;

		// Gain direct access to the pushbuffer. Note that we have 
		// to reserve overhead for the encoding parameters.
		DWORD* pPush;
		if ( FDX8_pDev->BeginPush( uPushSizeInDWORDs + 5, &pPush ) != S_OK )
		{
			return;
		}

		// Push the macro that start things off
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
		
		// Specify the primitive type of the vertices that follow
		*pPush++ = __anPrimType[nPrimType];

		// Specify that an array of vertices comes next. Note that a max
		// of 2047 DWORDs can be specified in an INLINE_ARRAY section so
		// for handling more vertex data than that, we simply split the data
		// into multiple INLINE_ARRAY sections, thus the point of the while loop.
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG|D3DPUSH_INLINE_ARRAY, uPushSizeInDWORDs );

		// Write the dynamic vertices
		if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
		{
			for( i = 0; i < nIndexCount; i++, paSrcIndices++ ) 
			{
				pCurrVert = &paSrcVertices[ *paSrcIndices ];

				// Write the position
				*((CFVec3*)pPush) = pCurrVert->Pos_MS;
				pPush += 3;

				// Write the diffuse color
				*((D3DCOLOR*)pPush) = ((u32)(pCurrVert->ColorRGBA.fAlpha * 255)<<24) | ((u32)(pCurrVert->ColorRGBA.fRed * 255)<<16) | ((u32)(pCurrVert->ColorRGBA.fGreen * 255)<<8) | ((u32)(pCurrVert->ColorRGBA.fBlue * 255));
				pPush += 1;
			}
		}
		else
		{
			for( i = 0; i < nIndexCount; i++, paSrcIndices++ ) 
			{
				pCurrVert = &paSrcVertices[ *paSrcIndices ];

				// Write the position
				*((CFVec3*)pPush) = pCurrVert->Pos_MS;
				pPush += 3;

				// Write the diffuse color
				*((D3DCOLOR*)pPush) = ((u8)(pCurrVert->ColorRGBA.fAlpha * 255)<<24) | ((u8)(pCurrVert->ColorRGBA.fRed * 255)<<16) | ((u8)(pCurrVert->ColorRGBA.fGreen * 255)<<8) | ((u8)(pCurrVert->ColorRGBA.fBlue * 255));
				pPush += 1;

				// Write the texture ST
				*((CFVec2*)pPush) = pCurrVert->ST;
				pPush += 2;
			}
		}

		// Push the macros that finish things off
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
		*pPush++ = 0;

		FDX8_pDev->EndPush( pPush );
	}
	else
	{
		if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
		{
			FDX8_pDev->Begin( __anPrimType[nPrimType] );
			for( i = 0; i < nIndexCount; i++, paSrcIndices++ ) 
			{
				pCurrVert = &paSrcVertices[ *paSrcIndices ];
				FDX8_pDev->SetVertexData4f( D3DVSDE_DIFFUSE, pCurrVert->ColorRGBA.fRed, pCurrVert->ColorRGBA.fGreen, pCurrVert->ColorRGBA.fBlue, pCurrVert->ColorRGBA.fAlpha );
				FDX8_pDev->SetVertexData4f( D3DVSDE_VERTEX, pCurrVert->Pos_MS.x, pCurrVert->Pos_MS.y, pCurrVert->Pos_MS.z, 1.f );
			}
			FDX8_pDev->End();
		}
		else
		{
			FDX8_pDev->Begin( __anPrimType[nPrimType] );
			for( i = 0; i < nIndexCount; i++, paSrcIndices++ ) 
			{
				pCurrVert = &paSrcVertices[ *paSrcIndices ];
				FDX8_pDev->SetVertexData4f( D3DVSDE_DIFFUSE, pCurrVert->ColorRGBA.fRed, pCurrVert->ColorRGBA.fGreen, pCurrVert->ColorRGBA.fBlue, pCurrVert->ColorRGBA.fAlpha );
				FDX8_pDev->SetVertexData2f( D3DVSDE_TEXCOORD0, pCurrVert->ST.x, pCurrVert->ST.y );
				FDX8_pDev->SetVertexData4f( D3DVSDE_VERTEX, pCurrVert->Pos_MS.x, pCurrVert->Pos_MS.y, pCurrVert->Pos_MS.z, 1.f );
			}
			FDX8_pDev->End();
		}
	}
#else
	u32 nFreeElementCount, nElementCount, nPrimVtxStartCount, nPrimVtxRepeatCount, nPrimVtxBackCount;
	s32 nMaxPrimCount;

	nPrimVtxStartCount = _aPrimVtxCountInfo[nPrimType].nStart;
	nPrimVtxRepeatCount = _aPrimVtxCountInfo[nPrimType].nRepeat;
	nPrimVtxBackCount = _aPrimVtxCountInfo[nPrimType].nBack;

	FASSERT( nIndexCount >= nPrimVtxStartCount );
	FASSERT( ((nIndexCount - nPrimVtxStartCount) % nPrimVtxRepeatCount) == 0 );

	if( nPrimType == FDRAW_PRIMTYPE_TRISTRIP ) 
	{
		// We need to split triangle strips on even-triangle boundaries
		// to keep the vertex winding polarity the same...
		nPrimVtxStartCount++;
		nPrimVtxRepeatCount++;
	}

	while( nIndexCount ) 
	{
		// Compute the number of free elements in our vertex buffer...
		nFreeElementCount = (_nVertexCount - _nNextFreeVtxIndex);

		u32 nElementsRequired = nIndexCount;
		if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
		{
			nElementsRequired = (u32)(nElementsRequired * 1.5f);

			if( 6 > nFreeElementCount ) 
			{
				// There's not enough room in the vertex buffer to store one complete
				// primitive. We'll use the D3DLOCK_DISCARD flag to allow DX to give
				// us another vertex buffer, and set our pointers to reflect the
				// completely empty vertex buffer...

				_nNextFreeVtxIndex = 0;
				nFreeElementCount = _nVertexCount;
			}
		}
		else
		{
			if( nPrimVtxStartCount > nFreeElementCount ) 
			{
				// There's not enough room in the vertex buffer to store one complete
				// primitive. We'll use the D3DLOCK_DISCARD flag to allow DX to give
				// us another vertex buffer, and set our pointers to reflect the
				// completely empty vertex buffer...

				_nNextFreeVtxIndex = 0;
				nFreeElementCount = _nVertexCount;
			}
		}

		if( nElementsRequired > nFreeElementCount ) 
		{
			// There's not enough room in the vertex buffer to store one complete
			// primitive. We'll use the D3DLOCK_DISCARD flag to allow DX to give
			// us another vertex buffer, and set our pointers to reflect the
			// completely empty vertex buffer...

			_nNextFreeVtxIndex = 0;
			nFreeElementCount = _nVertexCount;
		}

		if( nIndexCount > nFreeElementCount ) 
		{
			// Too many vertices to fit into remainder of the vertex buffer...

			switch( nPrimType ) 
			{
				case FDRAW_PRIMTYPE_POINTS:
					nMaxPrimCount = nFreeElementCount;
					nElementCount = nFreeElementCount;
					break;

				case FDRAW_PRIMTYPE_LINELIST:
					nMaxPrimCount = nFreeElementCount >> 1;
					nElementCount = nMaxPrimCount << 1;
					break;

				case FDRAW_PRIMTYPE_LINESTRIP:
					nMaxPrimCount = nFreeElementCount - 1;
					nElementCount = nMaxPrimCount + 1;
					break;

				case FDRAW_PRIMTYPE_TRILIST:
					nMaxPrimCount = nFreeElementCount / 3;
					nElementCount = nMaxPrimCount * 3;
					break;

				case FDRAW_PRIMTYPE_TRISTRIP:
					nMaxPrimCount = ((nFreeElementCount>>1) << 1) - 2;
					nElementCount = nMaxPrimCount + 2;
					break;

				case FDRAW_PRIMTYPE_QUADLIST:
					nMaxPrimCount = nFreeElementCount / 6;
					nElementCount = nMaxPrimCount * 4;
					break;
			}

			if( nMaxPrimCount > 0 ) 
			{
				_FillVertexBufferAndDrawIndexedPrim( nPrimType, paSrcVertices, paSrcIndices, _nNextFreeVtxIndex, nElementCount );

				if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
				{
					_nNextFreeVtxIndex += nMaxPrimCount * 6;
					paSrcVertices += nElementCount;
					paSrcIndices += nElementCount;
					nIndexCount -= nElementCount;
				}
				else
				{
					_nNextFreeVtxIndex += nElementCount;
					nElementCount -= nPrimVtxBackCount;
					paSrcVertices += nElementCount;
					paSrcIndices += nElementCount;
					nIndexCount -= nElementCount;
				}
			}
		} 
		else 
		{
			// Remaining vertices all fit into the remainder of the vertex buffer...

			_FillVertexBufferAndDrawIndexedPrim( nPrimType, paSrcVertices, paSrcIndices, _nNextFreeVtxIndex, nIndexCount );
			if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
			{
				_nNextFreeVtxIndex += (u32)(nIndexCount * 1.5f);
			}
			else
			{
				_nNextFreeVtxIndex += nIndexCount;
			}
			return;
		}
	}
#endif
}


//
//
//
static void _DrawPrim( FDrawPrimType_e nPrimType, const FDrawVtx_t *paSrcVertices, u32 nVtxCount ) 
{
	if ( nVtxCount == 0 ) 
	{
		// Exit if no vertices...
		return;
	}

	// Set fog...
	if ( FRenderer_bFogEnabled && _nFogChangedKey != FRenderer_nFogChangedKey ) 
	{
		// Fog values changed since last time this function was called...

		_nFogChangedKey = FRenderer_nFogChangedKey;

		frenderer_Fog_ComputeColor();
	}

	// Set fill mode...
	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	fdx8xfm_SetDXMatrices( FALSE );

#if FANG_PLATFORM_XB
	static const D3DPRIMITIVETYPE __anPrimType[FDRAW_PRIMTYPE_COUNT] = {
		D3DPT_POINTLIST,		// FDRAW_PRIMTYPE_POINTS
		D3DPT_LINELIST,			// FDRAW_PRIMTYPE_LINELIST
		D3DPT_LINESTRIP,		// FDRAW_PRIMTYPE_LINESTRIP
		D3DPT_TRIANGLELIST,		// FDRAW_PRIMTYPE_TRILIST
		D3DPT_TRIANGLESTRIP,	// FDRAW_PRIMTYPE_TRISTRIP
//		D3DPT_TRIANGLEFAN,		// FDRAW_PRIMTYPE_TRIFAN
		D3DPT_QUADLIST,			// FDRAW_PRIMTYPE_QUADLIST
	};

	if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
	{
		FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	}
	else
	{
		FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );
	}

	u32 i, uBytesPerVertex, uMaxVertCountPerArray;
	if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
	{
		uBytesPerVertex = (sizeof(f32) * 3) + sizeof(u32);
		uMaxVertCountPerArray = 508; // We can submit a max of 2047 DWORDS of data at a time
	}
	else
	{
		uBytesPerVertex = (sizeof(f32) * 3) + sizeof(u32) + (sizeof(f32) * 2);
		uMaxVertCountPerArray = 340; // We can submit a max of 2047 DWORDS of data at a time
	}

	if ( nVtxCount <= uMaxVertCountPerArray )
	{
		u32 uPushSizeInDWORDs = (uBytesPerVertex * nVtxCount) >> 2;

		// Gain direct access to the pushbuffer. Note that we have 
		// to reserve overhead for the encoding parameters.
		DWORD* pPush;
		if ( FDX8_pDev->BeginPush( uPushSizeInDWORDs + 5, &pPush ) != S_OK )
		{
			return;
		}

		// Push the macro that start things off
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
		
		// Specify the primitive type of the vertices that follow
		*pPush++ = __anPrimType[nPrimType];

		// Specify that an array of vertices comes next. Note that a max
		// of 2047 DWORDs can be specified in an INLINE_ARRAY section so
		// for handling more vertex data than that, we simply split the data
		// into multiple INLINE_ARRAY sections, thus the point of the while loop.
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG|D3DPUSH_INLINE_ARRAY, uPushSizeInDWORDs );

		// Write the dynamic vertices
		if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
		{
			for( i = 0; i < nVtxCount; i++, paSrcVertices++ ) 
			{
				// Write the position
				*((CFVec3*)pPush) = paSrcVertices->Pos_MS;
				pPush += 3;

				// Write the diffuse color
				*((D3DCOLOR*)pPush) = ((u32)(paSrcVertices->ColorRGBA.fAlpha * 255)<<24) | ((u32)(paSrcVertices->ColorRGBA.fRed * 255)<<16) | ((u32)(paSrcVertices->ColorRGBA.fGreen * 255)<<8) | ((u32)(paSrcVertices->ColorRGBA.fBlue * 255));
				pPush += 1;
			}
		}
		else
		{
			for( i = 0; i < nVtxCount; i++, paSrcVertices++ ) 
			{
				// Write the position
				*((CFVec3*)pPush) = paSrcVertices->Pos_MS;
				pPush += 3;

				// Write the diffuse color
				*((D3DCOLOR*)pPush) = ((u8)(paSrcVertices->ColorRGBA.fAlpha * 255)<<24) | ((u8)(paSrcVertices->ColorRGBA.fRed * 255)<<16) | ((u8)(paSrcVertices->ColorRGBA.fGreen * 255)<<8) | ((u8)(paSrcVertices->ColorRGBA.fBlue * 255));
				pPush += 1;

				// Write the texture ST
				*((CFVec2*)pPush) = paSrcVertices->ST;
				pPush += 2;
			}
		}

		// Push the macros that finish things off
		*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
		*pPush++ = 0;

		FDX8_pDev->EndPush( pPush );
	}
	else
	{
		if ( _nCurrentColorFunc == FDRAW_COLORFUNC_DECAL_AI )
		{
			FDX8_pDev->Begin( __anPrimType[nPrimType] );
			for( i = 0; i < nVtxCount; i++, paSrcVertices++ ) 
			{
				FDX8_pDev->SetVertexData4f( D3DVSDE_DIFFUSE, paSrcVertices->ColorRGBA.fRed, paSrcVertices->ColorRGBA.fGreen, paSrcVertices->ColorRGBA.fBlue, paSrcVertices->ColorRGBA.fAlpha );
				FDX8_pDev->SetVertexData4f( D3DVSDE_VERTEX, paSrcVertices->Pos_MS.x, paSrcVertices->Pos_MS.y, paSrcVertices->Pos_MS.z, 1.f );
			}
			FDX8_pDev->End();
		}
		else
		{
			FDX8_pDev->Begin( __anPrimType[nPrimType] );
			for( i=0; i<nVtxCount; i++, paSrcVertices++ ) 
			{
				FDX8_pDev->SetVertexData4f( D3DVSDE_DIFFUSE, paSrcVertices->ColorRGBA.fRed, paSrcVertices->ColorRGBA.fGreen, paSrcVertices->ColorRGBA.fBlue, paSrcVertices->ColorRGBA.fAlpha );
				FDX8_pDev->SetVertexData2f( D3DVSDE_TEXCOORD0, paSrcVertices->ST.x, paSrcVertices->ST.y );
				FDX8_pDev->SetVertexData4f( D3DVSDE_VERTEX, paSrcVertices->Pos_MS.x, paSrcVertices->Pos_MS.y, paSrcVertices->Pos_MS.z, 1.f );
			}
			FDX8_pDev->End();
		}
	}
#else
	u32 nFreeElementCount, nElementCount, nPrimVtxStartCount, nPrimVtxRepeatCount, nPrimVtxBackCount;
	s32 nMaxPrimCount;

	nPrimVtxStartCount = _aPrimVtxCountInfo[nPrimType].nStart;
	nPrimVtxRepeatCount = _aPrimVtxCountInfo[nPrimType].nRepeat;
	nPrimVtxBackCount = _aPrimVtxCountInfo[nPrimType].nBack;

	FASSERT( nVtxCount >= nPrimVtxStartCount );
	FASSERT( ((nVtxCount - nPrimVtxStartCount) % nPrimVtxRepeatCount) == 0 );

	if( nPrimType == FDRAW_PRIMTYPE_TRISTRIP ) 
	{
		// We need to split triangle strips on even-triangle boundaries
		// to keep the vertex winding polarity the same...
		nPrimVtxStartCount++;
		nPrimVtxRepeatCount++;
	}

	while( nVtxCount ) 
	{
		// Compute the number of free elements in our vertex buffer...
		nFreeElementCount = (_nVertexCount - _nNextFreeVtxIndex);

		u32 nElementsRequired = nVtxCount;
		if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
		{
			nElementsRequired = (u32)(nElementsRequired * 1.5f);

			if( 6 > nFreeElementCount ) 
			{
				// There's not enough room in the vertex buffer to store one complete
				// primitive. We'll use the D3DLOCK_DISCARD flag to allow DX to give
				// us another vertex buffer, and set our pointers to reflect the
				// completely empty vertex buffer...

				_nNextFreeVtxIndex = 0;
				nFreeElementCount = _nVertexCount;
			}
		}
		else
		{
			if( nPrimVtxStartCount > nFreeElementCount ) 
			{
				// There's not enough room in the vertex buffer to store one complete
				// primitive. We'll use the D3DLOCK_DISCARD flag to allow DX to give
				// us another vertex buffer, and set our pointers to reflect the
				// completely empty vertex buffer...

				_nNextFreeVtxIndex = 0;
				nFreeElementCount = _nVertexCount;
			}
		}

		if( nElementsRequired > nFreeElementCount ) 
		{
			// Too many vertices to fit into remainder of the vertex buffer...

			switch( nPrimType ) 
			{
				case FDRAW_PRIMTYPE_POINTS:
					nMaxPrimCount = nFreeElementCount;
					nElementCount = nFreeElementCount;
					break;

				case FDRAW_PRIMTYPE_LINELIST:
					nMaxPrimCount = nFreeElementCount >> 1;
					nElementCount = nMaxPrimCount << 1;
					break;

				case FDRAW_PRIMTYPE_LINESTRIP:
					nMaxPrimCount = nFreeElementCount - 1;
					nElementCount = nMaxPrimCount + 1;
					break;

				case FDRAW_PRIMTYPE_TRILIST:
					nMaxPrimCount = nFreeElementCount / 3;
					nElementCount = nMaxPrimCount * 3;
					break;

				case FDRAW_PRIMTYPE_TRISTRIP:
					nMaxPrimCount = ((nFreeElementCount>>1) << 1) - 2;
					nElementCount = nMaxPrimCount + 2;
					break;

				case FDRAW_PRIMTYPE_QUADLIST:
					nMaxPrimCount = nFreeElementCount / 6;
					nElementCount = nMaxPrimCount * 4;
					break;
			}

			if( nMaxPrimCount > 0 ) 
			{
				_FillVertexBufferAndDrawPrim( nPrimType, paSrcVertices, _nNextFreeVtxIndex, nElementCount );

				if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
				{
					_nNextFreeVtxIndex += nMaxPrimCount * 6;
					paSrcVertices += nElementCount;
					nVtxCount -= nElementCount;
				}
				else
				{
					_nNextFreeVtxIndex += nElementCount;
					nElementCount -= nPrimVtxBackCount;
					paSrcVertices += nElementCount;
					nVtxCount -= nElementCount;
				}
			}
		} 
		else 
		{
			// Remaining vertices all fit into the remainder of the vertex buffer...

			_FillVertexBufferAndDrawPrim( nPrimType, paSrcVertices, _nNextFreeVtxIndex, nVtxCount );
			if ( nPrimType == FDRAW_PRIMTYPE_QUADLIST )
			{
				_nNextFreeVtxIndex += (u32)(nVtxCount * 1.5f);
			}
			else
			{
				_nNextFreeVtxIndex += nVtxCount;
			}
			return;
		}
	}
#endif
}

void fdx8draw_Renderer_Open( void ) {
	// Init render states...
	fdx8_SetRenderState_ALPHABLENDENABLE( TRUE );
	fdx8_SetRenderState_ALPHATESTENABLE( TRUE );
	fdx8_SetRenderState_SHADEMODE( D3DSHADE_GOURAUD );
	fdx8_SetRenderState_SPECULARENABLE( FALSE );
	fdx8_SetRenderState_LIGHTING( FALSE );
	fdx8_SetRenderState_LOCALVIEWER( TRUE );
	fdx8_SetRenderState_NORMALIZENORMALS( FALSE );
	fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
	fdx8_SetRenderState_BLENDOP( D3DBLENDOP_ADD );
	fdx8_SetRenderState_POINTSPRITEENABLE( FALSE );
	fdx8_SetRenderState_POINTSCALEENABLE( FALSE );

	// Init texture stage states...
	fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( 0, D3DTTFF_DISABLE );
	fdx8_SetTextureState_COLOROP( 1, D3DTOP_DISABLE );
	fdx8_SetTextureState_ALPHAOP( 1, D3DTOP_DISABLE );

	#if FANG_PLATFORM_XB
		FDX8_pDev->SetPixelShader( 0 );
		FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );
		fdx8sh_SetVertexType( 0 );  // Clear the shader vertex type cache
	#endif

	// Select the VB for Windows...
	#if FANG_PLATFORM_WIN
		fdx8vb_Select( _pVB );
	#endif

	// No texture...
	fdx8tex_SetTexture( 0, NULL, 0 );

	// Default fog...
	fdx8_SetRenderState_FOGENABLE( FALSE );
	fdx8_SetRenderState_FOGTABLEMODE( D3DFOG_LINEAR );
	fdx8_SetRenderState_RANGEFOGENABLE( FALSE );
	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_FOGVERTEXMODE( D3DFOG_LINEAR );
	#endif

	// Invalidate fog key...
	_nFogChangedKey = FRenderer_nFogChangedKey - 1;
}

void fdx8draw_Renderer_Close( void ) {
	FDX8_pDev->SetPixelShader(0);
}

static u32 _CreateIntensityShader()
{
	u32 nHandle=0;
	HRESULT hr;

	#if FANG_PLATFORM_WIN
		hr = FDX8_pDev->CreatePixelShader((const DWORD *)(dwFdx8IntensityPixelShader), (DWORD *)&nHandle);
	#else
		hr = FDX8_pDev->CreatePixelShader((const D3DPIXELSHADERDEF *)(dwCompiledPShaderfxbIntensityPixelShader), (DWORD *)&nHandle);
	#endif

	if ( FAILED(hr) )
	{
		DEVPRINTF("fdx8draw: Failed creating Intensity shader!\n");
		nHandle = 0;
	}

	return (nHandle);
}

static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) {
	case FDX8VID_EVENT_WINDOW_CREATED:
		_bWindowCreated = TRUE;

		_nPShader_Intensity = _CreateIntensityShader();

		if( !_ComputePublicCapsFromD3DCaps() ) {
			_ClearPublicCaps();
			_bWindowCreated = FALSE;
			return FALSE;
		}

		#if FANG_PLATFORM_WIN
			if( !_CreateVertexBuffers() ) {
				_bWindowCreated = FALSE;
				return FALSE;
			}
		#endif

		if( FDX8_Caps.MaxPrimitiveCount >= 2 ) {
			// Record the maximum number of primitives the hardware will allow
			// in a single call to DrawPrimitive()...
			_nMaxD3DDrawPrimElements = FDX8_Caps.MaxPrimitiveCount;
		} else {
			if( FDX8_Caps.MaxPrimitiveCount == 0 ) {
				// Hmmmm, the driver doesn't appear to be setting the MaxPrimitiveCount
				// field. Let's just assume it can draw a reasonable number of primitives...
				_nMaxD3DDrawPrimElements = 255;
			} else {
				// Hmmmm, the hardware supports only a single primitive at a time.
				// We need a minimum of 2, so we'll have to fail. What lousy hardware!
				_bWindowCreated = FALSE;
				return FALSE;
			}
		}

		break;

	case FDX8VID_EVENT_WINDOW_DESTROYED:
		_ClearPublicCaps();
		#if FANG_PLATFORM_WIN
			_DeleteVertexBuffers();
		#endif
		_bWindowCreated = FALSE;

		break;

	case FDX8VID_EVENT_PRE_RESET:
		break;

	case FDX8VID_EVENT_POST_RESET:
		#if FANG_PLATFORM_WIN
			_nCurrentVB = 0;
			_nNextFreeVtxIndex = 0;
		#endif
		break;
	}

	return TRUE;
}

#if FANG_PLATFORM_WIN
	static BOOL _CreateVertexBuffers( void ) {
		u32 i;

		for( i=0; i<_nVBCount; i++ ) {
			if( !fdx8vb_CreateFVF( &_pVB[i], _nVertexCount, FDX8VB_TYPE_C1T1, TRUE, FALSE ) ) {
				// Unable to create VB...
				DEVPRINTF( "fdx8draw.cpp::_CreateVertexBuffers(): Could not create VB.\n" );
				return FALSE;
			}
		}

		_nCurrentVB = 0;
		_nNextFreeVtxIndex = 0;

		return TRUE;
	}

	static BOOL _DeleteVertexBuffers( void ) {
		u32 i, nCnt;

		for( i=0; i<_nVBCount; i++ ) {
			if ( _pVB[i].pDXVB )
			{
				nCnt = _pVB[i].pDXVB->Release();
				_pVB[i].pDXVB = NULL;
			}
		}

		_nCurrentVB = 0;
		_nNextFreeVtxIndex = 0;

		return TRUE;
	}
#endif

static void _ClearPublicCaps( void ) {
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

static BOOL _ComputePublicCapsFromD3DCaps( void ) {
	// Color function...
	FDraw_nCapColorFunc = 0;
	if( FDX8_Caps.TextureOpCaps & D3DTEXOPCAPS_SELECTARG1 ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_DECAL_AI |
								 FDRAW_CAP_COLORFUNC_DECALTEX_AI |
								 FDRAW_CAP_COLORFUNC_DECALTEX_AT;
	}
	if( (FDX8_Caps.TextureOpCaps & (D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_MODULATE)) == (D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_MODULATE) ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_DIFFUSETEX_AI |
								 FDRAW_CAP_COLORFUNC_DIFFUSETEX_AT;
	}
	if( FDX8_Caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_DIFFUSETEX_AIAT;
	}
	if( (FDX8_Caps.TextureOpCaps & (D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)) == (D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA) ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_SPECULARTEX_AT;
	}
	if( !(FDraw_nCapColorFunc & FDRAW_CAP_COLORFUNC_DECAL_AI) ) {
		return FALSE;
	}
	if( FDX8_Caps.TextureOpCaps & D3DTEXOPCAPS_ADD ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_ADD;
	}
	if( (FDX8_Caps.TextureOpCaps & (D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_BLENDDIFFUSEALPHA)) == (D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_BLENDDIFFUSEALPHA) ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_BLEND_AIPLUSAT;
	}
	if( (FDX8_Caps.TextureOpCaps & (D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)) == (D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR) ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_CT_PLUS_CIAT_AI;
	}
	if( FDX8_Caps.PixelShaderVersion > 0 ) {
		FDraw_nCapColorFunc |= FDRAW_CAP_COLORFUNC_DECALTEX_AI_INTENSITY;
	}

	// Alpha test...
	FDraw_nCapAlphaTest = 0;
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_ALWAYS ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_ALWAYS;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_NEVER ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_NEVER;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_LESS ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_LESS;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_LEQUAL;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_EQUAL ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_EQUAL;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_NOTEQUAL;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_GEQUAL;
	}
	if( FDX8_Caps.AlphaCmpCaps & D3DPCMPCAPS_GREATER ) {
		FDraw_nCapAlphaTest |= FDRAW_CAP_ALPHATEST_GREATER;
	}
	if( !(FDraw_nCapAlphaTest & FDRAW_CAP_ALPHATEST_ALWAYS) ) {
		return FALSE;
	}

	// Alpha blending...
	FDraw_nCapBlendOp = 0;
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ONE) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ZERO) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_SRC;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ONE) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_DST;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ZERO) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_ZERO;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ONE) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ONE) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_SRC_PLUS_DST;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ZERO) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_SRCALPHA) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ZERO) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_INVALPHA_TIMES_SRC;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_INVALPHA_TIMES_DST;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_ONE) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_ONE) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_SRCALPHA) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_OPAQUE;
	}
	if( (FDX8_Caps.SrcBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) && (FDX8_Caps.DestBlendCaps & D3DPBLENDCAPS_SRCALPHA) ) {
		FDraw_nCapBlendOp |= FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_XPARENT;
	}
	if( !(FDraw_nCapBlendOp & FDRAW_CAP_BLENDOP_SRC) ) {
		return FALSE;
	}

	// Depth buffer test...
	FDraw_nCapDepthTest = 0;
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_ALWAYS ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_ALWAYS;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_NEVER ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_NEVER;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_LESS ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_CLOSER;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_LESSEQUAL ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_CLOSER_OR_EQUAL;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_EQUAL ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_EQUAL;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_NOTEQUAL ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_NOTEQUAL;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_GREATER ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_FARTHER;
	}
	if( FDX8_Caps.ZCmpCaps & D3DPCMPCAPS_GREATEREQUAL ) {
		FDraw_nCapDepthTest |= FDRAW_CAP_DEPTHTEST_FARTHER_OR_EQUAL;
	}
	if( !(FDraw_nCapDepthTest & FDRAW_CAP_DEPTHTEST_ALWAYS) ) {
		return FALSE;
	}

	FDraw_nDepthBitDepth = FVid_Mode.nDepthBits;
	FDraw_nStencilBitDepth = FVid_Mode.nStencilBits;
	FDraw_nStencilBitMask = (1 << FVid_Mode.nStencilBits) - 1;

	if( FDX8_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_MASKZ ) {
		FDraw_bCanDisableDepthWrites = TRUE;
	} else {
		FDraw_bCanDisableDepthWrites = FALSE;
	}

	if( FDX8_Caps.RasterCaps & D3DPRASTERCAPS_ZBIAS ) {
		FDraw_nMaxDepthBiasValue = 16;
	} else {
		FDraw_nMaxDepthBiasValue = 0;
	}

	// Determine how we can mask writes to the color buffer...
	if( FDX8_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE ) {
		// Ah, we can simply use the D3DRS_COLORWRITEENABLE renderstate to
		// disable writes to the color buffer on this card...
		_nColorMaskType = _COLORMASKTYPE_RENDERSTATE;
	} else {
		// This video card doesn't allow us to mask off individual color planes.
		// Let's see if we can find a way to mask off writes to the color buffer
		// altogether...

		if( FDraw_nCapDepthTest & FDRAW_CAP_DEPTHTEST_NEVER ) {
			// Set the depth test to "never" to mask writes to the color buffer...
			_nColorMaskType = _COLORMASKTYPE_DEPTH;
		} else if( FDraw_nCapAlphaTest & FDRAW_CAP_ALPHATEST_NEVER ) {
			// Set the alpha test to "never" to mask writes to the color buffer...
			_nColorMaskType = _COLORMASKTYPE_ALPHA;
		} else {
			// What a pitiful video card this guy has!
			_nColorMaskType = _COLORMASKTYPE_NONE;
		}
	}

	return TRUE;
}

static void _FixupMapTables( void ) {
	u32 i;

	for( i=0; _MapTableList[i].pMapTable; i++ ) {
		fclib_QSort( _MapTableList[i].pMapTable, _MapTableList[i].nElementCount, sizeof(_MapTable_t), _SortMapTableCallback );

		#if FANG_ENABLE_FASSERT
			// Make sure there are no holes in the table mappings...
			for( u32 j=0; j<_MapTableList[i].nElementCount; j++ ) {
				FASSERT( _MapTableList[i].pMapTable[j].nFangValue == j );
			}
		#endif
	}
}

static int _SortMapTableCallback( const void *pArg1, const void *pArg2 ) {
	_MapTable_t *pMapTable1, *pMapTable2;

	pMapTable1 = (_MapTable_t *)pArg1;
	pMapTable2 = (_MapTable_t *)pArg2;

	if( pMapTable1->nFangValue > pMapTable2->nFangValue ) {
		return +1;
	}
	if( pMapTable1->nFangValue < pMapTable2->nFangValue ) {
		return -1;
	}

	return 0;
}

#include "fworld_coll.h"
#include "flight.h"
#include "floop.h"

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
	_CoronaCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
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

	fdx8draw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
	if (pTex)
	{
		fdx8draw_SetTexture(pTex);
	}
	else
	{
		FDX8_pDev->SetTexture(0, (LPDIRECT3DTEXTURE8)fsh_GetAttenMap());
	}
	//fdx8draw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC_PLUS_DST);
	fdx8draw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC);

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
	CFVec3 quad[4], XAxis, YAxis, ZAxis, Pos, Vec;
	CFColorRGBA Color;
	float fScale;

	ZAxis.x = pCamera->aa[0][2]; ZAxis.y = pCamera->aa[1][2]; ZAxis.z = pCamera->aa[2][2];

	if ( nPhase == (u32)(pLight->m_nRayCastPhase&(_CORONA_VIS_FLAG-1)) )
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
				pLight->m_fFadeDelta = +6.0f;
			}
			else
			{
				pLight->m_nRayCastPhase &= ~_CORONA_VIS_FLAG;
				pLight->m_fFadeDelta = -6.0f;
			}
		}
		else
		{
			pLight->m_nRayCastPhase &= ~_CORONA_VIS_FLAG;
			pLight->m_fFadeDelta = -6.0f;
		}
	}

	pLight->m_fFade += pLight->m_fFadeDelta*FLoop_fPreviousLoopSecs;
	if (pLight->m_fFade < 0.0f) pLight->m_fFade = 0.0f;
	if (pLight->m_fFade > 1.0f) pLight->m_fFade = 1.0f;

	if ( !(pLight->m_nRayCastPhase&_CORONA_VIS_FLAG) && pLight->m_fFade == 0.0f)
	{
		return;
	}

	fScaleDelta = pLight->m_fFade*3.0f;
	if (fScaleDelta > 1.0f) fScaleDelta = 1.0f;

	fScaleDelta *= pLight->m_fCoronaScale;

	fdx8draw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
	if (pLight->m_pCoronaTex)
	{
		fdx8draw_SetTexture(pLight->m_pCoronaTex);
	}
	else
	{
		FDX8_pDev->SetTexture(0, (LPDIRECT3DTEXTURE8)fsh_GetAttenMap());
	}
	fdx8draw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC_PLUS_DST);

	pLight->ComputeColor();

	Color.fRed = pLight->m_ScaledColor.fRed*pLight->m_fFade*0.5f;
	Color.fGreen = pLight->m_ScaledColor.fGreen*pLight->m_fFade*0.5f;
	Color.fBlue = pLight->m_ScaledColor.fBlue*pLight->m_fFade*0.5f;
	Color.fAlpha = pLight->m_ScaledColor.fAlpha;

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
	Color.fRed *= fInvDist;
	Color.fGreen *= fInvDist;
	Color.fBlue *= fInvDist;

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

			Color.fRed *= fDistScale;
			Color.fGreen *= fDistScale;
			Color.fBlue *= fDistScale;
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
		
	Vec.x = -1.0f; Vec.y = -1.0f; Vec.z = 0.0f;
	quad[0].x = Vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x;
	quad[0].y = Vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y;
	quad[0].z = Vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z;

	Vec.x = +1.0f; Vec.y = -1.0f; Vec.z = 0.0f;
	quad[1].x = Vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x; 
	quad[1].y = Vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y; 
	quad[1].z = Vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z; 

	Vec.x = +1.0f; Vec.y = +1.0f; Vec.z = 0.0f;
	quad[2].x = Vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x; 
	quad[2].y = Vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y; 
	quad[2].z = Vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z; 

	Vec.x = -1.0f; Vec.y = +1.0f; Vec.z = 0.0f;
	quad[3].x = Vec.Dot(XAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.x; 
	quad[3].y = Vec.Dot(YAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.y; 
	quad[3].z = Vec.Dot(ZAxis)*fScale + pLight->m_spInfluence_WS.m_Pos.z; 

	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_EnableWriting( FALSE );
	
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &Color);
}
