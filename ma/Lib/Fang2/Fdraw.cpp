//////////////////////////////////////////////////////////////////////////////////////
// fdraw.cpp - Fang drawing utility module.
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
// 03/08/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdraw.h"
#include "ftex.h"
#include "frenderer.h"
#include "fshaders.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _MAX_DEV_CAPSULES	32
#define _MAX_DEV_SPHERES	64
#define _MAX_DEV_TRIANGLES	64
#define _MAX_DEV_LINES		128

#if FANG_PLATFORM_DX
	// DX:
	extern void fdx8draw_Renderer_Open( void );
	extern void fdx8draw_Renderer_Close( void );

	extern BOOL fdx8draw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
	extern BOOL fdx8draw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp );
	extern void fdx8draw_SetCullDir( FDrawCullDir_e nCullDir );
	extern void fdx8draw_EnableClipping( BOOL bEnable );
	extern void fdx8draw_Color_SetFunc( FDrawColorFunc_e nColorFunc );
	extern void fdx8draw_SetTexture( CFTexInst *pTexInst );
	extern void fdx8draw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode );
	extern void fdx8draw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43A );
	extern void fdx8draw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
	extern void fdx8draw_Color_EnableDither( BOOL bEnable );
	extern void fdx8draw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference );
	extern void fdx8draw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp );
	extern void fdx8draw_Depth_EnableWriting( BOOL bEnable );
	extern void fdx8draw_Depth_SetTest( FDrawDepthTest_e nDepthTest );
	extern void fdx8draw_Depth_SetBiasLevel( u32 nNewLevel );
	extern void fdx8draw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
										  FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
										  FDrawStencilOp_e nDepthPassOp );

	#define _IS_IsColorMaskSupported	fdx8draw_IsColorMaskSupported
	#define _IS_IsStencilModeSupported	fdx8draw_IsStencilModeSupported
	#define _IS_SetCullDir				fdx8draw_SetCullDir
	#define _IS_EnableClipping			fdx8draw_EnableClipping
	#define _IS_Color_SetFunc			fdx8draw_Color_SetFunc
	#define _IS_SetTexture				fdx8draw_SetTexture
	#define _IS_SetTexCoordXfmMode		fdx8draw_SetTexCoordXfmMode
	#define _IS_SetTexCoordXfmMtx		fdx8draw_SetTexCoordXfmMtx
	#define _IS_Color_SetMask			fdx8draw_Color_SetMask
	#define _IS_Color_EnableDither		fdx8draw_Color_EnableDither
	#define _IS_Alpha_SetTest			fdx8draw_Alpha_SetTest
	#define _IS_Alpha_SetBlendOp		fdx8draw_Alpha_SetBlendOp
	#define _IS_Depth_EnableWriting		fdx8draw_Depth_EnableWriting
	#define _IS_Depth_SetTest			fdx8draw_Depth_SetTest
	#define _IS_Depth_SetBiasLevel		fdx8draw_Depth_SetBiasLevel
	#define _IS_Stencil_SetMode			fdx8draw_Stencil_SetMode

#elif FANG_PLATFORM_GC
	// GC:
	extern void fgcdraw_Renderer_Open( void );
	extern void fgcdraw_Renderer_Close( void );

	extern BOOL fgcdraw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
	extern BOOL fgcdraw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp );
	extern void fgcdraw_SetCullDir( FDrawCullDir_e nCullDir );
	extern void fgcdraw_EnableClipping( BOOL bEnable );
	extern void fgcdraw_Color_SetFunc( FDrawColorFunc_e nColorFunc );
	extern void fgcdraw_SetTexture( CFTexInst *pTexInst );
	extern void fgcdraw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode );
	extern void fgcdraw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43 );
	extern void fgcdraw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
	extern void fgcdraw_Color_EnableDither( BOOL bEnable );
	extern void fgcdraw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference );
	extern void fgcdraw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp );
	extern void fgcdraw_Depth_EnableWriting( BOOL bEnable );
	extern void fgcdraw_Depth_SetTest( FDrawDepthTest_e nDepthTest );
	extern void fgcdraw_Depth_SetBiasLevel( u32 nNewLevel );
	extern void fgcdraw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
										  FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
										  FDrawStencilOp_e nDepthPassOp );

	#define _IS_IsColorMaskSupported	fgcdraw_IsColorMaskSupported
	#define _IS_IsStencilModeSupported	fgcdraw_IsStencilModeSupported
	#define _IS_SetCullDir				fgcdraw_SetCullDir
	#define _IS_EnableClipping			fgcdraw_EnableClipping
	#define _IS_Color_SetFunc			fgcdraw_Color_SetFunc
	#define _IS_SetTexture				fgcdraw_SetTexture
	#define _IS_SetTexCoordXfmMode		fgcdraw_SetTexCoordXfmMode
	#define _IS_SetTexCoordXfmMtx		fgcdraw_SetTexCoordXfmMtx
	#define _IS_Color_SetMask			fgcdraw_Color_SetMask
	#define _IS_Color_EnableDither		fgcdraw_Color_EnableDither
	#define _IS_Alpha_SetTest			fgcdraw_Alpha_SetTest
	#define _IS_Alpha_SetBlendOp		fgcdraw_Alpha_SetBlendOp
	#define _IS_Depth_EnableWriting		fgcdraw_Depth_EnableWriting
	#define _IS_Depth_SetTest			fgcdraw_Depth_SetTest
	#define _IS_Depth_SetBiasLevel		fgcdraw_Depth_SetBiasLevel
	#define _IS_Stencil_SetMode			fgcdraw_Stencil_SetMode
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	// PS2:
	extern void fps2draw_Renderer_Open( void );
	extern void fps2draw_Renderer_Close( void );

	extern BOOL fps2draw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
	extern BOOL fps2draw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp );
	extern void fps2draw_SetCullDir( FDrawCullDir_e nCullDir );
	extern void fps2draw_EnableClipping( BOOL bEnable );
	extern void fps2draw_Color_SetFunc( FDrawColorFunc_e nColorFunc );
	extern void fps2draw_SetTexture( CFTexInst *pTexInst );
	extern void fps2draw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode );
	extern void fps2draw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43 );
	extern void fps2draw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
	extern void fps2draw_Color_EnableDither( BOOL bEnable );
	extern void fps2draw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference );
	extern void fps2draw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp );
	extern void fps2draw_Depth_EnableWriting( BOOL bEnable );
	extern void fps2draw_Depth_SetTest( FDrawDepthTest_e nDepthTest );
	extern void fps2draw_Depth_SetBiasLevel( u32 nNewLevel );
	extern void fps2draw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
										  FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
										  FDrawStencilOp_e nDepthPassOp );

	#define _IS_IsColorMaskSupported	fps2draw_IsColorMaskSupported
	#define _IS_IsStencilModeSupported	fps2draw_IsStencilModeSupported
	#define _IS_SetCullDir				fps2draw_SetCullDir
	#define _IS_EnableClipping			fps2draw_EnableClipping
	#define _IS_Color_SetFunc			fps2draw_Color_SetFunc
	#define _IS_SetTexture				fps2draw_SetTexture
	#define _IS_SetTexCoordXfmMode		fps2draw_SetTexCoordXfmMode
	#define _IS_SetTexCoordXfmMtx		fps2draw_SetTexCoordXfmMtx
	#define _IS_Color_SetMask			fps2draw_Color_SetMask
	#define _IS_Color_EnableDither		fps2draw_Color_EnableDither
	#define _IS_Alpha_SetTest			fps2draw_Alpha_SetTest
	#define _IS_Alpha_SetBlendOp		fps2draw_Alpha_SetBlendOp
	#define _IS_Depth_EnableWriting		fps2draw_Depth_EnableWriting
	#define _IS_Depth_SetTest			fps2draw_Depth_SetTest
	#define _IS_Depth_SetBiasLevel		fps2draw_Depth_SetBiasLevel
	#define _IS_Stencil_SetMode			fps2draw_Stencil_SetMode
//ARG - <<<<<
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	FDrawCullDir_e nCullDir;
	BOOL bClipEnabled;
	FDrawColorFunc_e nColorFunc;
	CFTexInst TexInst;
	FDrawTexCoordXfmMode_e nTexCoordXfmMode;
	CFMtx43A TexCoordXfmMtx43A;
	BOOL bColorMask_Red;
	BOOL bColorMask_Green;
	BOOL bColorMask_Blue;
	BOOL bDitherEnabled;
	FDrawAlphaTest_e nAlphaTest;
	f32 fAlphaReference;
	FDrawBlendOp_e nBlendOp;
	BOOL bDepthWriteEnable;
	FDrawDepthTest_e nDepthTest;
	u32 nDepthBiasLevel;
	FDrawStencilTest_e nStencil_Test;
	u32 nStencil_Reference;
	u32 nStencil_TestMask;
	u32 nStencil_WriteMask;
	FDrawStencilOp_e nStencil_StencilFailOp;
	FDrawStencilOp_e nStencil_DepthFailOp;
	FDrawStencilOp_e nStencil_DepthPassOp;
} _State_t;


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////////////////

