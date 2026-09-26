//////////////////////////////////////////////////////////////////////////////////////
// fdx8sh.h - Fang DX shader module.
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
// 11/01/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8SH_H_
#define _FDX8SH_H_ 1

#include "fang.h"
#include "fsh.h"

extern void fdx8sh_ToggleExternalColorStream(BOOL bExtStream=FALSE);

extern BOOL fdx8sh_bStream1Set;

// Returns the FDX8ShOp_e from the current address:
#define FDX8SH_GET_OP( pSh ) ((FDX8ShOp_e)(pSh)->n)


// Returns the u32 parameter from the current address:
#define FDX8SH_GET_PN( pSh ) ((pSh)->n)


// Returns the f32 parameter from the current address:
#define FDX8SH_GET_PF( pSh ) ((pSh)->f)


// Performs FDX8SH_GET_OP() and then advances the address:
#define FDX8SH_READ_OP( pSh ) ((FDX8ShOp_e)(pSh)->n); { (pSh)+=2; }


// Performs FDX8SH_GET_PN() and then advances the address:
#define FDX8SH_READ_PN( pSh ) ((pSh)->n); { (pSh)++; }


// Performs FDX8SH_GET_PF() and then advances the address:
#define FDX8SH_READ_PF( pSh ) ((pSh)->f); { (pSh)++; }


// Performs FDX8SH_GET_OP and then skips to the next op:
#define FDX8SH_NEXT_OP1( pSh ) ((FDX8ShOp_e)(pSh)->n); { (pSh)+=(((pSh)+1)->n+2); }


// Skips from the first parameter to the next op:
#define FDX8SH_NEXT_OP2( pSh ) { (pSh) += (((pSh)-1)->n); }



typedef struct {
	u32 n;
	f32 f;
} FDX8Sh_t;

typedef enum {
	FDX8SHOP_RESERVED,

	FDX8SHOP_END,
	FDX8SHOP_NAME,
	FDX8SHOP_METHOD,
	FDX8SHOP_PASS,
	FDX8SHOP_CALL,
	FDX8SHOP_CHECK,
	FDX8SHOP_REGROUTE,
	FDX8SHOP_MACRO,

	FDX8SHOP_ALPHABLEND_DISABLE,
	FDX8SHOP_ALPHABLEND_ENABLE,

	FDX8SHOP_ALPHATEST_DISABLE,
	FDX8SHOP_ALPHATEST_ENABLE,

	FDX8SHOP_DEPTHTEST_DISABLE,
	FDX8SHOP_DEPTHTEST_ENABLE,
	FDX8SHOP_DEPTHWRITE_DISABLE,
	FDX8SHOP_DEPTHWRITE_ENABLE,

	FDX8SHOP_SPECULAR_DISABLE,
	FDX8SHOP_SPECULAR_ENABLE,

	FDX8SHOP_FOGPASS_DISABLE,
	FDX8SHOP_FOGPASS_ENABLE,

	FDX8SHOP_LIGHTING_DISABLE,
	FDX8SHOP_LIGHTING_ENABLE,
	FDX8SHOP_LIGHTING_ENABLE_NOLIGHTS,

	FDX8SHOP_MTLSRCA,
	FDX8SHOP_MTLSRCE,
	FDX8SHOP_MTLSRCD,
	FDX8SHOP_MTLSRCS,

	FDX8SHOP_STAGES,
	FDX8SHOP_ENVMAP_ENABLE,
	FDX8SHOP_ENVMAP_DISABLE,
	FDX8SHOP_TEXTURE,
	FDX8SHOP_COLORSTAGE,
	FDX8SHOP_ALPHASTAGE,
	FDX8SHOP_ARG0,
	FDX8SHOP_RESULTARG,

	FDX8SHOP_SPECPOWER,
	FDX8SHOP_TFACTOR,
	FDX8SHOP_AMBIENT,
	FDX8SHOP_MTLA,
	FDX8SHOP_MTLE,
	FDX8SHOP_MTLD,
	FDX8SHOP_MTLS,

	FDX8SHOP_COUNT
} FDX8ShOp_e;


#define FDX8SHC( nShOp, nParamCount )	(nShOp), 0.0f, (nParamCount), 0.0f
#define FDX8SHP( nParam, fParam )		(nParam), (fParam)
#define FDX8SHP_U32( nParam )			FDX8SHP( nParam, 0.0f )
#define FDX8SHP_F32( fParam )			FDX8SHP( 0, fParam )

