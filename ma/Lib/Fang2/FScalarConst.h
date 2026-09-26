//////////////////////////////////////////////////////////////////////////////////////
// FScalarConst.h - Constant scalar object.
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
// 04/30/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FSCALARCONST_H_
#define _FSCALARCONST_H_ 1

// =============================================================================================================

#include "FScalarObj.h"

// =============================================================================================================

static const u32 CFScalarConst_uPoolSize = 10;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFScalarConst : public CFScalarObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFScalarConst();

	void Reset(BOOL bAutoStart = TRUE) { Init(m_fValue, bAutoStart); }
	void SetTimePos(f32 fNewTime) {}

	static CFScalarConst *GetAvailable();
	BOOL Init(f32 fValue, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFScalarConst m_aoPool[CFScalarConst_uPoolSize];

	FCLASS_STACKMEM_NOALIGN(CFScalarConst);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