// These are maintained by the implementation-specific fdraw modules:
FDrawCapColorFunc_e	FDraw_nCapColorFunc;	// Bits indicating which color combine functions are supported
FDrawCapAlphaTest_e	FDraw_nCapAlphaTest;	// Bits indicating which alpha tests are supported
FDrawCapBlendOp_e	FDraw_nCapBlendOp;		// Bits indicating which blend operations are supported
FDrawCapDepthTest_e	FDraw_nCapDepthTest;	// Bits indicating which depth tests are supported

BOOL FDraw_nDepthBitDepth;					// Number of depth bits (0=depth buffer not supported)
u32 FDraw_nStencilBitDepth;					// Number of stencil bits (0=stencil buffer not supported)
u32 FDraw_nStencilBitMask;					// Mask with the valid stencil bits set

BOOL FDraw_bCanDisableDepthWrites;			// TRUE=writing to depth buffer can be disabled
u32 FDraw_nMaxDepthBiasValue;				// The maximum depth bias value supported (0=depth biasing not supported)

// Array of temporary vert indices to be used for drawing indexed primitives
u16 *FDraw_paTempVertIndices;
// Maximum amount of indices that can be placed in the FDraw_paTempVertIndices
u16 FDraw_nTempVertIndicesArraySize;

FDrawShaderMap_t FDraw_ColorFuncToShaderMap[FDRAW_COLORFUNC_COUNT] =
{
	FDRAW_COLORFUNC_DECAL_AI,			FSHADERS_DECAL_AI,
	FDRAW_COLORFUNC_DECALTEX_AI,		FSHADERS_DECALTEX_AI,
	FDRAW_COLORFUNC_DECALTEX_AT,		FSHADERS_DECALTEX_AT,
	FDRAW_COLORFUNC_DIFFUSETEX_AI,		FSHADERS_DIFFUSETEX_AI,
	FDRAW_COLORFUNC_DIFFUSETEX_AT,		FSHADERS_DIFFUSETEX_AT,
	FDRAW_COLORFUNC_DIFFUSETEX_AIAT,	FSHADERS_DIFFUSETEX_AIAT,
	FDRAW_COLORFUNC_SPECULARTEX_AT,		FSHADERS_SPECULARTEX_AT,
	FDRAW_COLORFUNC_ADD,				FSHADERS_ADD,
	FDRAW_COLORFUNC_BLEND_AIPLUSAT,		FSHADERS_BLEND_AIPLUSAT,
	FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,	FSHADERS_CT_PLUS_CIAT_AI,	
#if FANG_PLATFORM_GC
	FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY, FSHADERS_INTENSITY,
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY, FSHADERS_INTENSITY,
//ARG - <<<<<
#endif
};



//////////////////////////////////////////////////////////////////////////////////////
// Local Variables
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;
static _State_t _CurrentState;

static FDrawVtx_t _aCircleVtx[FDRAW_MAX_CIRCLE_FACETS + 1];


#if !FANG_PRODUCTION_BUILD
	// Counts
	static s16			_nDevCapsuleCount;
	static s16			_nDevSphereCount;
	static s16			_nDevTriangleCount;
	static s16			_nDevLineCount;

	// Sphere data
	static CFCapsule	*_aCapsules;
	static CFColorRGBA	*_aCapsuleColors;
	static u16			*_aCapsuleTessalationInfo;

	// Sphere data
	static CFSphere		*_aSpheres;
	static CFColorRGBA	*_aSphereColors;
	static u16			*_aSphereTessalationInfo;

	// Triangle Data
	static CFVec3		*_avTrianglePoints;
	static CFColorRGBA	*_aTriangleColors;

	// Line Data
	static CFVec3		*_avLinePoints;
	static CFColorRGBA	*_aLineColors;
#endif


static BOOL _bStateValid_CullDir;
static BOOL _bStateValid_ClipEnable;
static BOOL _bStateValid_ColorFunc;
static BOOL _bStateValid_TexInst;
static BOOL _bStateValid_TexCoordXfmMode;
static BOOL _bStateValid_TexCoordXfmMtx;
static BOOL _bStateValid_ColorMask;
static BOOL _bStateValid_DitherEnable;
static BOOL _bStateValid_AlphaTest;
static BOOL _bStateValid_BlendOp;
static BOOL _bStateValid_DepthWriteEnable;
static BOOL _bStateValid_DepthTest;
static BOOL _bStateValid_DepthBias;
static BOOL _bStateValid_Stencil;



static void _InitGlobalData( void );
static void _InvalidateCurrentState( void );
static void _SetDefaultState( _State_t *pDestState );
static void _ReadCurrentState( _State_t *pDestState );
static void _ApplyState( const _State_t *pNewState );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fdraw_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_InitGlobalData();
	_InvalidateCurrentState();
	_SetDefaultState( &_CurrentState );

	FDraw_nTempVertIndicesArraySize = 256 * 3; // Support for up to 256 triangles in the indexed primitive array
	FDraw_paTempVertIndices = (u16 *)fres_Alloc( sizeof( u16 ) * FDraw_nTempVertIndicesArraySize );
	if ( !FDraw_paTempVertIndices )
	{
		return FALSE;
	}

	#if !FANG_PRODUCTION_BUILD
		// Counts
		_nDevCapsuleCount = 0;
		_nDevSphereCount = 0;
		_nDevTriangleCount = 0;
		_nDevLineCount = 0;

		// Capsule data
		_aCapsules = (CFCapsule *)fres_Alloc( sizeof( CFCapsule ) * _MAX_DEV_CAPSULES );
		_aCapsuleColors = (CFColorRGBA *)fres_Alloc( sizeof( CFColorRGBA ) * _MAX_DEV_CAPSULES );
		_aCapsuleTessalationInfo = (u16 *)fres_Alloc( sizeof( u16 ) * _MAX_DEV_CAPSULES );

		// Sphere data
		_aSpheres = (CFSphere *)fres_Alloc( sizeof( CFSphere ) * _MAX_DEV_SPHERES );
		_aSphereColors = (CFColorRGBA *)fres_Alloc( sizeof( CFColorRGBA ) * _MAX_DEV_SPHERES );
		_aSphereTessalationInfo = (u16 *)fres_Alloc( sizeof( u16 ) * _MAX_DEV_SPHERES );

		// Triangle Data
		_avTrianglePoints = (CFVec3 *)fres_Alloc( sizeof( CFVec3 ) * _MAX_DEV_TRIANGLES * 3 );
		_aTriangleColors = (CFColorRGBA *)fres_Alloc( sizeof( CFColorRGBA ) * _MAX_DEV_TRIANGLES );

		// Line Data
		_avLinePoints = (CFVec3 *)fres_Alloc( sizeof( CFVec3 ) * _MAX_DEV_LINES * 2 );
		_aLineColors = (CFColorRGBA *)fres_Alloc( sizeof( CFColorRGBA ) * _MAX_DEV_LINES * 2 );
	#endif // !FANG_PRODUCTION_BUILD

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fdraw_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}


//
//
//
BOOL fdraw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue ) 
{
	FASSERT( _bModuleInitialized );
	return _IS_IsColorMaskSupported( bMaskOffRed, bMaskOffGreen, bMaskOffBlue );
}


//
//
//
BOOL fdraw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp, 
								  FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp ) 
{
	FASSERT( _bModuleInitialized );
	return _IS_IsStencilModeSupported( nStencilTest, nStencilFailOp, nDepthFailOp, nDepthPassOp );
}


//
//
//
void fdraw_SetCullDir( FDrawCullDir_e nCullDir ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nCullDir>=0 && nCullDir<FDRAW_CULLDIR_COUNT );

	if( !_bStateValid_CullDir || _CurrentState.nCullDir!=nCullDir ) 
	{
		_bStateValid_CullDir = TRUE;
		_CurrentState.nCullDir = nCullDir;
		_IS_SetCullDir( nCullDir );
	}
}


//
//
//
FDrawCullDir_e fdraw_GetCullDir( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_CullDir ) 
	{
		_bStateValid_CullDir = TRUE;
		_CurrentState.nCullDir = FDRAW_DEFAULT_CULLDIR;
		_IS_SetCullDir( _CurrentState.nCullDir );
	}

	return _CurrentState.nCullDir;
}


//
//
//
void fdraw_EnableClipping( BOOL bEnable ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	bEnable = !!bEnable;

	if( !_bStateValid_ClipEnable || _CurrentState.bClipEnabled!=bEnable ) 
	{
		_bStateValid_ClipEnable = TRUE;
		_CurrentState.bClipEnabled = bEnable;
		_IS_EnableClipping( bEnable );
	}
}


//
//
//
BOOL fdraw_IsClippingEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_ClipEnable ) 
	{
		_bStateValid_ClipEnable = TRUE;
		_CurrentState.bClipEnabled = FDRAW_DEFAULT_CLIPENABLE;
		_IS_EnableClipping( _CurrentState.bClipEnabled );
	}

	return _CurrentState.bClipEnabled;
}


//
//
//
void fdraw_Color_SetFunc( FDrawColorFunc_e nColorFunc ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nColorFunc>=0 && nColorFunc<FDRAW_COLORFUNC_COUNT );

	if( !((1<<nColorFunc) & FDraw_nCapColorFunc) ) 
	{
		// Requested value not supported...
		return;
	}

	if( !_bStateValid_ColorFunc || _CurrentState.nColorFunc!=nColorFunc ) 
	{
		_bStateValid_ColorFunc = TRUE;
		_CurrentState.nColorFunc = nColorFunc;
		_IS_Color_SetFunc( nColorFunc );
	}
}


