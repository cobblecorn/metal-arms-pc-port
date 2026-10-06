//////////////////////////////////////////////////////////////////////////////////////
// fdx8.h - Fang commonly used DX8 variables.
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
// 09/30/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8_H_
#define _FDX8_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fdx8vshader_const.h"


#if FANG_PLATFORM_XB
	#include <xtl.h>
	#include <xgmath.h>
#endif

#include <d3d8.h>
#include <d3dx8.h>


// Note: The DX8 docs are wrong regarding the parameters used in D3DTOP_MULTIPLYADD and D3DTOP_LERP.
//       Here are the correct parameters:
//
// Triadic ops
//   D3DTOP_MULTIPLYADD:	Arg0 + Arg1*Arg2
//   D3DTOP_LERP:			(Arg0)*Arg1 + (1-Arg0)*Arg2


#define FDX8_MAX_TEX_STAGES		8	// DX8 limitation (mileage may vary depending on the particular hardware)
#define FDX8_MAX_LIGHTS			8	// DX8 limitation (mileage may vary depending on the particular hardware)
#define FDX8_MAX_ZBIAS			16	// DX8 limitation


#define FDX8_SAFE_RELEASE(p)	{ if(p) { (p)->Release(); (p)=NULL; } }

#if FANG_WINGC
// Saturate effect fades before unsigned conversion; NaN maps to zero.
FINLINE u32 fdx8_ColorByte( f32 fValue ) {
	if( !(fValue > 0.0f) ) return 0;
	if( fValue >= 1.0f ) return 255;
	return fmath_FloatToU32( fValue * 255.0f );
}
#define FDX8_RGBA_TO_DXCOLOR( ColorRGBA ) ( \
	(fdx8_ColorByte((ColorRGBA).fAlpha)<<24) | \
	(fdx8_ColorByte((ColorRGBA).fRed)<<16) | \
	(fdx8_ColorByte((ColorRGBA).fGreen)<<8) | fdx8_ColorByte((ColorRGBA).fBlue) )
#else
#define FDX8_RGBA_TO_DXCOLOR( ColorRGBA ) (		\
			(fmath_FloatToU32((ColorRGBA).fAlpha*255.0f)<<24)	|	\
			(fmath_FloatToU32((ColorRGBA).fRed*255.0f)<<16)		|	\
			(fmath_FloatToU32((ColorRGBA).fGreen*255.0f)<<8)	|	\
			fmath_FloatToU32((ColorRGBA).fBlue*255.0f) )
#endif


extern IDirect3D8			*FDX8_pD3D;
extern IDirect3DDevice8		*FDX8_pDev;
extern IDirect3DSurface8	*FDX8_pSurfBack;
extern IDirect3DSurface8	*FDX8_pSurfDepthStencil;

extern D3DCAPS8				FDX8_Caps;
extern D3DSURFACE_DESC		FDX8_BackDesc;
extern D3DSURFACE_DESC		FDX8_DepthStencilDesc;

extern BOOL					FDX8_bHardwareVertexShaders;

extern u32					FDX8_nStageCount;
extern BOOL					FDX8_bDXTexStageArg0Supported;
extern BOOL					FDX8_bDXTexStageResultArgSupported;

extern u32					FDX8_nMaxD3DLights;

extern const D3DMATRIX		FDX8_MatrixIdentity;


#if FANG_PLATFORM_XB
	// Global xbox information about how we were launched
	extern DWORD			FDX8_uLaunchType;
	extern LAUNCH_DATA		FDX8_LaunchData;
	extern char				FDX8_szXBEName[ MAX_PATH+1 ];
#endif

extern BOOL fdx8_Startup( void );
extern void fdx8_Shutdown( void );
extern void fdx8_InitD3DState( void );


