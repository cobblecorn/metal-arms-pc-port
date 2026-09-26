//////////////////////////////////////////////////////////////////////////////////////
// fdx8xfm.h - Fang Transformation module.
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

#ifndef _FDX8XFM_H_
#define _FDX8XFM_H_ 1

#include "fang.h"
#include "fxfm.h"
#include "fdx8.h"
#include "d3dx8.h"

extern BOOL fdx8xfm_ModuleStartup( void );
extern void fdx8xfm_ModuleShutdown( void );

extern void fdx8xfm_SetRenormalizeNormalsIfNeeded( f32 fMtxScale );
extern void fdx8xfm_SetRenormalizeNormalsIfNeeded( const CFMtx43A *pMtx );

extern void fdx8xfm_SetDXMatrices( BOOL bUsingVertexShaders );
extern void fdx8xfm_SetModelDXMatrix( BOOL bUsingVertexShaders );
extern void fdx8xfm_SetViewDXMatrix( BOOL bUsingVertexShaders );
extern void fdx8xfm_SetCustomDXMatrix( D3DTRANSFORMSTATETYPE nD3DMatrixType, const CFMtx43A *pMtx43A, BOOL bUsingVertexShaders );
extern void fdx8xfm_SetCustomDXMatrix( D3DTRANSFORMSTATETYPE nD3DMatrixType, const CFMtx44A *pMtx44A, BOOL bUsingVertexShaders );
extern void fdx8xfm_SetCustomDXTextureMatrix( u32 nTexMtxIndex, const CFMtx43 *pMtx43 );

#endif