//
//
//
FDrawColorFunc_e fdraw_Color_GetFunc( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_ColorFunc ) 
	{
		_bStateValid_ColorFunc = TRUE;
		_CurrentState.nColorFunc = FDRAW_DEFAULT_COLORFUNC;
		_IS_Color_SetFunc( _CurrentState.nColorFunc );
	}

	return _CurrentState.nColorFunc;
}


//
//
//
void fdraw_SetTexture( CFTexInst *pTexInst ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( pTexInst == NULL ) 
	{
		pTexInst = &CFTexInst::NullTexInst;
	}

	if( _bStateValid_TexInst ) 
	{
		if( _CurrentState.TexInst == *pTexInst ) 
		{
			return;
		}
	}

	// State has changed...

	_bStateValid_TexInst = TRUE;
	_CurrentState.TexInst = *pTexInst;
	_IS_SetTexture( pTexInst );
}


//
//
//
void fdraw_GetTexture( CFTexInst *pDestTexInst ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_TexInst ) 
	{
		_bStateValid_TexInst = TRUE;
		_CurrentState.TexInst = CFTexInst::NullTexInst;
		_IS_SetTexture( &_CurrentState.TexInst );
	}

	*pDestTexInst = _CurrentState.TexInst;
}


//
//
//
void fdraw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nTexCoordXfmMode>=0 && nTexCoordXfmMode<FDRAW_TEXCOORDXFMMODE_COUNT );

	if( !_bStateValid_TexCoordXfmMode || _CurrentState.nTexCoordXfmMode!=nTexCoordXfmMode ) 
	{
		_bStateValid_TexCoordXfmMode = TRUE;
		_CurrentState.nTexCoordXfmMode = nTexCoordXfmMode;
		_IS_SetTexCoordXfmMode( nTexCoordXfmMode );
	}
}


//
//
//
FDrawTexCoordXfmMode_e fdraw_GetTexCoordXfmMode( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_TexCoordXfmMode ) 
	{
		_bStateValid_TexCoordXfmMode = TRUE;
		_CurrentState.nTexCoordXfmMode = FDRAW_DEFAULT_TEXCOORDXFMMODE;
		_IS_SetTexCoordXfmMode( _CurrentState.nTexCoordXfmMode );
	}

	return _CurrentState.nTexCoordXfmMode;
}


//
//
//
void fdraw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43A ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	_bStateValid_TexCoordXfmMtx = TRUE;
	if( pMtx43A ) 
	{
		_CurrentState.TexCoordXfmMtx43A = *pMtx43A;
	} 
	else 
	{
		_CurrentState.TexCoordXfmMtx43A.Identity();
	}
	_IS_SetTexCoordXfmMtx( pMtx43A );
}


//
//
//
void fdraw_GetTexCoordXfmMtx( CFMtx43A *pMtx43A ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_TexCoordXfmMtx ) 
	{
		_bStateValid_TexCoordXfmMtx = TRUE;
		_CurrentState.TexCoordXfmMtx43A.Identity();
		_IS_SetTexCoordXfmMtx( &_CurrentState.TexCoordXfmMtx43A );
	}

	*pMtx43A = _CurrentState.TexCoordXfmMtx43A;
}


//
//
//
void fdraw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	bMaskOffRed = !!bMaskOffRed;
	bMaskOffGreen = !!bMaskOffGreen;
	bMaskOffBlue = !!bMaskOffBlue;

	if( !_bStateValid_ColorMask || _CurrentState.bColorMask_Red!=bMaskOffRed || _CurrentState.bColorMask_Green!=bMaskOffGreen 
		|| _CurrentState.bColorMask_Blue!=bMaskOffBlue ) 
	{
		_bStateValid_ColorMask = TRUE;
		_CurrentState.bColorMask_Red = bMaskOffRed;
		_CurrentState.bColorMask_Green = bMaskOffGreen;
		_CurrentState.bColorMask_Blue = bMaskOffBlue;
		_IS_Color_SetMask( bMaskOffRed, bMaskOffGreen, bMaskOffBlue );
	}
}


//
//
//
void fdraw_Color_GetMask( BOOL *pbMaskOffRed, BOOL *pbMaskOffGreen, BOOL *pbMaskOffBlue ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_ColorMask ) 
	{
		_bStateValid_ColorMask = TRUE;
		_CurrentState.bColorMask_Red = FDRAW_DEFAULT_COLORMASK_RED;
		_CurrentState.bColorMask_Green = FDRAW_DEFAULT_COLORMASK_GREEN;
		_CurrentState.bColorMask_Blue = FDRAW_DEFAULT_COLORMASK_BLUE;
		_IS_Color_SetMask( _CurrentState.bColorMask_Red, _CurrentState.bColorMask_Green, _CurrentState.bColorMask_Blue );
	}

	if( pbMaskOffRed ) 
	{
		*pbMaskOffRed = _CurrentState.bColorMask_Red;
	}
	if( pbMaskOffGreen ) 
	{
		*pbMaskOffGreen = _CurrentState.bColorMask_Green;
	}
	if( pbMaskOffBlue ) 
	{
		*pbMaskOffBlue = _CurrentState.bColorMask_Blue;
	}
}


//
//
//
void fdraw_Color_EnableDither( BOOL bEnable ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	bEnable = !!bEnable;

	if( !_bStateValid_DitherEnable || _CurrentState.bDitherEnabled!=bEnable ) 
	{
		_bStateValid_DitherEnable = TRUE;
		_CurrentState.bDitherEnabled = bEnable;
		_IS_Color_EnableDither( bEnable );
	}
}


//
//
//
BOOL fdraw_Color_IsDitherEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_DitherEnable ) 
	{
		_bStateValid_DitherEnable = TRUE;
		_CurrentState.bDitherEnabled = FDRAW_DEFAULT_DITHERENABLE;
		_IS_Color_EnableDither( _CurrentState.bDitherEnabled );
	}

	return _CurrentState.bDitherEnabled;
}


//
//
//
void fdraw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nAlphaTest>=0 && nAlphaTest<FDRAW_ALPHATEST_COUNT );

	if( !((1<<nAlphaTest) & FDraw_nCapAlphaTest) ) 
	{
		// Requested value not supported...
		nAlphaTest = FDRAW_DEFAULT_ALPHATEST;
		fReference = FDRAW_DEFAULT_ALPHAREF;
	}

	if( !_bStateValid_AlphaTest || _CurrentState.nAlphaTest!=nAlphaTest || _CurrentState.fAlphaReference!=fReference ) 
	{
		_bStateValid_AlphaTest = TRUE;
		_CurrentState.nAlphaTest = nAlphaTest;
		_CurrentState.fAlphaReference = fReference;
		_IS_Alpha_SetTest( nAlphaTest, fReference );
	}
}


//
//
//
FDrawAlphaTest_e fdraw_Alpha_GetTest( float *pfReference ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_AlphaTest ) 
	{
		_bStateValid_AlphaTest = TRUE;
		_CurrentState.nAlphaTest = FDRAW_DEFAULT_ALPHATEST;
		_CurrentState.fAlphaReference = FDRAW_DEFAULT_ALPHAREF;
		_IS_Alpha_SetTest( _CurrentState.nAlphaTest, _CurrentState.fAlphaReference );
	}

	if( pfReference ) 
	{
		*pfReference = _CurrentState.fAlphaReference;
	}

	return _CurrentState.nAlphaTest;
}


//
//
//
void fdraw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nBlendOp>=0 && nBlendOp<FDRAW_BLENDOP_COUNT );

	if( !((1<<nBlendOp) & FDraw_nCapBlendOp) ) 
	{
		// Requested value not supported...
		nBlendOp = FDRAW_DEFAULT_BLENDOP;
	}

	if( !_bStateValid_BlendOp || _CurrentState.nBlendOp!=nBlendOp ) 
	{
		_bStateValid_BlendOp = TRUE;
		_CurrentState.nBlendOp = nBlendOp;
		_IS_Alpha_SetBlendOp( nBlendOp );
	}
}


//
//
//
FDrawBlendOp_e fdraw_Alpha_GetBlendOp( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !_bStateValid_BlendOp ) 
	{
		_bStateValid_BlendOp = TRUE;
		_CurrentState.nBlendOp = FDRAW_DEFAULT_BLENDOP;
		_IS_Alpha_SetBlendOp( _CurrentState.nBlendOp );
	}

	return _CurrentState.nBlendOp;
}

void fdraw_ClearDepth_Cache()
{
	_bStateValid_DepthWriteEnable = TRUE;
	_CurrentState.bDepthWriteEnable = TRUE;
	_IS_Depth_EnableWriting( TRUE );
	
	_bStateValid_DepthTest = TRUE;
	_CurrentState.nDepthTest = FDRAW_DEPTHTEST_CLOSER;
	_IS_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
}

