//////////////////////////////////////////////////////////////////////////////////////
// MiscDraw.h
//
// Author: Michael Scholz
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
// 11/26/02 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _MISCDRAW_H_
#define _MISCDRAW_H_ 1

#include "fdraw.h"

FCLASS_NOALIGN_PREFIX class CDebugDraw
{
public:
#if !FANG_PRODUCTION_BUILD
	FINLINE static void Sphere(const CFVec3A& CenterPt,f32 fRadius_WS, CFColorRGBA *pVtxColorRGBA=&FColor_MotifGreen, u32 nSliceCountPerHalf=3, u32 nRingCountPerQuarter=3, u32 nVtxCountPerQuarter=3 )
	{
		fdraw_DevSphere(&CenterPt.v3,fRadius_WS,pVtxColorRGBA,nRingCountPerQuarter,nVtxCountPerQuarter);
	}
	FINLINE static void Line(const CFVec3A& vBeginPt,const CFVec3A& vEndPt, CFColorRGBA* pBeginColor=&FColor_MotifGreen,CFColorRGBA* pEndColor=&FColor_MotifGreen)
	{
		fdraw_DevLine( &vBeginPt.v3, &vEndPt.v3, pBeginColor, pEndColor );
	}
#else
	FINLINE static void Sphere(const CFVec3A& CenterPt,f32 fRadius_WS, CFColorRGBA *pVtxColorRGBA=&FColor_MotifGreen, u32 nSliceCountPerHalf=3, u32 nRingCountPerQuarter=3, u32 nVtxCountPerQuarter=3 )
	{}
	FINLINE static void Line(const CFVec3A& vBeginPt,const CFVec3A& vEndPt, CFColorRGBA* pBeginColor=&FColor_MotifGreen,CFColorRGBA* pEndColor=&FColor_MotifGreen)
	{}
#endif 
}FCLASS_NOALIGN_SUFFIX;

#endif