typedef enum {
	FDX8_RENDERSTATE_FILLMODE,
	FDX8_RENDERSTATE_SHADEMODE,
	FDX8_RENDERSTATE_ZWRITEENABLE,
	FDX8_RENDERSTATE_ALPHATESTENABLE,
	FDX8_RENDERSTATE_SRCBLEND,
	FDX8_RENDERSTATE_DESTBLEND,
	FDX8_RENDERSTATE_CULLMODE,
	FDX8_RENDERSTATE_ZFUNC,
	FDX8_RENDERSTATE_ALPHAREF,
	FDX8_RENDERSTATE_ALPHAFUNC,
	FDX8_RENDERSTATE_DITHERENABLE,
	FDX8_RENDERSTATE_ALPHABLENDENABLE,
	FDX8_RENDERSTATE_FOGENABLE,
	FDX8_RENDERSTATE_SPECULARENABLE,
	FDX8_RENDERSTATE_FOGCOLOR,
	FDX8_RENDERSTATE_FOGTABLEMODE,
	FDX8_RENDERSTATE_FOGSTART,
	FDX8_RENDERSTATE_FOGEND,
	FDX8_RENDERSTATE_FOGDENSITY,
	FDX8_RENDERSTATE_ZBIAS,
	FDX8_RENDERSTATE_RANGEFOGENABLE,
	FDX8_RENDERSTATE_STENCILENABLE,
	FDX8_RENDERSTATE_STENCILFAIL,
	FDX8_RENDERSTATE_STENCILZFAIL,
	FDX8_RENDERSTATE_STENCILPASS,
	FDX8_RENDERSTATE_STENCILFUNC,
	FDX8_RENDERSTATE_STENCILREF,
	FDX8_RENDERSTATE_STENCILMASK,
	FDX8_RENDERSTATE_STENCILWRITEMASK,
	FDX8_RENDERSTATE_TEXTUREFACTOR,
	FDX8_RENDERSTATE_LIGHTING,
	FDX8_RENDERSTATE_AMBIENT,
	FDX8_RENDERSTATE_LOCALVIEWER,
	FDX8_RENDERSTATE_NORMALIZENORMALS,
	FDX8_RENDERSTATE_DIFFUSEMATERIALSOURCE,
	FDX8_RENDERSTATE_SPECULARMATERIALSOURCE,
	FDX8_RENDERSTATE_AMBIENTMATERIALSOURCE,
	FDX8_RENDERSTATE_EMISSIVEMATERIALSOURCE,
	FDX8_RENDERSTATE_VERTEXBLEND,
	FDX8_RENDERSTATE_POINTSIZE,
	FDX8_RENDERSTATE_POINTSIZE_MIN,
	FDX8_RENDERSTATE_POINTSPRITEENABLE,
	FDX8_RENDERSTATE_POINTSCALEENABLE,
	FDX8_RENDERSTATE_POINTSCALE_A,
	FDX8_RENDERSTATE_POINTSCALE_B,
	FDX8_RENDERSTATE_POINTSCALE_C,
	FDX8_RENDERSTATE_PATCHEDGESTYLE,
	FDX8_RENDERSTATE_PATCHSEGMENTS,
	FDX8_RENDERSTATE_POINTSIZE_MAX,
	FDX8_RENDERSTATE_COLORWRITEENABLE,
	FDX8_RENDERSTATE_BLENDOP,

#	if FANG_PLATFORM_WIN
		FDX8_RENDERSTATE_CLIPPING,
		FDX8_RENDERSTATE_FOGVERTEXMODE,
		FDX8_RENDERSTATE_TWEENFACTOR,
#	endif

	FDX8_RENDERSTATE_COUNT
} FDX8_RenderState_e;


typedef enum {
    FDX8_TEXTURESTATE_COLOROP,
    FDX8_TEXTURESTATE_COLORARG1,
    FDX8_TEXTURESTATE_COLORARG2,
    FDX8_TEXTURESTATE_ALPHAOP,
    FDX8_TEXTURESTATE_ALPHAARG1,
    FDX8_TEXTURESTATE_ALPHAARG2,
    FDX8_TEXTURESTATE_BUMPENVMAT00,
    FDX8_TEXTURESTATE_BUMPENVMAT01,
    FDX8_TEXTURESTATE_BUMPENVMAT10,
    FDX8_TEXTURESTATE_BUMPENVMAT11,
    FDX8_TEXTURESTATE_BUMPENVLSCALE,
    FDX8_TEXTURESTATE_BUMPENVLOFFSET,
    FDX8_TEXTURESTATE_TEXTURETRANSFORMFLAGS,
    FDX8_TEXTURESTATE_COLORARG0,
    FDX8_TEXTURESTATE_ALPHAARG0,
    FDX8_TEXTURESTATE_RESULTARG,

	FDX8_TEXTURESTATE_COUNT
} FDX8_TextureState_e;


