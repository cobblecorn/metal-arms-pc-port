//////////////////////////////////////////////////////////////////////////////////////
// FMulPath1.cpp - Class for mult-based CFVec3 motion object in Fang.
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

#include "FMulPath1.h"

// =============================================================================================================

u32 CFV3OMulPath1::m_uBufPos = 0;
u32 CFV3OMulPath1::m_uNumInPool = 0;
//CFV3OMulPath1 CFV3OMulPath1::m_aoPool[10];
static CFV3OMulPath1 s_aoPool[10];

// =============================================================================================================

CFV3OMulPath1::CFV3OMulPath1()
{
}

// =============================================================================================================

CFV3OMulPath1 *CFV3OMulPath1::GetAvailable()
{
	CFV3OMulPath1 *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	if(m_uBufPos == 10)
		m_uBufPos = 0;
	FASSERT(m_uBufPos < 10);

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OMulPath1::Init(f32 fFactor, CFVec3AObj *pV3O)
{
	m_fFactor = fFactor;
	m_pV3O = pV3O;

	m_uFlags |= FLAG_INITTED;

	return(TRUE);
}

// =============================================================================================================

void CFV3OMulPath1::Work()
{
	m_vecValue = m_pV3O->GetValue();
	m_vecValue.Mul(m_fFactor);
//	m_vecValue *= m_fFactor;
}

// =============================================================================================================
