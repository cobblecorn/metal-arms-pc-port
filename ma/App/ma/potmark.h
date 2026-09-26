//////////////////////////////////////////////////////////////////////////////////////
// potmark.h - 
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
// 08/29/01 Ranck		Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef _POTMARK_H_
#define _POTMARK_H_ 1

#include "fang.h"
#include "fviewport.h"
#include "fmath.h"

enum PotMarkType_e
{
	POTMARKTYPE_LASER1,
	POTMARKTYPE_LASER2,
	POTMARKTYPE_RIVET_EUK2,
	POTMARKTYPE_BLASTER_L1,
	POTMARKTYPE_BULLET,

	POTMARKTYPE_COUNT
};

extern BOOL potmark_InitSystem( void ) { return TRUE; };
extern void potmark_UninitSystem( void ) {};
extern void potmark_KillAll( void ) {};
extern void potmark_NewPotmark( const CFVec3A *pCenterPos_WS, const CFVec3A *pUnitNormal_WS, f32 fSize = 0.375f, PotMarkType_e eType = POTMARKTYPE_LASER1 ) {};
extern void potmark_Draw( void ) {};
extern void potmark_Work( void ) {};

#endif

#endif //0