//
//
//
void fdraw_Depth_EnableWriting( BOOL bEnable ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		return;
	}

	if( FDraw_bCanDisableDepthWrites ) 
	{
		bEnable = !!bEnable;
	} 
	else 
	{
		bEnable = TRUE;
	}		

	if( !_bStateValid_DepthWriteEnable || _CurrentState.bDepthWriteEnable!=bEnable ) 
	{
		_bStateValid_DepthWriteEnable = TRUE;
		_CurrentState.bDepthWriteEnable = bEnable;
		_IS_Depth_EnableWriting( bEnable );
	}
}


//
//
//
BOOL fdraw_Depth_IsWritingEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		_bStateValid_DepthWriteEnable = TRUE;
		_CurrentState.bDepthWriteEnable = FALSE;
		return _CurrentState.bDepthWriteEnable;
	}

	if( !_bStateValid_DepthWriteEnable ) 
	{
		_bStateValid_DepthWriteEnable = TRUE;
		_CurrentState.bDepthWriteEnable = FDRAW_DEFAULT_DEPTHWRITEENABLE;
		_IS_Depth_EnableWriting( _CurrentState.bDepthWriteEnable );
	}

	return _CurrentState.bDepthWriteEnable;
}


//
//
//
void fdraw_Depth_SetTest( FDrawDepthTest_e nDepthTest ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nDepthTest>=0 && nDepthTest<FDRAW_DEPTHTEST_COUNT );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		return;
	}

	if( !((1<<nDepthTest) & FDraw_nCapDepthTest) ) 
	{
		// Requested value not supported...
		nDepthTest = FDRAW_DEFAULT_DEPTHTEST;
	}

	if( !_bStateValid_DepthTest || _CurrentState.nDepthTest!=nDepthTest ) 
	{
		_bStateValid_DepthTest = TRUE;
		_CurrentState.nDepthTest = nDepthTest;
		_IS_Depth_SetTest( nDepthTest );
	}
}


//
//
//
FDrawDepthTest_e fdraw_Depth_GetTest( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		_bStateValid_DepthTest = TRUE;
		_CurrentState.nDepthTest = FDRAW_DEFAULT_DEPTHTEST;
		return _CurrentState.nDepthTest;
	}

	if( !_bStateValid_DepthTest ) 
	{
		_bStateValid_DepthTest = TRUE;
		_CurrentState.nDepthTest = FDRAW_DEFAULT_DEPTHTEST;
		_IS_Depth_SetTest( _CurrentState.nDepthTest );
	}

	return _CurrentState.nDepthTest;
}


//
//
//
void fdraw_Depth_SetBiasLevel( u32 nNewLevel ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		return;
	}

	if( !_bStateValid_DepthBias || _CurrentState.nDepthBiasLevel!=nNewLevel ) 
	{
		_bStateValid_DepthBias = TRUE;
		FMATH_CLAMPMAX( nNewLevel, FDraw_nMaxDepthBiasValue );
		_CurrentState.nDepthBiasLevel = nNewLevel;
		_IS_Depth_SetBiasLevel( nNewLevel );
	}
}


//
//
//
u32 fdraw_Depth_GetBiasLevel( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		_bStateValid_DepthBias = TRUE;
		_CurrentState.nDepthBiasLevel = FDRAW_DEFAULT_DEPTHBIAS;
		return _CurrentState.nDepthBiasLevel;
	}

	if( !_bStateValid_DepthBias ) 
	{
		_bStateValid_DepthBias = TRUE;
		_CurrentState.nDepthBiasLevel = FDRAW_DEFAULT_DEPTHBIAS;
		_IS_Depth_SetBiasLevel( _CurrentState.nDepthBiasLevel );
	}

	return _CurrentState.nDepthBiasLevel;
}


//
//
//
void fdraw_Depth_DeltaBiasLevel( s32 nDeltaLevel ) 
{
	s32 nNewBiasLevel;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nDepthBitDepth == 0 ) 
	{
		// Depth buffer not supported...
		return;
	}

	nNewBiasLevel = (s32)fdraw_Depth_GetBiasLevel();

	nNewBiasLevel += nDeltaLevel;
	FMATH_CLAMP( nNewBiasLevel, 0, (s32)FDraw_nMaxDepthBiasValue );

	fdraw_Depth_SetBiasLevel( (u32)nNewBiasLevel );
}


//
//
//
void fdraw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
							FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
							FDrawStencilOp_e nDepthPassOp ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	FASSERT( nStencilTest>=0 && nStencilTest<FDRAW_STENCILTEST_COUNT );
	FASSERT( nStencilFailOp>=0 && nStencilFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthFailOp>=0 && nDepthFailOp<FDRAW_STENCILOP_COUNT );
	FASSERT( nDepthPassOp>=0 && nDepthPassOp<FDRAW_STENCILOP_COUNT );

	if( FDraw_nStencilBitDepth == 0 ) 
	{
		// Stencil not supported...
		return;
	}

	if( !_bStateValid_Stencil ||
		_CurrentState.nStencil_Test != nStencilTest ||
		_CurrentState.nStencil_StencilFailOp != nStencilFailOp ||
		_CurrentState.nStencil_DepthFailOp != nDepthFailOp ||
		_CurrentState.nStencil_DepthPassOp != nDepthPassOp ||
		_CurrentState.nStencil_Reference != nReference ||
		_CurrentState.nStencil_TestMask != nTestMask ||
		_CurrentState.nStencil_WriteMask != nWriteMask ) 
	{
		_bStateValid_Stencil = TRUE;
		_CurrentState.nStencil_Test = nStencilTest;
		_CurrentState.nStencil_StencilFailOp = nStencilFailOp;
		_CurrentState.nStencil_DepthFailOp = nDepthFailOp;
		_CurrentState.nStencil_DepthPassOp = nDepthPassOp;
		_CurrentState.nStencil_Reference = nReference;
		_CurrentState.nStencil_TestMask = nTestMask;
		_CurrentState.nStencil_WriteMask = nWriteMask;

		_IS_Stencil_SetMode( nStencilTest, nReference, nTestMask, nWriteMask, nStencilFailOp, nDepthFailOp, nDepthPassOp );
	}
}


//
//
//
void fdraw_Stencil_GetMode( FDrawStencilTest_e *pnStencilTest, u32 *pnReference, u32 *pnTestMask,
							u32 *pnWriteMask, FDrawStencilOp_e *pnStencilFailOp,
							FDrawStencilOp_e *pnDepthFailOp, FDrawStencilOp_e *pnDepthPassOp ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( FDraw_nStencilBitDepth==0 || !_bStateValid_Stencil ) 
	{
		_bStateValid_Stencil = TRUE;
		_CurrentState.nStencil_Test = FDRAW_DEFAULT_STENCIL_TEST;
		_CurrentState.nStencil_Reference = FDRAW_DEFAULT_STENCIL_REF;
		_CurrentState.nStencil_TestMask = FDRAW_DEFAULT_STENCIL_TESTMASK;
		_CurrentState.nStencil_WriteMask = FDRAW_DEFAULT_STENCIL_WRITEMASK;
		_CurrentState.nStencil_StencilFailOp = FDRAW_DEFAULT_STENCIL_STENCILFAILOP;
		_CurrentState.nStencil_DepthFailOp = FDRAW_DEFAULT_STENCIL_DEPTHFAILOP;
		_CurrentState.nStencil_DepthPassOp = FDRAW_DEFAULT_STENCIL_DEPTHPASSOP;

		if( FDraw_nStencilBitDepth ) 
		{
			_IS_Stencil_SetMode( _CurrentState.nStencil_Test, _CurrentState.nStencil_Reference, _CurrentState.nStencil_TestMask, _CurrentState.nStencil_WriteMask, _CurrentState.nStencil_StencilFailOp, _CurrentState.nStencil_DepthFailOp, _CurrentState.nStencil_DepthPassOp );
		}
	}

	if( pnStencilTest ) 
	{
		*pnStencilTest = _CurrentState.nStencil_Test;
	}
	if( pnReference ) 
	{
		*pnReference = _CurrentState.nStencil_Reference;
	}
	if( pnTestMask ) 
	{
		*pnTestMask = _CurrentState.nStencil_TestMask;
	}
	if( pnWriteMask ) 
	{
		*pnWriteMask = _CurrentState.nStencil_WriteMask;
	}
	if( pnStencilFailOp ) 
	{
		*pnStencilFailOp = _CurrentState.nStencil_StencilFailOp;
	}
	if( pnDepthFailOp ) 
	{
		*pnDepthFailOp = _CurrentState.nStencil_DepthFailOp;
	}
	if( pnDepthPassOp ) 
	{
		*pnDepthPassOp = _CurrentState.nStencil_DepthPassOp;
	}
}


//
//
//
void fdraw_Point( const FDrawVtx_t *pVtx1 ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	fdraw_PrimList( FDRAW_PRIMTYPE_POINTS, pVtx1, 1 );
}


//
//
//
void fdraw_Line( const FDrawVtx_t *pVtx1, const FDrawVtx_t *pVtx2 ) 
{
	FDrawVtx_t aVtx[2];

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	aVtx[0] = *pVtx1;
	aVtx[1] = *pVtx2;

	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, aVtx, 2 );
}


//
//
//
void fdraw_Triangle( const FDrawVtx_t *pVtx1, const FDrawVtx_t *pVtx2, const FDrawVtx_t *pVtx3 ) 
{
	FDrawVtx_t aVtx[3];

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	aVtx[0] = *pVtx1;
	aVtx[1] = *pVtx2;
	aVtx[2] = *pVtx3;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, aVtx, 3 );
}