typedef enum {
	FDX8SHCC_K = 0,				// Constant color
	FDX8SHCC_RA,				// Select register A
	FDX8SHCC_RA_PLUS_RB,		// Select register A plus register B
	FDX8SHCC_RA_MULT_RB,		// Select register A times register B
	FDX8SHCC_RARC_PLUS_RBRC,	// Select (register A plus register B) times register C

	FDX8SHCC_COUNT
} FDX8ShCC_e;


#define FDX8SHCOLOR_KA( fA ) ( ((FDX8SHCC_K)<<24) | ((u32)((fA)*255.0f) & 0xff) )
#define FDX8SHCOLOR_KA0 FDX8SHCOLOR_KA(0.0f)
#define FDX8SHCOLOR_KA1 FDX8SHCOLOR_KA(1.0f)
#define FDX8SHCOLOR_KC( fR, fG, fB ) ( ((FDX8SHCC_K)<<24) | (((u32)((fR)*255.0f)<<16) & 0xff0000) | (((u32)((fG)*255.0f)<<8) & 0x00ff00) | ((u32)((fB)*255.0f) & 0x0000ff) )
#define FDX8SHCOLOR_KC0 FDX8SHCOLOR_KC(0.0f, 0.0f, 0.0f)
#define FDX8SHCOLOR_KC1 FDX8SHCOLOR_KC(1.0f, 1.0f, 1.0f)

#define FDX8SHCOLOR_RA( nRA ) ( ((FDX8SHCC_RA)<<24) | ((nRA)&0xff) )
#define FDX8SHCOLOR_RA_PLUS_RB( nRA, nRB ) ( ((FDX8SHCC_RA_PLUS_RB)<<24) | (((nRB)&0xff)<<8) | ((nRA)&0xff) )
#define FDX8SHCOLOR_RA_MULT_RB( nRA, nRB ) ( ((FDX8SHCC_RA_MULT_RB)<<24) | (((nRB)&0xff)<<8) | ((nRA)&0xff) )
#define FDX8SHCOLOR_RARC_PLUS_RBRC( nRA, nRB, nRC ) ( ((FDX8SHCC_RARC_PLUS_RBRC)<<24) | (((nRC)&0xff)<<16) | (((nRB)&0xff)<<8) | ((nRA)&0xff) )


// FDX8SHCMD_NAME( pszShaderName )
// FDX8SHCMD_CALL( pszShaderName )
// FDX8SHCMD_REGROUTE( nCallReg, nOurReg )
// FDX8SHCMD_MACRO( pszShaderName )
// FDX8SHCMD_METHOD
// FDX8SHCMD_PASS
// FDX8SHCMD_CHECK
// FDX8SHCMD_END
#define FDX8SHCMD_NAME( pszShaderName ) \
						FDX8SHC( FDX8SHOP_NAME, 1 ), \
						FDX8SHP_U32( (u32)pszShaderName )

#define FDX8SHCMD_CALL( pszShaderName ) \
						FDX8SHC( FDX8SHOP_CALL, 1 ), \
						FDX8SHP_U32( (u32)pszShaderName )

#define FDX8SHCMD_REGROUTE( nCallReg, nOurReg ) \
						FDX8SHC( FDX8SHOP_REGROUTE, 2 ), \
						FDX8SHP_U32( nCallReg ), \
						FDX8SHP_U32( nOurReg )

#define FDX8SHCMD_MACRO( pShaderMacro ) \
						FDX8SHC( FDX8SHOP_MACRO, 1 ), \
						FDX8SHP_U32( (u32)pShaderMacro )

#define FDX8SHCMD_METHOD	FDX8SHC( FDX8SHOP_METHOD, 0 )
#define FDX8SHCMD_PASS		FDX8SHC( FDX8SHOP_PASS, 0 )
#define FDX8SHCMD_CHECK		FDX8SHC( FDX8SHOP_CHECK, 0 )
#define FDX8SHCMD_END		FDX8SHC( FDX8SHOP_END, 0 )


// FDX8SHCMD_ALPHABLEND_DISABLE
// FDX8SHCMD_ALPHABLEND_ENABLE( nD3DSrcFactor, nD3DDestFactor, nD3DBlendOp )
#define FDX8SHCMD_ALPHABLEND_DISABLE \
						FDX8SHC( FDX8SHOP_ALPHABLEND_DISABLE, 0 )

