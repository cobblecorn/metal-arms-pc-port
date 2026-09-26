//////////////////////////////////////////////////////////////////////////////////////
// FScalarBlend.cpp - Blending scalar object.
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

#include "FScalarBlend.h"
#include "floop.h"

// =============================================================================================================

u32 CFScalarBlend::m_uBufPos = 0;
u32 CFScalarBlend::m_uNumInPool = 0;
//CFScalarBlend CFScalarBlend::m_aoPool[CFScalarBlend_uPoolSize];
static CFScalarBlend s_aoPool[CFScalarBlend_uPoolSize];

// =============================================================================================================

CFScalarBlend::CFScalarBlend()
{
}

// =============================================================================================================

CFScalarBlend *CFScalarBlend::GetAvailable()
{
	if(m_uBufPos == CFScalarBlend_uPoolSize)
	{
		return(NULL);
	}
	CFScalarBlend *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFScalarBlend_uPoolSize);

	return(poSO);
}

// =============================================================================================================

BOOL CFScalarBlend::Init(f32 fStart, f32 fEnd, f32 fDuration, BOOL bAutoStart/* = TRUE*/)
{
//	m_bIsActive = TRUE;

	m_fStart = fStart;
	m_fEnd = fEnd;
	m_fdFdt = (fEnd - fStart) / fDuration;
	m_fDuration = fDuration;

	m_fTimer = 0.0f;

	if(bAutoStart)
	{
		Start();
	}

	m_fValue = fStart;

	return(TRUE);
}

// =============================================================================================================

void CFScalarBlend::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;
	if(m_fTimer < m_fDuration)
	{
		m_fValue = m_fStart + m_fdFdt * m_fTimer;
	}
	else
	{
		End();
//		RemoveFromWorkList(this);
		m_fValue = m_fEnd;
	}
}

// =============================================================================================================