//
//
//
void fdraw_FacetedCircle( const CFVec3 *pUnitNormal_MS, const CFVec3 *pCenter_MS, f32 fRadius_MS, CFColorRGBA *pVtxColorRGBA, u32 nVtxCountPerQuarter ) 
{
	u32 nVtxNum, nVtxCount;
	f32 fSin, fCos, fRadians, fDeltaRadians;
	FDrawVtx_t *pVtx;
	CFQuatA Quat;
	CFMtx43A Mtx43;
	CFVec3A PointXZ;
	CFVec3A VectorToRotateAbout;
	CFVec3A UnitVectorToRotateAbout;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( nVtxCountPerQuarter == 0 ) 
	{
		return;
	}

	// Compute the total number of vertices...
	nVtxCount = nVtxCountPerQuarter << 2;
	FMATH_CLAMPMAX( nVtxCount, FDRAW_MAX_CIRCLE_FACETS );

	UnitVectorToRotateAbout.Set( *pUnitNormal_MS );
	Mtx43.UnitMtxFromUnitVec( &UnitVectorToRotateAbout );

/*
	// Compute our transformation matrix...
	if( pUnitNormal_MS->y >= (1.0f-FMATH_POS_EPSILON) ) {
		// pUnitNormal_MS is nearly pointing straight up...
		Quat.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	} else if( pUnitNormal_MS->y <= (-1.0f+FMATH_POS_EPSILON) ) {
		// pUnitNormal_MS is nearly pointing straight down...
		Quat.Set( 1.0f, 0.0f, 0.0f, 0.0f );
	} else {
		f32 fUpToNormalRadians;

		VectorToRotateAbout.Set( pUnitNormal_MS->z, 0.0f, -pUnitNormal_MS->x );
		fSin = UnitVectorToRotateAbout.UnitAndMag( VectorToRotateAbout );
		fCos = pUnitNormal_MS->y;
		fUpToNormalRadians = fmath_Atan( fSin, fCos );
		Quat.BuildQuat( UnitVectorToRotateAbout, fUpToNormalRadians );
	}
	Quat.BuildMtx( Mtx43.m33 );
*/
	Mtx43.m_vPos = *pCenter_MS;

	// Build the vertices...
	fDeltaRadians = fmath_Div( FMATH_2PI, (f32)nVtxCount );

	for( nVtxNum=0, fRadians=0.0f, pVtx=_aCircleVtx; nVtxNum<nVtxCount; nVtxNum++, fRadians+=fDeltaRadians, pVtx++ ) 
	{
		fmath_SinCos( fRadians, &fSin, &fCos );

		PointXZ.x = fSin * fRadius_MS;
		PointXZ.y = fCos * fRadius_MS;
		PointXZ.z = 0.0f;

		Mtx43.MulPoint( PointXZ );
		pVtx->Pos_MS = PointXZ.v3;
		pVtx->ColorRGBA = *pVtxColorRGBA;
		pVtx->ST.Zero();
	}

	// Last vertex is the same as the first...
	*pVtx = _aCircleVtx[0];

	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, _aCircleVtx, nVtxCount+1 );
}


//
//
//
void fdraw_FacetedCylinder( const CFVec3 *pBottomCenter_MS, const CFVec3 *pUnitNormal_MS, f32 fRadius_MS, 
						   f32 fHeight_MS, CFColorRGBA *pVtxColorRGBA, u32 nVtxCountPerQuarter ) 
{
	u32 nVtxNum, nVtxCount;
	f32 fSin, fCos, fRadians, fDeltaRadians;
	FDrawVtx_t *pVtx;
	CFQuatA Quat;
	CFMtx43 Mtx43;
	CFVec3 PointXZ;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( nVtxCountPerQuarter == 0 || fHeight_MS==0.0f || fRadius_MS == 0.0f) 
	{
		return;
	}

	// Compute the total number of vertices...
	nVtxCount = nVtxCountPerQuarter << 2;
	FMATH_CLAMPMAX( nVtxCount, FDRAW_MAX_CIRCLE_FACETS );

	// Compute our transformation matrix...
	if( pUnitNormal_MS->y >= 0.99f) 
	{
		// pUnitNormal_MS is nearly pointing straight up...
		Quat.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	} 
	else if( pUnitNormal_MS->y <= -0.99f ) 
	{
		// pUnitNormal_MS is nearly pointing straight down...
		Quat.Set( 1.0f, 0.0f, 0.0f, 0.0f );
	} 
	else 
	{
		CFVec3A VectorToRotateAbout, UnitVectorToRotateAbout;
		f32 fUpToNormalRadians;

		VectorToRotateAbout.Set( pUnitNormal_MS->z, 0.0f, -pUnitNormal_MS->x );
		fSin = UnitVectorToRotateAbout.UnitAndMag( VectorToRotateAbout );
		fCos = pUnitNormal_MS->y;
		fUpToNormalRadians = fmath_Atan( fSin, fCos );
		Quat.BuildQuat( UnitVectorToRotateAbout, fUpToNormalRadians );
	}
	Quat.BuildMtx( Mtx43.m33 );
	Mtx43.m_vPos = *pBottomCenter_MS;

	////////////////////////////////////////////////
	// Build the vertices (for the bottom circle)...
	PointXZ.y = 0.0f;
	fDeltaRadians = fmath_Div( FMATH_2PI, (f32)nVtxCount );

	for( nVtxNum=0, fRadians=0.0f, pVtx=_aCircleVtx; nVtxNum<nVtxCount; nVtxNum++, fRadians+=fDeltaRadians, pVtx++ ) 
	{
		fmath_SinCos( fRadians, &fSin, &fCos );

		PointXZ.x = fSin * fRadius_MS;
		PointXZ.z = fCos * fRadius_MS;

		pVtx->Pos_MS = Mtx43.MultPoint( PointXZ );
		pVtx->ColorRGBA = *pVtxColorRGBA;
		pVtx->ST.Zero();
	}

	// Last vertex is the same as the first...
	*pVtx = _aCircleVtx[0];

	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, _aCircleVtx, nVtxCount+1 );
	//
	/////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////
	// draw the top circle
	CFVec3 DirWS, PosWS;
	DirWS = ( *pUnitNormal_MS * fHeight_MS );
		
	for( nVtxNum=0, pVtx=_aCircleVtx; nVtxNum < nVtxCount; nVtxNum++, pVtx++ ) 
	{
		pVtx->Pos_MS += DirWS;		
	}

	// Last vertex is the same as the first...
	*pVtx = _aCircleVtx[0];

	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, _aCircleVtx, nVtxCount+1 );
	//
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// draw lines between each circle
	for( nVtxNum=0, pVtx=_aCircleVtx; nVtxNum < nVtxCount; nVtxNum++, pVtx++ ) 
	{
		PosWS = pVtx->Pos_MS - DirWS;
		fdraw_SolidLine( &pVtx->Pos_MS, &PosWS, pVtxColorRGBA );
	}
	//
	/////////////////////////////////////////////////////////////
}


//
//
//
void _FacetedSemiCircle( const CFVec3A *pUnitNormal_MS, const CFVec3A *pUnitHemiDir_MS, const CFVec3A *pCenter_MS, 
						f32 fRadius_MS, CFColorRGBA *pVtxColorRGBA, u32 nVtxCountPerQuarter ) 
{
	u32 nVtxNum, nVtxCount;
	f32 fSin, fCos, fRadians, fDeltaRadians;
	FDrawVtx_t *pVtx;
	CFQuatA Quat;
	CFMtx43A Mtx43;
	CFVec3A PointXZ;
	CFVec3A VectorToRotateAbout;
	CFVec3A UnitVectorToRotateAbout;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( nVtxCountPerQuarter == 0 ) 
	{
		return;
	}

	// Compute the total number of vertices...
	nVtxCount = nVtxCountPerQuarter << 2;
	FMATH_CLAMPMAX( nVtxCount, FDRAW_MAX_CIRCLE_FACETS );

	UnitVectorToRotateAbout.Set( *pUnitNormal_MS );
	Mtx43.m_vUp.Cross( *pUnitHemiDir_MS, *pUnitNormal_MS );
	Mtx43.m_vRight.Set( *pUnitHemiDir_MS );
	Mtx43.m_vFront.Set( *pUnitNormal_MS );
	Mtx43.m_vPos.Set( *pCenter_MS );

	// Build the vertices...
	fDeltaRadians = fmath_Div( FMATH_2PI, (f32)nVtxCount );

	nVtxCount = 0;
	pVtx = _aCircleVtx;
	fRadians = 0.0f; 
	for( nVtxNum = 0; nVtxNum<(nVtxCountPerQuarter << 1) + 1; nVtxNum++, fRadians += fDeltaRadians ) 
	{
		fmath_SinCos( fRadians, &fSin, &fCos );

		PointXZ.Set( fSin * fRadius_MS, fCos * fRadius_MS, 0.0f );

		Mtx43.MulPoint( PointXZ );
		pVtx->Pos_MS = PointXZ.v3;

		pVtx->ColorRGBA = *pVtxColorRGBA;
		pVtx->ST.Zero();
		nVtxCount++;
		pVtx++;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, _aCircleVtx, nVtxCount );
}


