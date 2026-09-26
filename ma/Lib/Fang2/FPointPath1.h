//////////////////////////////////////////////////////////////////////////////////////
// FPointPath1.h - CFVec3Obj defining a path from an ordered set of points.
//
// Author: Justin Link
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
// 04/03/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FPOINTPATH1_H_
#define _FPOINTPATH1_H_ 1

// =============================================================================================================

#include "fang.h"
#include "FVec3Obj.h"

// =============================================================================================================

const u32 FPointPath1_uMaxPts = 20;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFV3OPointPath1 : public CFVec3AObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit();// { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFV3OPointPath1();

	// TODO: This should get changed to be Reset().
	void Reset(BOOL bAutoStart) { /*Init(bAutoStart);*/ }
	void SetTimePos(f32 fNewTime) { FASSERT_NOW; }

	static CFV3OPointPath1 *GetAvailable();
	BOOL Init(const CFVec3A *aPtList, u32 uNumPts, f32 fTotalTime, BOOL bRestAtEnds, BOOL bAutoStart/* = TRUE*/);
	void Work();
	void Release();

	static CFV3OPointPath1 *GetPool( void );
	static s32 GetPoolNumEntries( void );

protected:
	f32 m_fTimer, m_fTotalTime;
	u32 m_uNumPts, m_uNumSegs;
	CFVec3A m_avecCoeff[FPointPath1_uMaxPts + 2];

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFV3OPointPath1 m_aoPool[10];

	static void PrintMtx(f32 *afMtx, u32 uNumCols, u32 uNumRows);

	FCLASS_STACKMEM_NOALIGN(CFV3OPointPath1);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
