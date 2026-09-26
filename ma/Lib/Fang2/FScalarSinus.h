//////////////////////////////////////////////////////////////////////////////////////
// FScalarSinus.h - Sinusoid scalar object.
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
#ifndef _FSCALARSINUS_H_
#define _FSCALARSINUS_H_ 1

// =============================================================================================================

#include "FScalarObj.h"

// =============================================================================================================

static const u32 CFScalarSinus_uPoolSize = 10;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFScalarSinus : public CFScalarObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFScalarSinus();

	void Reset(BOOL bAutoStart = TRUE) { Init(m_fFreq, m_fAmp, m_fPhaseX, m_fPhaseY, bAutoStart); }
	void SetTimePos(f32 fNewTime) {}

	static CFScalarSinus *GetAvailable();
	BOOL Init(f32 fFreq, f32 fAmp, f32 fPhaseX, f32 fPhaseY, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	f32 m_fFreq, m_fAmp, m_fPhaseX, m_fPhaseY;
	f32 m_fTimer;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFScalarSinus m_aoPool[CFScalarSinus_uPoolSize];

	FCLASS_STACKMEM_NOALIGN(CFScalarSinus);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