//
//
//
void fdraw_FacetedWireCapsule( const CFCapsule *pCapsule, u32 nSliceCountPerHalf, u32 nRingCountPerQuarter, CFColorRGBA *pVtxColorRGBA, u32 nVtxCountPerQuarter ) 
{
	u32 i;
	f32 fDelta, fY, fY2, fSliceRadius, fRadius2, fAngle;
	CFVec3A Center, RingAxis;
	CFVec3A vCenterlineNorm;
	CFMtx43A mtx;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	// Calculate the centerline
	vCenterlineNorm.Sub( pCapsule->m_vPoint1, pCapsule->m_vPoint2 );
	f32 fCenterDist = vCenterlineNorm.MagSq();
	if ( fCenterDist < 0.00001f )
	{
		return;
	}
	fCenterDist = fmath_Sqrt( fCenterDist );
	vCenterlineNorm.Mul( fmath_Inv( fCenterDist ) );

	mtx.UnitMtxFromUnitVec( &vCenterlineNorm );

	// Draw vertical slices for point 1...
	fDelta = 0.98f * fmath_Div( pCapsule->m_fRadius, (f32)nSliceCountPerHalf );
	fRadius2 = pCapsule->m_fRadius * pCapsule->m_fRadius;

	// Draw the cylinder portion
	f32 fFeetPerSlice = fmath_Div( pCapsule->m_fRadius, (f32)nSliceCountPerHalf );
	u32 nSlices = (u32)fmath_Div( fCenterDist, fFeetPerSlice );
	FMATH_CLAMPMIN( nSlices, 1 );
	fFeetPerSlice = fmath_Div( fCenterDist, (f32)nSlices );
	for( i = 0; i < nSlices + 1; i++ ) 
	{
		Center.Mul( vCenterlineNorm, (f32)i * fFeetPerSlice ).Add( pCapsule->m_vPoint2 );
		fdraw_FacetedCircle( &vCenterlineNorm.v3, &Center.v3, pCapsule->m_fRadius, pVtxColorRGBA, nVtxCountPerQuarter );
	}

	// draw lines between each circle
	FDrawVtx_t *pVtx;
	for( i=0, pVtx=_aCircleVtx; i < nVtxCountPerQuarter<<2; i++, pVtx++ ) {
		Center.v3 = vCenterlineNorm.v3 * -fCenterDist + pVtx->Pos_MS;
		fdraw_SolidLine( &pVtx->Pos_MS, &Center.v3, pVtxColorRGBA );
	}

	for ( i = 0, fY = fDelta; i < nSliceCountPerHalf; i++, fY += fDelta ) 
	{
		fY2 = fY * fY;
		FMATH_CLAMPMAX( fY2, fRadius2 );
		fSliceRadius = fmath_Sqrt( fRadius2 - fY2 );

		Center.Mul( vCenterlineNorm, fY ).Add( pCapsule->m_vPoint1 );
		fdraw_FacetedCircle( &vCenterlineNorm.v3, &Center.v3, fSliceRadius, pVtxColorRGBA, nVtxCountPerQuarter );

		Center.Mul( vCenterlineNorm, -fY ).Add( pCapsule->m_vPoint2 );
		fdraw_FacetedCircle( &vCenterlineNorm.v3, &Center.v3, fSliceRadius, pVtxColorRGBA, nVtxCountPerQuarter );
	}

	// Draw rings...
	fDelta = fmath_Div( FMATH_HALF_PI, (f32)nRingCountPerQuarter );

	CFVec3A vNegNorm;
	vNegNorm.ReceiveNegative( vCenterlineNorm ); 
	for ( i = 0, fAngle = 0.0f; i < nRingCountPerQuarter << 1; i++, fAngle += fDelta ) 
	{
		RingAxis.z = 0.0f;
		fmath_SinCos( fAngle, &RingAxis.x, &RingAxis.y );

		mtx.MulDir( RingAxis ); 
		_FacetedSemiCircle( &RingAxis, &vNegNorm, &pCapsule->m_vPoint2, pCapsule->m_fRadius, pVtxColorRGBA, nVtxCountPerQuarter );
		_FacetedSemiCircle( &RingAxis, &vCenterlineNorm, &pCapsule->m_vPoint1, pCapsule->m_fRadius, pVtxColorRGBA, nVtxCountPerQuarter );
	}
}


//
//
//
void fdraw_FacetedWireSphere( const CFVec3 *pCenter_MS, f32 fRadius_MS, u32 nSliceCountPerHalf, u32 nRingCountPerQuarter, 
							 CFColorRGBA *pVtxColorRGBA, u32 nVtxCountPerQuarter ) 
{
	u32 i;
	f32 fDelta, fY, fY2, fSliceRadius, fRadius2, fAngle, fOldX;
	CFVec3 Center, RingAxis;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	// Draw vertical slices...
	fDelta = 0.98f * fmath_Div( fRadius_MS, (f32)nSliceCountPerHalf );
	Center.x = pCenter_MS->x;
	Center.z = pCenter_MS->z;
	fRadius2 = fRadius_MS * fRadius_MS;

	fdraw_FacetedCircle( &CFVec3::m_UnitAxisY, pCenter_MS, fRadius_MS, pVtxColorRGBA, nVtxCountPerQuarter );

	for( i=0, fY=fDelta; i<nSliceCountPerHalf; i++, fY+=fDelta ) 
	{
		fY2 = fY * fY;
		FMATH_CLAMPMAX( fY2, fRadius2 );
		fSliceRadius = fmath_Sqrt( fRadius2 - fY2 );

		Center.y = pCenter_MS->y + fY;
		fdraw_FacetedCircle( &CFVec3::m_UnitAxisY, &Center, fSliceRadius, pVtxColorRGBA, nVtxCountPerQuarter );

		Center.y = pCenter_MS->y - fY;
		fdraw_FacetedCircle( &CFVec3::m_UnitAxisY, &Center, fSliceRadius, pVtxColorRGBA, nVtxCountPerQuarter );
	}

	// Draw rings...
	fDelta = fmath_Div( FMATH_HALF_PI, (f32)nRingCountPerQuarter );
	RingAxis.y = 0.0f;

	for( i=0, fAngle=0.0f; i<nRingCountPerQuarter; i++, fAngle+=fDelta ) 
	{
		fmath_SinCos( fAngle, &RingAxis.x, &RingAxis.z );
		fdraw_FacetedCircle( &RingAxis, pCenter_MS, fRadius_MS, pVtxColorRGBA, nVtxCountPerQuarter );

		fOldX = RingAxis.x;
		RingAxis.x = RingAxis.z;
		RingAxis.z = -fOldX;
		fdraw_FacetedCircle( &RingAxis, pCenter_MS, fRadius_MS, pVtxColorRGBA, nVtxCountPerQuarter );
	}
}


//
//
//
void fdraw_FacetedWireSphere( const CFVec3 *pCenter_MS, f32 fRadius_MS, CFColorRGBA *pVtxColorRGBA, u32 nVtxCountPerQuarter ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	fdraw_FacetedWireSphere( pCenter_MS, fRadius_MS, 2, 2, pVtxColorRGBA, nVtxCountPerQuarter );
}


//
// RGB = xyz
//
void fdraw_ModelSpaceAxis( f32 fScale ) 
{
	FDrawVtx_t V1, V2;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.Pos_MS.Zero();

	V1.ColorRGBA.OpaqueRed();
	V2.Pos_MS.Set( fScale, 0.0f, 0.0f );
	V2.ColorRGBA.OpaqueRed();
	fdraw_Line( &V1, &V2 );

	V1.ColorRGBA.OpaqueGreen();
	V2.Pos_MS.Set( 0.0f, fScale, 0.0f );
	V2.ColorRGBA.OpaqueGreen();
	fdraw_Line( &V1, &V2 );

	V1.ColorRGBA.OpaqueBlue();
	V2.Pos_MS.Set( 0.0f, 0.0f, fScale );
	V2.ColorRGBA.OpaqueBlue();
	fdraw_Line( &V1, &V2 );
}


//
//
//
void fdraw_SolidPoint( const CFVec3 *pPos_MS, const CFColorRGBA *pColor ) 
{
	FDrawVtx_t V1;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.ColorRGBA = *pColor;
	V1.Pos_MS = *pPos_MS;

	fdraw_Point( &V1 );
}


//
//
//
void fdraw_SolidLine( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFColorRGBA *pColor ) 
{
	FDrawVtx_t V1, V2;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.ColorRGBA = *pColor;
	V1.Pos_MS = *pPos1_MS;
	V2.ColorRGBA = *pColor;
	V2.Pos_MS = *pPos2_MS;

	fdraw_Line( &V1, &V2 );
}


//
//
//
void fdraw_SolidLine( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFColorRGBA *pColor1, const CFColorRGBA *pColor2 ) 
{
	FDrawVtx_t V1, V2;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.ColorRGBA = *pColor1;
	V1.Pos_MS = *pPos1_MS;
	V2.ColorRGBA = *pColor2;
	V2.Pos_MS = *pPos2_MS;

	fdraw_Line( &V1, &V2 );
}


