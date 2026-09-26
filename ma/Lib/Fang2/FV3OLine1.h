//////////////////////////////////////////////////////////////////////////////////////
// FV3OLine1.h - Class for line-based CFVec3 motion object in Fang.
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
// 04/10/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FV3OLINE1_H_
#define _FV3OLINE1_H_ 1

// =============================================================================================================

#include "FVec3Obj.h"

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFV3OLine1 : public CFVec3AObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFV3OLine1();

	static CFV3OLine1 *GetAvailable();
	BOOL Init(CFVec3A &vecStart, CFVec3A &vecEnd, f32 fTotalTime, BOOL bAutoStart = TRUE);

	void Reset(BOOL bAutoStart) { Init(m_vecStart, m_vecEnd, m_fTotalTime, bAutoStart); }
	void SetTimePos(f32 fNewTime);

	void Work();
	void Release();

protected:
	void Calculate();

	CFVec3A m_vecStart, m_vecEnd;
	f32 m_fTotalTime, m_fOOTotalTime;
	f32 m_fTimer;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFV3OLine1 m_aoPool[10];

	FCLASS_STACKMEM_NOALIGN(CFV3OLine1);
} FCLASS_ALIGN_SUFFIX;

#endif
