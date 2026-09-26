//////////////////////////////////////////////////////////////////////////////////////
// FV3OConst.h - Class for constant CFVec3 motion object in Fang.
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
#ifndef _FV3OCONST_H_
#define _FV3OCONST_H_ 1

// =============================================================================================================

#include "FVec3Obj.h"

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFV3OConst : public CFVec3AObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFV3OConst();

	void Reset(BOOL bAutoStart) { Init(m_vecValue, bAutoStart); }
	void SetTimePos(f32 fNewTime) {};

	static CFV3OConst *GetAvailable();
	BOOL Init(CFVec3A &vecValue, BOOL bAutoStart = TRUE);
	BOOL Init(f32 fX, f32 fY, f32 fZ, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFV3OConst m_aoPool[10];

	FCLASS_STACKMEM_NOALIGN(CFV3OConst);
} FCLASS_ALIGN_SUFFIX;

#endif