extern DWORD		FDX8_anRenderState[ FDX8_RENDERSTATE_COUNT ];
extern DWORD		FDX8_anTextureState[ FDX8_MAX_TEX_STAGES ][ FDX8_TEXTURESTATE_COUNT ];
extern BOOL			FDX8_abLightEnable[ FDX8_MAX_LIGHTS ];
extern D3DLIGHT8	FDX8_aLight[ FDX8_MAX_LIGHTS ];
extern D3DMATERIAL8 FDX8_Material;
#if FANG_PLATFORM_XB
	extern XGMATRIX		FDX8_CurrentViewMtx;
	extern XGMATRIX		FDX8_InvCurrentViewMtx;
	extern XGMATRIX		FDX8_CurrentProjMtx;
#else
	extern D3DXMATRIX	FDX8_CurrentViewMtx;
	extern D3DXMATRIX	FDX8_InvCurrentViewMtx;
	extern D3DXMATRIX	FDX8_CurrentProjMtx;
#endif
extern CFMtx43A		FDX8_CurrentFangViewMtx;
extern CFMtx43A		FDX8_CurrentFangProjMtx;
extern u32			FDX8_uCurrentVertexShader;
extern BOOL			FDX8_bViewMtxIsIdentity;

#define FDX8_CACHE_STATES	TRUE

//
//
FINLINE HRESULT FDX8_SetViewMatrixIdentity( void )
{
#if FDX8_CACHE_STATES
	if ( FDX8_bViewMtxIsIdentity )
	{
		return D3D_OK;
	}
#endif

	FDX8_bViewMtxIsIdentity = TRUE;
	FDX8_CurrentFangViewMtx.Identity();
	FDX8_CurrentViewMtx = FDX8_MatrixIdentity;
	FDX8_InvCurrentViewMtx = FDX8_MatrixIdentity;
	FDX8_pDev->SetVertexShaderConstant( CV_VIEW_0, &FDX8_CurrentFangViewMtx, 4 );
	HRESULT hResult = FDX8_pDev->SetTransform( D3DTS_VIEW, &FDX8_CurrentViewMtx );
	if ( hResult == D3D_OK )
	{
		static CFMtx44A mtxViewProj;
		mtxViewProj.Mul( FDX8_CurrentFangViewMtx.m44a, FDX8_CurrentFangProjMtx.m44a );
		hResult = FDX8_pDev->SetVertexShaderConstant( CV_VIEWPROJ_0, &mtxViewProj, 4 );
	}
	return hResult;
}

