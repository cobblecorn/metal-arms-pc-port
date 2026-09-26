//////////////////////////////////////////////////////////////////////////////////////
// fGCdraw.h - Fang drawing utility module (GameCube version).
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
// 02/18/02 Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGCDRAW_H_
#define _FGCDRAW_H_ 1

#include "fang.h"
#include "fdraw.h"

struct FTexDef_t;

extern BOOL fgcdraw_ModuleStartup( void );
extern void fgcdraw_ModuleShutdown( void );

extern void fgcdraw_SetCullDir( FDrawCullDir_e nCullDir );
extern void fgcdraw_EnableClipping( BOOL bEnable );
extern void fgcdraw_Color_SetFunc( FDrawColorFunc_e nColorFunc );
extern void fgcdraw_SetTexture( FTexDef_t *pTexDef );
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
extern BOOL fgcdraw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
extern BOOL fgcdraw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp,
									  FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp );
extern void fgcdraw_Renderer_Open( void );
extern void fgcdraw_Renderer_Close( void );


#endif

