//////////////////////////////////////////////////////////////////////////////////////
// FMulPath1.h - Class for mult-based CFVec3 motion object in Fang.
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
#ifndef _FMULPATH1_H_
#define _FMULPATH1_H_ 1

// =============================================================================================================

#include "FVec3Obj.h"

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFV3OMulPath1 : public CFVec3AObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFV3OMulPath1();

	void Reset(BOOL bAutoStart) {};
	void SetTimePos(f32 fNewTime) {}

	static CFV3OMulPath1 *GetAvailable();
	BOOL Init(f32 fFactor, CFVec3AObj *pV3O);
	void Work();
	void Release();

protected:
	CFVec3AObj *m_pV3O;
	f32 m_fFactor;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFV3OMulPath1 m_aoPool[10];

	FCLASS_STACKMEM_NOALIGN(CFV3OMulPath1);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