#define FDX8SHCMD_ALPHABLEND_ENABLE( nD3DSrcFactor, nD3DDestFactor, nD3DBlendOp ) \
						FDX8SHC( FDX8SHOP_ALPHABLEND_ENABLE, 3 ), \
						FDX8SHP_U32(nD3DSrcFactor), \
						FDX8SHP_U32(nD3DDestFactor), \
						FDX8SHP_U32(nD3DBlendOp)


// FDX8SHCMD_ALPHATEST_DISABLE
// FDX8SHCMD_ALPHATEST_ENABLE( nD3DAlphaFunc, nD3DAlphaRef )
#define FDX8SHCMD_ALPHATEST_DISABLE \
						FDX8SHC( FDX8SHOP_ALPHATEST_DISABLE, 0 )

#define FDX8SHCMD_ALPHATEST_ENABLE( nD3DAlphaFunc, nD3DAlphaRef ) \
						FDX8SHC( FDX8SHOP_ALPHATEST_ENABLE, 2 ), \
						FDX8SHP_U32(nD3DAlphaFunc), \
						FDX8SHP_U32(nD3DAlphaRef)


// FDX8SHCMD_DEPTHTEST_DISABLE
// FDX8SHCMD_DEPTHTEST_ENABLE( nD3DDepthFunc )
#define FDX8SHCMD_DEPTHTEST_DISABLE \
						FDX8SHC( FDX8SHOP_DEPTHTEST_DISABLE, 0 )

#define FDX8SHCMD_DEPTHTEST_ENABLE( nD3DDepthFunc ) \
						FDX8SHC( FDX8SHOP_DEPTHTEST_ENABLE, 1 ), \
						FDX8SHP_U32(nD3DDepthFunc)


// FDX8SHCMD_DEPTHWRITE_DISABLE
// FDX8SHCMD_DEPTHWRITE_ENABLE
#define FDX8SHCMD_DEPTHWRITE_DISABLE \
						FDX8SHC( FDX8SHOP_DEPTHWRITE_DISABLE, 0 )

#define FDX8SHCMD_DEPTHWRITE_ENABLE \
						FDX8SHC( FDX8SHOP_DEPTHWRITE_ENABLE, 0 )


// FDX8SHCMD_SPECULAR_DISABLE
// FDX8SHCMD_SPECULAR_ENABLE
#define FDX8SHCMD_SPECULAR_DISABLE \
						FDX8SHC( FDX8SHOP_SPECULAR_DISABLE, 0 )

#define FDX8SHCMD_SPECULAR_ENABLE \
						FDX8SHC( FDX8SHOP_SPECULAR_ENABLE, 0 )


// FDX8SHCMD_FOGPASS_DISABLE
// FDX8SHCMD_FOGPASS_ENABLE
#define FDX8SHCMD_FOGPASS_DISABLE \
						FDX8SHC( FDX8SHOP_FOGPASS_DISABLE, 0 )

#define FDX8SHCMD_FOGPASS_ENABLE \
						FDX8SHC( FDX8SHOP_FOGPASS_ENABLE, 0 )


// FDX8SHCMD_LIGHTING_DISABLE
// FDX8SHCMD_LIGHTING_ENABLE
// FDX8SHCMD_LIGHTING_ENABLE_NOLIGHTS
#define FDX8SHCMD_LIGHTING_DISABLE \
						FDX8SHC( FDX8SHOP_LIGHTING_DISABLE, 0 )
#define FDX8SHCMD_LIGHTING_ENABLE \
						FDX8SHC( FDX8SHOP_LIGHTING_ENABLE, 0 )
#define FDX8SHCMD_LIGHTING_ENABLE_NOLIGHTS \
						FDX8SHC( FDX8SHOP_LIGHTING_ENABLE_NOLIGHTS, 0 )


// FDX8SHCMD_MTLSRCA( nMaterialSource )
// FDX8SHCMD_MTLSRCE( nMaterialSource )
// FDX8SHCMD_MTLSRCD( nMaterialSource )
// FDX8SHCMD_MTLSRCS( nMaterialSource )
#define FDX8SHCMD_MTLSRCA( nMaterialSource ) \
						FDX8SHC( FDX8SHOP_MTLSRCA, 1 ), \
						FDX8SHP_U32(nMaterialSource)

