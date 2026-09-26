//////////////////////////////////////////////////////////////////////////////////////
// fvtxpool.h - 
//
// Author: Michael Starich / Russell Foushee   
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
// 01/28/03 Foushee		Created and used VtxPool as a basis 
// 03/30/02 Starich     VtxPool Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FVTXPOOL_H_
#define _FVTXPOOL_H_ 1

#include "fang.h"

//#define VTX_POOL_COUNT			2500

struct FDrawVtx_t;

extern BOOL fvtxpool_ModuleStartup( void );
extern void fvtxpool_ModuleShutdown( void );
extern FDrawVtx_t *fvtxpool_GetArray( u32 nNumVerts, BOOL bSuppressNotEnoughVerticesWarning = FALSE );
extern void fvtxpool_ReturnArray( FDrawVtx_t *pVtx );

#endif
















