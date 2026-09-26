//////////////////////////////////////////////////////////////////////////////////////
// FScalarBlend.h - Blending scalar object.
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
#ifndef _FSCALARBLEND_H_
#define _FSCALARBLEND_H_ 1

// =============================================================================================================

#include "FScalarObj.h"

// =============================================================================================================

static const u32 CFScalarBlend_uPoolSize = 20;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFScalarBlend : public CFScalarObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFScalarBlend();

	void Reset(BOOL bAutoStart) { Init(m_fStart, m_fEnd, m_fDuration, bAutoStart); }
	void SetTimePos(f32 fNewTime) {}

	static CFScalarBlend *GetAvailable();
	BOOL Init(f32 fStart, f32 fEnd, f32 fDuration, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	f32 m_fStart, m_fEnd, m_fdFdt, m_fDuration/*, m_fOODuration*/;
	f32 m_fTimer;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFScalarBlend m_aoPool[CFScalarBlend_uPoolSize];

	FCLASS_STACKMEM_NOALIGN(CFScalarBlend);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
