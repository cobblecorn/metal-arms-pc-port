//////////////////////////////////////////////////////////////////////////////////////
// FPointPath2.h - CFVec3Obj defining a path from an ordered set of points.
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
// 04/04/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FPOINTPATH2_H_
#define _FPOINTPATH2_H_ 1

#include "fang.h"
#include "FVec3Obj.h"

// Changeable constants.
const u32 FPointPath2_uMaxPts = 256;

// Derived constants (do not change these directly!)
const u32 FPointPath2_uMaxSegs = FPointPath2_uMaxPts - 1;

FCLASS_ALIGN_PREFIX class CFV3OPointPath2 : public CFVec3AObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit();

	static void LevelUninit() {}

	CFV3OPointPath2();
	~CFV3OPointPath2();

	void Reset(BOOL bAutoStart = TRUE) { SetTimePos(0.0f);  if( bAutoStart ) { Start(); } }
	void SetTimePos(f32 fNewTime);
	f32 GetTimePos() { return(m_fTimePos); }
	f32 GetTotalTime() { return(m_fTotalTime); }

	static CFV3OPointPath2 *GetAvailable();
	BOOL Init(const char *pszPathName, f32 fTotalTime, BOOL bAutoStart = TRUE); //THIS LOOKS TO BE AN INVALID CALL -- RAF
	BOOL Init(const CFVec3A *aPtList, u32 uNumPts, f32 fTotalTime, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

	static CFV3OPointPath2 *GetPool( void );
	static s32 GetPoolNumEntries( void );

protected:
	void GetCoeffFromPtList(const CFVec3A *aPtList, u32 uNumPts, f32 fTotalTime);
	void Calculate();
	void CalcPos();
	void CalcVel();
	void CalcTang();
	void CalcAccel();

	f32 m_fTimer, m_fTotalTime;
	f32 m_fTimePos;				// This is relative to the whole path.
	u32 m_uNumPts, m_uNumSegs;
	f32 m_afSegTime[FPointPath2_uMaxSegs];
//	BOOL m_bInitted;

	FCLASS_ALIGN_PREFIX class CPathCoeff
	{
	public:
		CFVec3A m_avecCoeff[4];

		FCLASS_STACKMEM_ALIGN(CPathCoeff);
	} FCLASS_ALIGN_SUFFIX;

//	CFVec3A m_avecCoeff[FPointPath2_uMaxSegs][4];
	CPathCoeff *m_pavecCoeff;
	u32 m_uCurSeg;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFV3OPointPath2 m_aoPool[50];

	FCLASS_STACKMEM_ALIGN(CFV3OPointPath2);
} FCLASS_ALIGN_SUFFIX;

#endif