//
//
FINLINE HRESULT FDX8_SetViewMatrix( CFMtx43A *pFangViewMatrix )
{
	FDX8_bViewMtxIsIdentity = FALSE;
	FDX8_CurrentFangViewMtx.Set( *pFangViewMatrix );
	FDX8_CurrentFangViewMtx.aa[3][3] = 1.f;
	FDX8_CurrentViewMtx._11 = pFangViewMatrix->aa[0][0];
	FDX8_CurrentViewMtx._12 = pFangViewMatrix->aa[0][1];
	FDX8_CurrentViewMtx._13 = pFangViewMatrix->aa[0][2];
	FDX8_CurrentViewMtx._14 = 0.0f;
	FDX8_CurrentViewMtx._21 = pFangViewMatrix->aa[1][0];
	FDX8_CurrentViewMtx._22 = pFangViewMatrix->aa[1][1];
	FDX8_CurrentViewMtx._23 = pFangViewMatrix->aa[1][2];
	FDX8_CurrentViewMtx._24 = 0.0f;
	FDX8_CurrentViewMtx._31 = pFangViewMatrix->aa[2][0];
	FDX8_CurrentViewMtx._32 = pFangViewMatrix->aa[2][1];
	FDX8_CurrentViewMtx._33 = pFangViewMatrix->aa[2][2];
	FDX8_CurrentViewMtx._34 = 0.0f;
	FDX8_CurrentViewMtx._41 = pFangViewMatrix->aa[3][0];
	FDX8_CurrentViewMtx._42 = pFangViewMatrix->aa[3][1];
	FDX8_CurrentViewMtx._43 = pFangViewMatrix->aa[3][2];
	FDX8_CurrentViewMtx._44 = 1.0f;

	#if FANG_PLATFORM_XB
		XGMatrixInverse( &FDX8_InvCurrentViewMtx, NULL, (const XGMATRIX *)&FDX8_CurrentFangViewMtx );
	#else
		D3DXMatrixInverse( &FDX8_InvCurrentViewMtx, NULL, (const D3DXMATRIX *)&FDX8_CurrentFangViewMtx );
	#endif

	FDX8_pDev->SetVertexShaderConstant( CV_VIEW_0, &FDX8_CurrentFangViewMtx, 4 );
	HRESULT hResult = FDX8_pDev->SetTransform( D3DTS_VIEW, &FDX8_CurrentViewMtx );
	if ( hResult == D3D_OK )
	{
	#if FANG_PLATFORM_XB
		XGMATRIX matViewProj;
		XGMatrixMultiply(&matViewProj, &FDX8_CurrentViewMtx, &FDX8_CurrentProjMtx);
			
		XGMatrixTranspose(&matViewProj, &matViewProj);
	#else
		D3DXMATRIX matViewProj;
		D3DXMatrixMultiply(&matViewProj, &FDX8_CurrentViewMtx, &FDX8_CurrentProjMtx);
			
		D3DXMatrixTranspose(&matViewProj, &matViewProj);
	#endif
		FDX8_pDev->SetVertexShaderConstant(CV_VIEWPROJ_0, &matViewProj(0, 0), 4);
	}
	return hResult;
}

//
//
FINLINE HRESULT FDX8_SetProjMatrix( CFMtx44A *pD3DProjMatrix )
{
	FDX8_CurrentFangProjMtx.m44a.ReceiveTranspose( *pD3DProjMatrix );
	FDX8_CurrentProjMtx = *(D3DXMATRIX *)pD3DProjMatrix;
	HRESULT hResult = FDX8_pDev->SetTransform( D3DTS_PROJECTION, &FDX8_CurrentProjMtx );
	if ( hResult == D3D_OK )
	{
	#if FANG_PLATFORM_XB
		XGMATRIX matViewProj;
		XGMatrixMultiply(&matViewProj, &FDX8_CurrentViewMtx, &FDX8_CurrentProjMtx);
			
		XGMatrixTranspose(&matViewProj, &matViewProj);
	#else
		D3DXMATRIX matViewProj;
		D3DXMatrixMultiply(&matViewProj, &FDX8_CurrentViewMtx, &FDX8_CurrentProjMtx);
			
		D3DXMatrixTranspose(&matViewProj, &matViewProj);
	#endif
		FDX8_pDev->SetVertexShaderConstant(CV_VIEWPROJ_0, &matViewProj(0, 0), 4);
	}
	return hResult;
}

//
//
FINLINE HRESULT FDX8_SetProjMatrix( D3DXMATRIX *pD3DProjMatrix )
{
	FDX8_CurrentFangProjMtx.m44 = *(CFMtx44 *)pD3DProjMatrix;
	FDX8_CurrentFangProjMtx.m44a.Transpose();
	FDX8_CurrentProjMtx = *pD3DProjMatrix;
	HRESULT hResult = FDX8_pDev->SetTransform( D3DTS_PROJECTION, &FDX8_CurrentProjMtx );
	if ( hResult == D3D_OK )
	{
	#if FANG_PLATFORM_XB
		XGMATRIX matViewProj;
		XGMatrixMultiply(&matViewProj, &FDX8_CurrentViewMtx, &FDX8_CurrentProjMtx);
			
		XGMatrixTranspose(&matViewProj, &matViewProj);
	#else
		D3DXMATRIX matViewProj;
		D3DXMatrixMultiply(&matViewProj, &FDX8_CurrentViewMtx, &FDX8_CurrentProjMtx);
			
		D3DXMatrixTranspose(&matViewProj, &matViewProj);
	#endif
		hResult = FDX8_pDev->SetVertexShaderConstant( CV_VIEWPROJ_0, &matViewProj(0, 0), 4 );
	}
	return hResult;
}