#define FDX8SHCMD_MTLSRCE( nMaterialSource ) \
						FDX8SHC( FDX8SHOP_MTLSRCE, 1 ), \
						FDX8SHP_U32(nMaterialSource)

#define FDX8SHCMD_MTLSRCD( nMaterialSource ) \
						FDX8SHC( FDX8SHOP_MTLSRCD, 1 ), \
						FDX8SHP_U32(nMaterialSource)

#define FDX8SHCMD_MTLSRCS( nMaterialSource ) \
						FDX8SHC( FDX8SHOP_MTLSRCS, 1 ), \
						FDX8SHP_U32(nMaterialSource)


// FDX8SHCMD_STAGES( nStageCount )
// FDX8SHCMD_TEXTURE( nD3DStageNum, nRegShTexInst, nRegTexCoordIndex )     (set nRegShTexInst to -1 to disable textures on this stage)
// FDX8SHCMD_COLORSTAGE( nD3DStageNum, nD3DColorOp, nD3DColorArg1, nD3DColorArg2 )
// FDX8SHCMD_ALPHASTAGE( nD3DStageNum, nD3DAlphaOp, nD3DAlphaArg1, nD3DAlphaArg2 )
// FDX8SHCMD_ARG0( nD3DStageNum, nD3DAlphaArg0, nD3DColorArg0 )
// FDX8SHCMD_RESULTARG( nD3DStageNum, nD3DResultArg )
#define FDX8SHCMD_STAGES( nStageCount ) \
						FDX8SHC( FDX8SHOP_STAGES, 1 ), \
						FDX8SHP_U32(nStageCount)

#define FDX8SHCMD_TEXTURE( nD3DStageNum, nRegShTexInst, nRegTexCoordIndex ) \
						FDX8SHC( FDX8SHOP_TEXTURE, 3 ), \
						FDX8SHP_U32(nD3DStageNum), \
						FDX8SHP_U32(nRegShTexInst), \
						FDX8SHP_U32(nRegTexCoordIndex)

#define FDX8SHCMD_ENVMAP_ENABLE( nStageCount ) \
						FDX8SHC( FDX8SHOP_ENVMAP_ENABLE, 1 ), \
						FDX8SHP_U32(nStageCount)

#define FDX8SHCMD_ENVMAP_DISABLE( nStageCount ) \
						FDX8SHC( FDX8SHOP_ENVMAP_DISABLE, 1 ), \
						FDX8SHP_U32(nStageCount)

#define FDX8SHCMD_COLORSTAGE( nD3DStageNum, nD3DColorOp, nD3DColorArg1, nD3DColorArg2 ) \
						FDX8SHC( FDX8SHOP_COLORSTAGE, 4 ), \
						FDX8SHP_U32(nD3DStageNum), \
						FDX8SHP_U32(nD3DColorOp), \
						FDX8SHP_U32(nD3DColorArg1), \
						FDX8SHP_U32(nD3DColorArg2)

#define FDX8SHCMD_ALPHASTAGE( nD3DStageNum, nD3DAlphaOp, nD3DAlphaArg1, nD3DAlphaArg2 ) \
						FDX8SHC( FDX8SHOP_ALPHASTAGE, 4 ), \
						FDX8SHP_U32(nD3DStageNum), \
						FDX8SHP_U32(nD3DAlphaOp), \
						FDX8SHP_U32(nD3DAlphaArg1), \
						FDX8SHP_U32(nD3DAlphaArg2)

#define FDX8SHCMD_ARG0( nD3DStageNum, nD3DAlphaArg0, nD3DColorArg0 ) \
						FDX8SHC( FDX8SHOP_ARG0, 3 ), \
						FDX8SHP_U32( nD3DStageNum ), \
						FDX8SHP_U32( nD3DAlphaArg0 ), \
						FDX8SHP_U32( nD3DColorArg0 )

#define FDX8SHCMD_RESULTARG( nD3DStageNum, nD3DResultArg ) \
						FDX8SHC( FDX8SHOP_RESULTARG, 2 ), \
						FDX8SHP_U32(nD3DStageNum), \
						FDX8SHP_U32(nD3DResultArg)