//
//
//
void fdraw_SolidTriangle( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFVec3 *pPos3_MS, const CFColorRGBA *pColor )
{
	FDrawVtx_t V1, V2, V3;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.ColorRGBA = *pColor;
	V1.Pos_MS = *pPos1_MS;
	V2.ColorRGBA = *pColor;
	V2.Pos_MS = *pPos2_MS;
	V3.ColorRGBA = *pColor;
	V3.Pos_MS = *pPos3_MS;

	fdraw_Triangle( &V1, &V2, &V3 );
}


//
// Pos1, 2, 3, and 4 are in clockwise order.
//
void fdraw_SolidQuad( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFVec3 *pPos3_MS, const CFVec3 *pPos4_MS, const CFColorRGBA *pColor ) 
{
	FDrawVtx_t V1, V2, V3;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.ColorRGBA = *pColor;
	V1.Pos_MS = *pPos1_MS;
	V2.ColorRGBA = *pColor;
	V2.Pos_MS = *pPos2_MS;
	V3.ColorRGBA = *pColor;
	V3.Pos_MS = *pPos3_MS;

	fdraw_Triangle( &V1, &V2, &V3 );

	V2.ColorRGBA = *pColor;
	V2.Pos_MS = *pPos3_MS;
	V3.ColorRGBA = *pColor;
	V3.Pos_MS = *pPos4_MS;

	fdraw_Triangle( &V1, &V2, &V3 );
}


//
//
//
void fdraw_TexQuad( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFVec3 *pPos3_MS, const CFVec3 *pPos4_MS, const CFColorRGBA *pColor ) 
{
	FDrawVtx_t V1, V2, V3;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	V1.ColorRGBA = *pColor;
	V1.Pos_MS = *pPos1_MS;
	V1.ST.x = 0; V1.ST.y = 1;
	V2.ColorRGBA = *pColor;
	V2.Pos_MS = *pPos2_MS;
	V2.ST.x = 1; V2.ST.y = 1;
	V3.ColorRGBA = *pColor;
	V3.Pos_MS = *pPos3_MS;
	V3.ST.x = 1; V3.ST.y = 0;

	fdraw_Triangle( &V1, &V2, &V3 );

	V2.ColorRGBA = *pColor;
	V2.Pos_MS = *pPos3_MS;
	V2.ST.x = 1; V2.ST.y = 0;
	V3.ColorRGBA = *pColor;
	V3.Pos_MS = *pPos4_MS;
	V3.ST.x = 0; V3.ST.y = 0;

	fdraw_Triangle( &V1, &V2, &V3 );
}


//
//
//
void fdraw_Renderer_Open( void ) 
{
	FASSERT( _bModuleInitialized );

	_InvalidateCurrentState();

	#if FANG_PLATFORM_PS2
		fps2draw_Renderer_Open();
	#elif FANG_PLATFORM_DX
		fdx8draw_Renderer_Open();
	#elif FANG_PLATFORM_GC
		fgcdraw_Renderer_Open();
	#else
		#error <Fang Error: No valid platform defined.>
	#endif
}


//
//
//
void fdraw_Renderer_Close( void ) 
{
	FASSERT( _bModuleInitialized );

	#if FANG_PLATFORM_PS2
		fps2draw_Renderer_Close();
	#elif FANG_PLATFORM_DX
		fdx8draw_Renderer_Close();
	#elif FANG_PLATFORM_GC
		fgcdraw_Renderer_Close();
	#else
		#error <Fang Error: No valid platform defined.>
	#endif
}


//
//
//
u32 fdraw_Renderer_GetStateSize( void ) 
{
	FASSERT( _bModuleInitialized );
	return sizeof(_State_t);
}


//
//
//
void fdraw_Renderer_GetState( void *pDestState ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	_ReadCurrentState( (_State_t *)pDestState );
}


//
//
//
void fdraw_Renderer_SetState( const void *pState ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );
	_ApplyState( (const _State_t *)pState );
}


//
//
//
void fdraw_Renderer_SetDefaultState( void ) 
{
	_State_t State;

	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

//ARG - >>>>>
//ARG - BUG local being used as global ???
	#if !FANG_PLATFORM_PS2
//ARG - <<<<<
	_SetDefaultState( &State );
	_ApplyState( &State );
	#endif	//ARG
}


//
//
//
static void _InitGlobalData( void ) 
{
	FDraw_nCapColorFunc = FDRAW_CAP_COLORFUNC_DECAL_AI;
	FDraw_nCapAlphaTest = FDRAW_CAP_ALPHATEST_ALWAYS;
	FDraw_nCapBlendOp = FDRAW_CAP_BLENDOP_SRC;
	FDraw_nCapDepthTest = FDRAW_CAP_DEPTHTEST_ALWAYS;

	FDraw_nDepthBitDepth = 0;
	FDraw_nStencilBitDepth = 0;
	FDraw_nStencilBitMask = 0;

	FDraw_bCanDisableDepthWrites = FALSE;
	FDraw_nMaxDepthBiasValue = 0;
}


//
//
//
static void _InvalidateCurrentState( void ) 
{
	_bStateValid_CullDir = FALSE;
	_bStateValid_ClipEnable = FALSE;
	_bStateValid_ColorFunc = FALSE;
	_bStateValid_TexInst = FALSE;
	_bStateValid_TexCoordXfmMode = FALSE;
	_bStateValid_TexCoordXfmMtx = FALSE;
	_bStateValid_ColorMask = FALSE;
	_bStateValid_DitherEnable = FALSE;
	_bStateValid_AlphaTest = FALSE;
	_bStateValid_BlendOp = FALSE;
	_bStateValid_DepthWriteEnable = FALSE;
	_bStateValid_DepthTest = FALSE;
	_bStateValid_DepthBias = FALSE;
	_bStateValid_Stencil = FALSE;
}


//
//
//
static void _SetDefaultState( _State_t *pDestState ) 
{
	pDestState->nCullDir = FDRAW_DEFAULT_CULLDIR;
	pDestState->bClipEnabled = FDRAW_DEFAULT_CLIPENABLE;
	pDestState->nColorFunc = FDRAW_DEFAULT_COLORFUNC;
	pDestState->TexInst = CFTexInst::NullTexInst;
	pDestState->nTexCoordXfmMode = FDRAW_DEFAULT_TEXCOORDXFMMODE;
	pDestState->TexCoordXfmMtx43A.Identity();
	pDestState->bColorMask_Red = FDRAW_DEFAULT_COLORMASK_RED;
	pDestState->bColorMask_Green = FDRAW_DEFAULT_COLORMASK_GREEN;
	pDestState->bColorMask_Blue = FDRAW_DEFAULT_COLORMASK_BLUE;
	pDestState->bDitherEnabled = FDRAW_DEFAULT_DITHERENABLE;
	pDestState->nAlphaTest = FDRAW_DEFAULT_ALPHATEST;
	pDestState->fAlphaReference = FDRAW_DEFAULT_ALPHAREF;
	pDestState->nBlendOp = FDRAW_DEFAULT_BLENDOP;
	pDestState->bDepthWriteEnable = FDRAW_DEFAULT_DEPTHWRITEENABLE;
	pDestState->nDepthTest = FDRAW_DEFAULT_DEPTHTEST;
	pDestState->nDepthBiasLevel = FDRAW_DEFAULT_DEPTHBIAS;
	pDestState->nStencil_Test = FDRAW_DEFAULT_STENCIL_TEST;
	pDestState->nStencil_Reference = FDRAW_DEFAULT_STENCIL_REF;
	pDestState->nStencil_TestMask = FDRAW_DEFAULT_STENCIL_TESTMASK;
	pDestState->nStencil_WriteMask = FDRAW_DEFAULT_STENCIL_WRITEMASK;
	pDestState->nStencil_StencilFailOp = FDRAW_DEFAULT_STENCIL_STENCILFAILOP;
	pDestState->nStencil_DepthFailOp = FDRAW_DEFAULT_STENCIL_DEPTHFAILOP;
	pDestState->nStencil_DepthPassOp = FDRAW_DEFAULT_STENCIL_DEPTHPASSOP;
}


//
//
//
static void _ReadCurrentState( _State_t *pDestState ) 
{
	pDestState->nCullDir = fdraw_GetCullDir();
	pDestState->bClipEnabled = fdraw_IsClippingEnabled();
	pDestState->nColorFunc = fdraw_Color_GetFunc();
	fdraw_GetTexture( &pDestState->TexInst );
	pDestState->nTexCoordXfmMode = fdraw_GetTexCoordXfmMode();
	fdraw_GetTexCoordXfmMtx( &pDestState->TexCoordXfmMtx43A );
	fdraw_Color_GetMask( &pDestState->bColorMask_Red, &pDestState->bColorMask_Green, &pDestState->bColorMask_Blue );
	pDestState->bDitherEnabled = fdraw_Color_IsDitherEnabled();
	pDestState->nAlphaTest = fdraw_Alpha_GetTest( &pDestState->fAlphaReference );
	pDestState->nBlendOp = fdraw_Alpha_GetBlendOp();
	pDestState->bDepthWriteEnable = fdraw_Depth_IsWritingEnabled();
	pDestState->nDepthTest = fdraw_Depth_GetTest();
	pDestState->nDepthBiasLevel = fdraw_Depth_GetBiasLevel();
	fdraw_Stencil_GetMode(
		&pDestState->nStencil_Test,
		&pDestState->nStencil_Reference,
		&pDestState->nStencil_TestMask,
		&pDestState->nStencil_WriteMask,
		&pDestState->nStencil_StencilFailOp,
		&pDestState->nStencil_DepthFailOp,
		&pDestState->nStencil_DepthPassOp
	);
}


