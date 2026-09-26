//////////////////////////////////////////////////////////////////////////////////////
// PSPool.h - Point sprite pool.
//
// Author: Michael Starich   
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
// 03/30/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _PSPOOL_H_
#define _PSPOOL_H_ 1

#include "fang.h"
#include "fpsprite.h"

#define PS_POOL_COUNT			1000


extern BOOL pspool_InitSystem( void );
extern void pspool_UninitSystem( void );
extern FPSprite_t *pspool_GetArray( u32 nNumPSs );
extern void pspool_ReturnArray( FPSprite_t *pPS );

#endif