// FDX8SHCMD_SPECPOWER( fPower )
// FDX8SHCMD_TFACTOR( AlphaCode, ColorCode )
// FDX8SHCMD_AMBIENT( AlphaCode, ColorCode )
// FDX8SHCMD_MTLA( AlphaCode, ColorCode )
// FDX8SHCMD_MTLE( AlphaCode, ColorCode )
// FDX8SHCMD_MTLD( AlphaCode, ColorCode )
// FDX8SHCMD_MTLS( AlphaCode, ColorCode )
#define FDX8SHCMD_SPECPOWER( nRegPower ) \
						FDX8SHC( FDX8SHOP_SPECPOWER, 1 ), \
						FDX8SHP_U32( nRegPower )

#define FDX8SHCMD_TFACTOR( AlphaCode, ColorCode ) \
						FDX8SHC( FDX8SHOP_TFACTOR, 2 ), \
						FDX8SHP_U32( AlphaCode ), \
						FDX8SHP_U32( ColorCode )

#define FDX8SHCMD_AMBIENT( AlphaCode, ColorCode ) \
						FDX8SHC( FDX8SHOP_AMBIENT, 2 ), \
						FDX8SHP_U32( AlphaCode ), \
						FDX8SHP_U32( ColorCode )

#define FDX8SHCMD_MTLA( AlphaCode, ColorCode ) \
						FDX8SHC( FDX8SHOP_MTLA, 2 ), \
						FDX8SHP_U32( AlphaCode ), \
						FDX8SHP_U32( ColorCode )

#define FDX8SHCMD_MTLE( AlphaCode, ColorCode ) \
						FDX8SHC( FDX8SHOP_MTLE, 2 ), \
						FDX8SHP_U32( AlphaCode ), \
						FDX8SHP_U32( ColorCode )

#define FDX8SHCMD_MTLD( AlphaCode, ColorCode ) \
						FDX8SHC( FDX8SHOP_MTLD, 2 ), \
						FDX8SHP_U32( AlphaCode ), \
						FDX8SHP_U32( ColorCode )

#define FDX8SHCMD_MTLS( AlphaCode, ColorCode ) \
						FDX8SHC( FDX8SHOP_MTLS, 2 ), \
						FDX8SHP_U32( AlphaCode ), \
						FDX8SHP_U32( ColorCode )


// Defaults:
//   FDX8SHCMD_ALPHABLEND_DISABLE
//   FDX8SHCMD_ALPHATEST_DISABLE
//   FDX8SHCMD_DEPTHTEST_ENABLE( D3DCMP_LESSEQUAL )
//   FDX8SHOP_DEPTHWRITE_ENABLE
//   FDX8SHCMD_SPECULAR_DISABLE
//   FDX8SHCMD_FOGPASS_DISABLE
//   FDX8SHCMD_LIGHTING_ENABLE
//   FDX8SHCMD_MTLSRCA( D3DMCS_MATERIAL )
//   FDX8SHCMD_MTLSRCE( D3DMCS_COLOR1 )
//   FDX8SHCMD_MTLSRCD( D3DMCS_MATERIAL )
//   FDX8SHCMD_MTLSRCS( D3DMCS_MATERIAL )
//   FDX8SHCMD_TEXTURE( n, -1, 0 )   (texture disabled)
//   FDX8SHCMD_COLORSTAGE( 0, D3DTOP_SELECTARG1, D3DTA_DIFFUSE, D3DTA_CURRENT )
//   FDX8SHCMD_ALPHASTAGE( 0, D3DTOP_SELECTARG1, D3DTA_DIFFUSE, D3DTA_CURRENT )
//   FDX8SHCMD_COLORSTAGE( >0, D3DTOP_DISABLE, D3DTA_DIFFUSE, D3DTA_CURRENT )
//   FDX8SHCMD_ALPHASTAGE( >0, D3DTOP_DISABLE, D3DTA_DIFFUSE, D3DTA_CURRENT )
//   FDX8SHCMD_ARG0( n, D3DTA_CURRENT, D3DTA_CURRENT )
//   FDX8SHCMD_RESULTARG( n, D3DTA_CURRENT )
//   FDX8SHCMD_MTLA( FDX8SHCODE_K( 0.0f, 1.0f, 1.0f, 1.0f ) )
//   FDX8SHCMD_MTLE( FDX8SHCODE_K( 0.0f, 1.0f, 1.0f, 1.0f ) )
//   FDX8SHCMD_MTLD( FDX8SHCODE_K( 1.0f, 1.0f, 1.0f, 1.0f ) )
//   FDX8SHCMD_MTLS( FDX8SHCODE_K( 0.0f, 1.0f, 1.0f, 1.0f ) )
//   FDX8SHCMD_TFACTOR( FDX8SHCODE_K( 1.0f, 1.0f, 1.0f, 1.0f ) )
//   FDX8SHCMD_AMBIENT( FDX8SHCODE_K( 0.0f, 1.0f, 1.0f, 1.0f ) )
//