//
//
//
static void _ApplyState( const _State_t *pNewState ) 
{
	fdraw_SetCullDir( pNewState->nCullDir );
	fdraw_EnableClipping( pNewState->bClipEnabled );
	fdraw_Color_SetFunc( pNewState->nColorFunc );
	fdraw_SetTexture( (CFTexInst *)&pNewState->TexInst );
	fdraw_SetTexCoordXfmMode( pNewState->nTexCoordXfmMode );
	fdraw_SetTexCoordXfmMtx( &pNewState->TexCoordXfmMtx43A );
	fdraw_Color_SetMask( pNewState->bColorMask_Red, pNewState->bColorMask_Green, pNewState->bColorMask_Blue );
	fdraw_Color_EnableDither( pNewState->bDitherEnabled );
	fdraw_Alpha_SetTest( pNewState->nAlphaTest, pNewState->fAlphaReference );
	fdraw_Alpha_SetBlendOp( pNewState->nBlendOp );
	fdraw_Depth_EnableWriting( pNewState->bDepthWriteEnable );
	fdraw_Depth_SetTest( pNewState->nDepthTest );
	fdraw_Depth_SetBiasLevel( pNewState->nDepthBiasLevel );
	fdraw_Stencil_SetMode(
		pNewState->nStencil_Test,
		pNewState->nStencil_Reference,
		pNewState->nStencil_TestMask,
		pNewState->nStencil_WriteMask,
		pNewState->nStencil_StencilFailOp,
		pNewState->nStencil_DepthFailOp,
		pNewState->nStencil_DepthPassOp
	);
}


//
// returns TRUE if rSphere was filled in with a bounding sphere of all paVtx->Pos_MS points
//
BOOL fdraw_ComputeBoundingSphere( const FDrawVtx_t *paVtx, u32 nVtxCount, CFSphere &rSphere ) 
{
	CFVec3 Min, Max;

	if( !fdraw_ComputeMinMaxBox( paVtx, nVtxCount, Min, Max ) ) 
	{
		return FALSE;
	}
	rSphere.BuildFromMinMaxBox( Min, Max );

	return TRUE;
}


//
//
//
BOOL fdraw_ComputeMinMaxBox( const FDrawVtx_t *paVtx, u32 nVtxCount, CFVec3 &rMin, CFVec3 &rMax ) 
{
	u32 i;

	if( !nVtxCount || !paVtx ) 
	{
		return FALSE;
	}
	rMin = paVtx->Pos_MS;
	rMax = paVtx->Pos_MS;

	for( i=0; i < nVtxCount; i++ ) 
	{
		++paVtx;
		rMin.Min( paVtx->Pos_MS );
		rMax.Max( paVtx->Pos_MS );
	}
	return TRUE;
}


#if !FANG_PRODUCTION_BUILD

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// SHAPE DRAWING FUNCTIONS FOR DEVELOPMENT PURPOSES
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//
//
//
void fdraw_DevCapsule( const CFCapsule *pCapsule_WS, CFColorRGBA *pColor/*=&FColor_MotifWhite*/, u32 nSliceCountPerHalf/*=3*/, u32 nRingCountPerQuarter/*=3*/, u32 nVtxCountPerQuarter/*=3*/ )
{
	if ( !pCapsule_WS || _nDevCapsuleCount == _MAX_DEV_CAPSULES )
	{
		return;
	}

	if ( nSliceCountPerHalf > 15 )
	{
		nSliceCountPerHalf = 15;
	}

	if ( nRingCountPerQuarter > 15 )
	{
		nRingCountPerQuarter = 15;
	}

	if ( nVtxCountPerQuarter > 15 )
	{
		nVtxCountPerQuarter = 15;
	}

	_aCapsuleTessalationInfo[_nDevCapsuleCount] = (nSliceCountPerHalf << 8) | (nRingCountPerQuarter << 4) | (nVtxCountPerQuarter);
	_aCapsules[_nDevCapsuleCount] = *pCapsule_WS;
	_aCapsuleColors[_nDevCapsuleCount].Set( *pColor );
	_nDevCapsuleCount++;
}


//
//
//
void fdraw_DevSphere( const CFVec3 *pCenter_WS, f32 fRadius_WS, CFColorRGBA *pColor/*=&FColor_MotifWhite*/, u32 nSliceCountPerHalf/*=3*/, u32 nRingCountPerQuarter/*=3*/, u32 nVtxCountPerQuarter/*=3*/ )
{
	if ( !pCenter_WS || _nDevSphereCount == _MAX_DEV_SPHERES )
	{
		return;
	}

	if ( nSliceCountPerHalf > 15 )
	{
		nSliceCountPerHalf = 15;
	}

	if ( nRingCountPerQuarter > 15 )
	{
		nRingCountPerQuarter = 15;
	}

	if ( nVtxCountPerQuarter > 15 )
	{
		nVtxCountPerQuarter = 15;
	}

	_aSphereTessalationInfo[_nDevSphereCount] = (nSliceCountPerHalf << 8) | (nRingCountPerQuarter << 4) | (nVtxCountPerQuarter);
	_aSpheres[_nDevSphereCount].m_Pos = *pCenter_WS;
	_aSpheres[_nDevSphereCount].m_fRadius = fRadius_WS;
	_aSphereColors[_nDevSphereCount].Set( *pColor );
	_nDevSphereCount++;
}


//
//
//
void fdraw_DevTriangle( const CFVec3 *pPoint0_WS, const CFVec3 *pPoint1_WS, const CFVec3 *pPoint2_WS, CFColorRGBA *pColor/*=&FColor_MotifWhite*/ )
{
	if ( !pPoint0_WS || !pPoint1_WS || !pPoint2_WS || _nDevTriangleCount == _MAX_DEV_TRIANGLES )
	{
		return;
	}

	u32 nBaseIdx = _nDevTriangleCount * 3;
	_avTrianglePoints[nBaseIdx] = *pPoint0_WS;
	_avTrianglePoints[nBaseIdx+1] = *pPoint1_WS;
	_avTrianglePoints[nBaseIdx+2] = *pPoint2_WS;

	_aTriangleColors[_nDevTriangleCount].Set( *pColor );

	_nDevTriangleCount++;
}


//
//
//
void fdraw_DevLine( const CFVec3 *pPoint0_WS, const CFVec3 *pPoint1_WS, CFColorRGBA *pColor1/*=&FColor_MotifWhite*/, CFColorRGBA *pColor2/*=&FColor_MotifWhite*/ )
{
	if ( !pPoint0_WS || !pPoint1_WS || _nDevLineCount == _MAX_DEV_LINES )
	{
		return;
	}

	_avLinePoints[_nDevLineCount<<1] = *pPoint0_WS;
	_aLineColors[_nDevLineCount<<1].Set( *pColor2 );
	_avLinePoints[(_nDevLineCount<<1)+1] = *pPoint1_WS;
	_aLineColors[(_nDevLineCount<<1)+1].Set( *pColor2 );
	_nDevLineCount++;
}


//
//
//
void fdraw_RenderDevShapes( void )
{
	// Setup fdraw for rendering
	frenderer_Push( FRENDERER_DRAW, NULL );

	// Set the states we need
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Depth_EnableWriting( TRUE );
	
	s32 i;

	for ( i = 0; i < _nDevLineCount; i++ )
	{
		fdraw_SolidLine( &_avLinePoints[i<<1], &_avLinePoints[(i<<1)+1], &_aLineColors[i<<1], &_aLineColors[(i<<1)+1] );
	}

	for ( i = 0; i < _nDevTriangleCount; i++ )
	{
		u32 nBaseIdx = i * 3;
		fdraw_SolidTriangle( &_avTrianglePoints[nBaseIdx], &_avTrianglePoints[nBaseIdx+1], &_avTrianglePoints[nBaseIdx+2], &_aTriangleColors[i] );
	}

	for ( i = 0; i < _nDevSphereCount; i++ )
	{
		fdraw_FacetedWireSphere( &_aSpheres[i].m_Pos, 
								_aSpheres[i].m_fRadius, 
								(_aSphereTessalationInfo[i] >> 8) & 15, 
								(_aSphereTessalationInfo[i] >> 4) & 15, 
								&_aSphereColors[i], 
								_aSphereTessalationInfo[i] & 15 );
	}

	for ( i = 0; i < _nDevCapsuleCount; i++ )
	{
			fdraw_FacetedWireCapsule( &_aCapsules[i], 
									(_aCapsuleTessalationInfo[i] >> 8) & 15, 
									(_aCapsuleTessalationInfo[i] >> 4) & 15, 
									&_aCapsuleColors[i], 
									_aCapsuleTessalationInfo[i] & 15 );
	}

	// Pop the render
	frenderer_Pop();
}


//
//
//
void fdraw_ClearDevShapes( void )
{
	_nDevLineCount = 0;
	_nDevTriangleCount = 0;
	_nDevSphereCount = 0;
	_nDevCapsuleCount = 0;
}


#endif // !FANG_PRODUCTION BUILD