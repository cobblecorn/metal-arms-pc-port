//////////////////////////////////////////////////////////////////////////////////////
// FQuatSLERP.cpp - 
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
// 07/31/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "FQuatSLERP.h"
#include "floop.h"

// =============================================================================================================

static const u32 CFQuatSLERP_uPoolSize = 10;
u32 CFQuatSLERP::m_uBufPos = 0;
u32 CFQuatSLERP::m_uNumInPool = 0;
CFQuatSLERP FQuatSLERP_aoPool[CFQuatSLERP_uPoolSize];

// =============================================================================================================

CFQuatSLERP::CFQuatSLERP()
{
	// These initializations shouldn't need to be here, and they probably don't.
	// Just being safe.
	m_qValue.Identity();
	m_fCurTimePos = 0.0f;
	m_fOOTotalTime = 0.0f;
}

// =============================================================================================================

CFQuatSLERP *CFQuatSLERP::GetAvailable()
{
	if(m_uBufPos == CFQuatSLERP_uPoolSize)
	{
		return(NULL);
	}
	CFQuatSLERP *poQO = &(FQuatSLERP_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatSLERP_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatSLERP::Init(CFQuatA *pQuat1, CFQuatA *pQuat2, f32 fTotalTime, BOOL bAutoStart/* = TRUE*/)
{
	FASSERT(pQuat1 != NULL);
	FASSERT(pQuat2 != NULL);
	FASSERT(fTotalTime >= 0.0f);

	m_fTotalTime = fTotalTime;
	m_fOOTotalTime = 1.0f / fTotalTime;
	m_qStart = *pQuat1;
	m_qEnd = *pQuat2;

	m_qValue = *pQuat1;
	m_fCurTimePos = 0.0f;

	m_uFlags |= FLAG_INITTED;
	Start();

	return(TRUE);
}

// =============================================================================================================

void CFQuatSLERP::Work()
{
	FASSERT(IsWorking());

	m_fCurTimePos += FLoop_fPreviousLoopSecs;

	if(m_fCurTimePos < m_fTotalTime)
	{
		Calculate();
	}
	else
	{
		m_fCurTimePos = m_fTotalTime;
		m_qValue = m_qEnd;
	}
}

// =============================================================================================================

void CFQuatSLERP::Calculate()
{
	// JUSTIN: Change this.
	f32 fUnitPos = m_fCurTimePos * m_fOOTotalTime;
	m_qValue.ReceiveSlerpOf(fUnitPos, m_qStart, m_qEnd);
}

// =============================================================================================================