// Command Summary:
//  *FDX8SHCMD_ALPHABLEND_DISABLE
//   FDX8SHCMD_ALPHABLEND_ENABLE( nD3DSrcFactor, nD3DDestFactor, nD3DBlendOp )
//
//  *FDX8SHCMD_ALPHATEST_DISABLE
//   FDX8SHCMD_ALPHATEST_ENABLE( nD3DAlphaFunc, nD3DAlphaRef )
//
//   FDX8SHCMD_DEPTHTEST_DISABLE
//  *FDX8SHCMD_DEPTHTEST_ENABLE( nD3DDepthFunc )
//
//   FDX8SHOP_DEPTHWRITE_DISABLE
//  *FDX8SHOP_DEPTHWRITE_ENABLE
//
//  *FDX8SHCMD_SPECULAR_DISABLE
//   FDX8SHCMD_SPECULAR_ENABLE
//
//  *FDX8SHCMD_FOGPASS_DISABLE
//   FDX8SHCMD_FOGPASS_ENABLE
//
//   FDX8SHCMD_LIGHTING_DISABLE
//  *FDX8SHCMD_LIGHTING_ENABLE
//   FDX8SHCMD_LIGHTING_ENABLE_NOLIGHTS
//
//   FDX8SHCMD_MTLSRCA( nD3DMaterialColorSource )
//   FDX8SHCMD_MTLSRCE( nD3DMaterialColorSource )
//   FDX8SHCMD_MTLSRCD( nD3DMaterialColorSource )
//   FDX8SHCMD_MTLSRCS( nD3DMaterialColorSource )
//
//   FDX8SHCMD_TEXTURE( nD3DStageNum, nRegShTexInst, nRegTexCoordIndex )
//   FDX8SHCMD_COLORSTAGE( nD3DStageNum, nD3DColorOp, nD3DColorArg1, nD3DColorArg2 )
//   FDX8SHCMD_ALPHASTAGE( nD3DStageNum, nD3DAlphaOp, nD3DAlphaArg1, nD3DAlphaArg2 )
//   FDX8SHCMD_ARG0( nD3DStageNum, nD3DAlphaArg0, nD3DColorArg0 )
//   FDX8SHCMD_RESULTARG( nD3DStageNum, nD3DResultArg )
//
//   FDX8SHCMD_AMBIENT( AlphaCode, ColorCode )
//   FDX8SHCMD_MTLA( AlphaCode, ColorCode )
//   FDX8SHCMD_MTLE( AlphaCode, ColorCode )
//   FDX8SHCMD_MTLD( AlphaCode, ColorCode )
//   FDX8SHCMD_MTLS( AlphaCode, ColorCode )
//   FDX8SHCMD_TFACTOR( AlphaCode, ColorCode )
//
//   FDX8SHOP_NAME( pszShaderName )
//   FDX8SHOP_CALL( pszShaderName )
//   FDX8SHOP_METHOD
//   FDX8SHOP_PASS
//   FDX8SHOP_CHECK
//   FDX8SHOP_END



typedef struct {
	const FDX8Sh_t *pSh;			// Pointer to shader program
	cchar *pszName;					// Pointer to shader name
	FLinkRoot_t *pRootPass;			// Linklist of _DXPass_t's that make up this shader
	BOOL bCompiledOk;				// TRUE=compiled ok, FALSE=compilation failed (pRootPass is hooked to default)
	BOOL bApplyFogPass;				// TRUE=apply a separate fog pass for this shader

	u32 nNormalCount;				// Number of normal fields that this shader requires
	u32 nTCCount;					// Number of texture coordinate pairs that this shader requires
	u32 nColorCount;				// Number of color fields that this shader requires
	u32 nRegCount;					// Number of input registers this shader requires
} FDX8Shader_t;



extern FTexDef_t *FDX8Sh_pGradientTexDef;		// N-by-1 white gradient A8R8G8B8 texture

extern void fdx8sh_SetVertexType(u32 type);
FINLINE void fdx8sh_Stream1Set(BOOL bSet) { fdx8sh_bStream1Set = bSet; }

#endif