//
//
FINLINE HRESULT FDX8_SetVertexShader( u32 nShader )
{
	#if FDX8_CACHE_STATES
		if ( nShader == FDX8_uCurrentVertexShader )
		{
			return D3D_OK;
		}
		FDX8_uCurrentVertexShader = nShader;
	#endif

	return FDX8_pDev->SetVertexShader( nShader );
}

#if FDX8_CACHE_STATES
	// Cache D3D states:
	#define FDX8_CREATE_RENDERSTATE_FCN( szUppercaseName )										\
		inline void fdx8_SetRenderState_##szUppercaseName( DWORD nValue ) {						\
			if( FDX8_anRenderState[FDX8_RENDERSTATE_##szUppercaseName] != nValue ) {			\
				FDX8_anRenderState[FDX8_RENDERSTATE_##szUppercaseName] = nValue;				\
				FDX8_pDev->SetRenderState( D3DRS_##szUppercaseName, nValue );					\
			}																					\
		}

	#define FDX8_CREATE_TEXTURESTATE_FCN( szUppercaseName )										\
		inline void fdx8_SetTextureState_##szUppercaseName( u32 nStage, DWORD nValue ) {		\
			FASSERT( nStage < FDX8_MAX_TEX_STAGES );											\
			if( FDX8_anTextureState[nStage][FDX8_TEXTURESTATE_##szUppercaseName] != nValue ) {	\
				FDX8_anTextureState[nStage][FDX8_TEXTURESTATE_##szUppercaseName] = nValue;		\
				FDX8_pDev->SetTextureStageState( nStage, D3DTSS_##szUppercaseName, nValue );	\
			}																					\
		}

	inline void fdx8_LightEnable( u32 nLightIndex, BOOL bEnable ) {
		FASSERT( nLightIndex < FDX8_MAX_LIGHTS );

		bEnable = !!bEnable;

		if( FDX8_abLightEnable[nLightIndex] != bEnable ) {
			FDX8_abLightEnable[nLightIndex] = bEnable;
			FDX8_pDev->LightEnable( nLightIndex, bEnable );
		}
	}

	inline void fdx8_SetLight( u32 nLightIndex, CONST D3DLIGHT8* pLight ) {
		FASSERT( nLightIndex < FDX8_MAX_LIGHTS );

		if( !fang_IsMemIdentical( &FDX8_aLight[nLightIndex], pLight, sizeof(D3DLIGHT8) ) ) {
			FANG_COPYSTRUCT( FDX8_aLight[nLightIndex], *pLight );
			FDX8_pDev->SetLight( nLightIndex, pLight );
		}
	}

	inline void fdx8_SetMaterial( CONST D3DMATERIAL8* pMaterial ) {
		if( !fang_IsMemIdentical( &FDX8_Material, pMaterial, sizeof(D3DMATERIAL8) ) ) {
			FANG_COPYSTRUCT( FDX8_Material, *pMaterial );
			FDX8_pDev->SetMaterial( pMaterial );
		}
	}

#else
	// Don't cache D3D states:

	#define FDX8_CREATE_RENDERSTATE_FCN( szUppercaseName )										\
		inline void fdx8_SetRenderState_##szUppercaseName( DWORD nValue ) {						\
			FDX8_anRenderState[FDX8_RENDERSTATE_##szUppercaseName] = nValue;					\
			FDX8_pDev->SetRenderState( D3DRS_##szUppercaseName, nValue );						\
		}

	#define FDX8_CREATE_TEXTURESTATE_FCN( szUppercaseName )										\
		inline void fdx8_SetTextureState_##szUppercaseName( u32 nStage, DWORD nValue ) {		\
			FASSERT( nStage < FDX8_MAX_TEX_STAGES );											\
			FDX8_anTextureState[nStage][FDX8_TEXTURESTATE_##szUppercaseName] = nValue;			\
			FDX8_pDev->SetTextureStageState( nStage, D3DTSS_##szUppercaseName, nValue );		\
		}

	inline void fdx8_LightEnable( u32 nLightIndex, BOOL bEnable ) {
		FASSERT( nLightIndex < FDX8_MAX_LIGHTS );
		bEnable = !!bEnable;
		FDX8_abLightEnable[nLightIndex] = bEnable;
		FDX8_pDev->LightEnable( nLightIndex, bEnable );
	}

	inline void fdx8_SetLight( u32 nLightIndex, CONST D3DLIGHT8* pLight ) {
		FASSERT( nLightIndex < FDX8_MAX_LIGHTS );
		FANG_COPYSTRUCT( FDX8_aLight[nLightIndex], *pLight );
		FDX8_pDev->SetLight( nLightIndex, pLight );
	}

	inline void fdx8_SetMaterial( CONST D3DMATERIAL8* pMaterial ) {
		FANG_COPYSTRUCT( FDX8_Material, *pMaterial );
		FDX8_pDev->SetMaterial( pMaterial );
	}

#endif


FDX8_CREATE_RENDERSTATE_FCN( FILLMODE )
FDX8_CREATE_RENDERSTATE_FCN( SHADEMODE )
FDX8_CREATE_RENDERSTATE_FCN( ZWRITEENABLE )
FDX8_CREATE_RENDERSTATE_FCN( ALPHATESTENABLE )
FDX8_CREATE_RENDERSTATE_FCN( SRCBLEND )
FDX8_CREATE_RENDERSTATE_FCN( DESTBLEND )
FDX8_CREATE_RENDERSTATE_FCN( CULLMODE )
FDX8_CREATE_RENDERSTATE_FCN( ZFUNC )
FDX8_CREATE_RENDERSTATE_FCN( ALPHAREF )
FDX8_CREATE_RENDERSTATE_FCN( ALPHAFUNC )
FDX8_CREATE_RENDERSTATE_FCN( DITHERENABLE )
FDX8_CREATE_RENDERSTATE_FCN( ALPHABLENDENABLE )
FDX8_CREATE_RENDERSTATE_FCN( FOGENABLE )
FDX8_CREATE_RENDERSTATE_FCN( SPECULARENABLE )
FDX8_CREATE_RENDERSTATE_FCN( FOGCOLOR )
FDX8_CREATE_RENDERSTATE_FCN( FOGTABLEMODE )
FDX8_CREATE_RENDERSTATE_FCN( FOGSTART )
FDX8_CREATE_RENDERSTATE_FCN( FOGEND )
FDX8_CREATE_RENDERSTATE_FCN( FOGDENSITY )
FDX8_CREATE_RENDERSTATE_FCN( ZBIAS )
FDX8_CREATE_RENDERSTATE_FCN( RANGEFOGENABLE )
FDX8_CREATE_RENDERSTATE_FCN( STENCILENABLE )
FDX8_CREATE_RENDERSTATE_FCN( STENCILFAIL )
FDX8_CREATE_RENDERSTATE_FCN( STENCILZFAIL )
FDX8_CREATE_RENDERSTATE_FCN( STENCILPASS )
FDX8_CREATE_RENDERSTATE_FCN( STENCILFUNC )
FDX8_CREATE_RENDERSTATE_FCN( STENCILREF )
FDX8_CREATE_RENDERSTATE_FCN( STENCILMASK )
FDX8_CREATE_RENDERSTATE_FCN( STENCILWRITEMASK )
FDX8_CREATE_RENDERSTATE_FCN( TEXTUREFACTOR )
FDX8_CREATE_RENDERSTATE_FCN( LIGHTING )
FDX8_CREATE_RENDERSTATE_FCN( AMBIENT )
FDX8_CREATE_RENDERSTATE_FCN( LOCALVIEWER )
FDX8_CREATE_RENDERSTATE_FCN( NORMALIZENORMALS )
FDX8_CREATE_RENDERSTATE_FCN( DIFFUSEMATERIALSOURCE )
FDX8_CREATE_RENDERSTATE_FCN( SPECULARMATERIALSOURCE )
FDX8_CREATE_RENDERSTATE_FCN( AMBIENTMATERIALSOURCE )
FDX8_CREATE_RENDERSTATE_FCN( EMISSIVEMATERIALSOURCE )
FDX8_CREATE_RENDERSTATE_FCN( VERTEXBLEND )
FDX8_CREATE_RENDERSTATE_FCN( POINTSIZE )
FDX8_CREATE_RENDERSTATE_FCN( POINTSIZE_MIN )
FDX8_CREATE_RENDERSTATE_FCN( POINTSPRITEENABLE )
FDX8_CREATE_RENDERSTATE_FCN( POINTSCALEENABLE )
FDX8_CREATE_RENDERSTATE_FCN( POINTSCALE_A )
FDX8_CREATE_RENDERSTATE_FCN( POINTSCALE_B )
FDX8_CREATE_RENDERSTATE_FCN( POINTSCALE_C )
FDX8_CREATE_RENDERSTATE_FCN( PATCHEDGESTYLE )
FDX8_CREATE_RENDERSTATE_FCN( PATCHSEGMENTS )
FDX8_CREATE_RENDERSTATE_FCN( POINTSIZE_MAX )
FDX8_CREATE_RENDERSTATE_FCN( COLORWRITEENABLE )
FDX8_CREATE_RENDERSTATE_FCN( BLENDOP )

#if FANG_PLATFORM_WIN
	FDX8_CREATE_RENDERSTATE_FCN( CLIPPING )
	FDX8_CREATE_RENDERSTATE_FCN( FOGVERTEXMODE )
	FDX8_CREATE_RENDERSTATE_FCN( TWEENFACTOR )
#endif

FDX8_CREATE_TEXTURESTATE_FCN( COLOROP )
FDX8_CREATE_TEXTURESTATE_FCN( COLORARG1 )
FDX8_CREATE_TEXTURESTATE_FCN( COLORARG2 )
FDX8_CREATE_TEXTURESTATE_FCN( ALPHAOP )
FDX8_CREATE_TEXTURESTATE_FCN( ALPHAARG1 )
FDX8_CREATE_TEXTURESTATE_FCN( ALPHAARG2 )
FDX8_CREATE_TEXTURESTATE_FCN( BUMPENVMAT00 )
FDX8_CREATE_TEXTURESTATE_FCN( BUMPENVMAT01 )
FDX8_CREATE_TEXTURESTATE_FCN( BUMPENVMAT10 )
FDX8_CREATE_TEXTURESTATE_FCN( BUMPENVMAT11 )
FDX8_CREATE_TEXTURESTATE_FCN( BUMPENVLSCALE )
FDX8_CREATE_TEXTURESTATE_FCN( BUMPENVLOFFSET )
FDX8_CREATE_TEXTURESTATE_FCN( TEXTURETRANSFORMFLAGS )
FDX8_CREATE_TEXTURESTATE_FCN( COLORARG0 )
FDX8_CREATE_TEXTURESTATE_FCN( ALPHAARG0 )
FDX8_CREATE_TEXTURESTATE_FCN( RESULTARG )


// The above defines the following functions:

// fdx8_SetRenderState_FILLMODE( DWORD nNewValue )
// fdx8_SetRenderState_SHADEMODE( DWORD nNewValue )
// fdx8_SetRenderState_ZWRITEENABLE( DWORD nNewValue )
// fdx8_SetRenderState_ALPHATESTENABLE( DWORD nNewValue )
// fdx8_SetRenderState_SRCBLEND( DWORD nNewValue )
// fdx8_SetRenderState_DESTBLEND( DWORD nNewValue )
// fdx8_SetRenderState_CULLMODE( DWORD nNewValue )
// fdx8_SetRenderState_ZFUNC( DWORD nNewValue )
// fdx8_SetRenderState_ALPHAREF( DWORD nNewValue )
// fdx8_SetRenderState_ALPHAFUNC( DWORD nNewValue )
// fdx8_SetRenderState_DITHERENABLE( DWORD nNewValue )
// fdx8_SetRenderState_ALPHABLENDENABLE( DWORD nNewValue )
// fdx8_SetRenderState_FOGENABLE( DWORD nNewValue )
// fdx8_SetRenderState_SPECULARENABLE( DWORD nNewValue )
// fdx8_SetRenderState_FOGCOLOR( DWORD nNewValue )
// fdx8_SetRenderState_FOGTABLEMODE( DWORD nNewValue )
// fdx8_SetRenderState_FOGSTART( DWORD nNewValue )
// fdx8_SetRenderState_FOGEND( DWORD nNewValue )
// fdx8_SetRenderState_FOGDENSITY( DWORD nNewValue )
// fdx8_SetRenderState_ZBIAS( DWORD nNewValue )
// fdx8_SetRenderState_RANGEFOGENABLE( DWORD nNewValue )
// fdx8_SetRenderState_STENCILENABLE( DWORD nNewValue )
// fdx8_SetRenderState_STENCILFAIL( DWORD nNewValue )
// fdx8_SetRenderState_STENCILZFAIL( DWORD nNewValue )
// fdx8_SetRenderState_STENCILPASS( DWORD nNewValue )
// fdx8_SetRenderState_STENCILFUNC( DWORD nNewValue )
// fdx8_SetRenderState_STENCILREF( DWORD nNewValue )
// fdx8_SetRenderState_STENCILMASK( DWORD nNewValue )
// fdx8_SetRenderState_STENCILWRITEMASK( DWORD nNewValue )
// fdx8_SetRenderState_TEXTUREFACTOR( DWORD nNewValue )
// fdx8_SetRenderState_LIGHTING( DWORD nNewValue )
// fdx8_SetRenderState_AMBIENT( DWORD nNewValue )
// fdx8_SetRenderState_LOCALVIEWER( DWORD nNewValue )
// fdx8_SetRenderState_NORMALIZENORMALS( DWORD nNewValue )
// fdx8_SetRenderState_DIFFUSEMATERIALSOURCE( DWORD nNewValue )
// fdx8_SetRenderState_SPECULARMATERIALSOURCE( DWORD nNewValue )
// fdx8_SetRenderState_AMBIENTMATERIALSOURCE( DWORD nNewValue )
// fdx8_SetRenderState_EMISSIVEMATERIALSOURCE( DWORD nNewValue )
// fdx8_SetRenderState_VERTEXBLEND( DWORD nNewValue )
// fdx8_SetRenderState_POINTSIZE( DWORD nNewValue )
// fdx8_SetRenderState_POINTSIZE_MIN( DWORD nNewValue )
// fdx8_SetRenderState_POINTSPRITEENABLE( DWORD nNewValue )
// fdx8_SetRenderState_POINTSCALEENABLE( DWORD nNewValue )
// fdx8_SetRenderState_POINTSCALE_A( DWORD nNewValue )
// fdx8_SetRenderState_POINTSCALE_B( DWORD nNewValue )
// fdx8_SetRenderState_POINTSCALE_C( DWORD nNewValue )
// fdx8_SetRenderState_PATCHEDGESTYLE( DWORD nNewValue )
// fdx8_SetRenderState_PATCHSEGMENTS( DWORD nNewValue )
// fdx8_SetRenderState_POINTSIZE_MAX( DWORD nNewValue )
// fdx8_SetRenderState_COLORWRITEENABLE( DWORD nNewValue )
// fdx8_SetRenderState_BLENDOP( DWORD nNewValue )
//
// PC Only:
// fdx8_SetRenderState_CLIPPING( DWORD nNewValue )
// fdx8_SetRenderState_FOGVERTEXMODE( DWORD nNewValue )
// fdx8_SetRenderState_TWEENFACTOR( DWORD nNewValue )



// fdx8_SetTextureState_COLOROP( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_COLORARG1( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_COLORARG2( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_ALPHAOP( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_ALPHAARG1( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_ALPHAARG2( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_BUMPENVMAT00( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_BUMPENVMAT01( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_BUMPENVMAT10( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_BUMPENVMAT11( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_BUMPENVLSCALE( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_BUMPENVLOFFSET( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_COLORARG0( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_ALPHAARG0( u32 nStageNum, DWORD nNewValue )
// fdx8_SetTextureState_RESULTARG( u32 nStageNum, DWORD nNewValue )

// fdx8_LightEnable( u32 nLightIndex, BOOL bEnable )
// fdx8_SetLight( u32 nLightIndex, CONST D3DLIGHT8* pLight )
// fdx8_SetMaterial( CONST D3DMATERIAL8* pMaterial )



#endif

