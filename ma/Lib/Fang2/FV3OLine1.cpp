//////////////////////////////////////////////////////////////////////////////////////
// FV3OLine1.cpp - Class for line-based CFVec3 motion object in Fang.
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

#include "FV3OLine1.h"
#include "floop.h"

// =============================================================================================================

u32 CFV3OLine1::m_uBufPos = 0;
u32 CFV3OLine1::m_uNumInPool = 0;
//CFV3OLine1 CFV3OLine1::m_aoPool[10];
CFV3OLine1 FV3OLine1_aoPool[10];

// =============================================================================================================

CFV3OLine1::CFV3OLine1()
{
}

// =============================================================================================================

CFV3OLine1 *CFV3OLine1::GetAvailable()
{
//	CFV3OLine1 *poSO = &(m_aoPool[m_uBufPos]);
	CFV3OLine1 *poSO = &(FV3OLine1_aoPool[m_uBufPos]);

	++m_uBufPos;
	if(m_uBufPos == 10)
		m_uBufPos = 0;
	FASSERT(m_uBufPos < 10);

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OLine1::Init(CFVec3A &vecStart, CFVec3A &vecEnd, f32 fTotalTime, BOOL bAutoStart/* = TRUE*/)
{
	m_vecStart = vecStart;
	m_vecEnd = vecEnd;
	m_fTotalTime = fTotalTime;
	m_fOOTotalTime = 1.0f / fTotalTime;
	m_fTimer = 0.0f;
	Calculate();

//	AddToWorkList(this);
//	m_bIsActive = TRUE;
	m_uFlags |= FLAG_INITTED;

	Start();

	return(TRUE);
}

// =============================================================================================================

// =============================================================================================================

void CFV3OLine1::SetTimePos(f32 fNewTime)
{
	m_fTimer = fNewTime;
	Calculate();
}

// =============================================================================================================

void CFV3OLine1::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;
	if(m_fTimer >= m_fTotalTime)
	{
		m_fTimer = m_fTotalTime;
//		RemoveFromWorkList(this);
		End();
	}
	Calculate();
}

// =============================================================================================================


void CFV3OLine1::Calculate()
{
	FASSERT(m_fTimer >= 0.0f);
	FASSERT(m_fTimer <= m_fTotalTime);

	f32 fUnitTime = m_fTimer * m_fOOTotalTime;

	f32 fWeight1 = 1.0f - fUnitTime;
	f32 fWeight2 = fUnitTime;

	m_vecValue = m_vecStart;
	m_vecValue.Mul(fWeight1);

	CFVec3A vecTemp = m_vecEnd;
	vecTemp.Mul(fWeight2);

	m_vecValue.Add(vecTemp);
}

// =============================================================================================================
