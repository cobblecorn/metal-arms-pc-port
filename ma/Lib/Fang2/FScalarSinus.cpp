//////////////////////////////////////////////////////////////////////////////////////
// FScalarSinus.cpp - Sinusoid scalar object.
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

#include "FScalarSinus.h"
#include "floop.h"

// =============================================================================================================

u32 CFScalarSinus::m_uBufPos = 0;
u32 CFScalarSinus::m_uNumInPool = 0;
//CFScalarSinus CFScalarSinus::m_aoPool[CFScalarSinus_uPoolSize];
static CFScalarSinus s_aoPool[CFScalarSinus_uPoolSize];

// =============================================================================================================

CFScalarSinus::CFScalarSinus()
{
}

// =============================================================================================================

CFScalarSinus *CFScalarSinus::GetAvailable()
{
	if(m_uBufPos == CFScalarSinus_uPoolSize)
	{
		return(NULL);
	}
	CFScalarSinus *poQO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFScalarSinus_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFScalarSinus::Init(f32 fFreq, f32 fAmp, f32 fPhaseX, f32 fPhaseY, BOOL bAutoStart/* = TRUE*/)
{
	m_fFreq = fFreq;
	m_fAmp = fAmp;
	m_fPhaseX = fPhaseX;
	m_fPhaseY = fPhaseY;

	m_fTimer = 0.0f;

	if(bAutoStart)
	{
		Start();
	}

	m_fValue = fAmp * fmath_Sin(fPhaseX) + fPhaseY;

	return(TRUE);
}

// =============================================================================================================

void CFScalarSinus::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;
	m_fValue = m_fAmp * fmath_Sin(m_fFreq * m_fTimer + m_fPhaseX) + m_fPhaseY;
}

// =============================================================================================================
