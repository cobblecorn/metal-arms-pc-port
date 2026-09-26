//////////////////////////////////////////////////////////////////////////////////////
// FScalarConst.cpp - Constant scalar object.
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

#include "FScalarConst.h"

// =============================================================================================================

u32 CFScalarConst::m_uBufPos = 0;
u32 CFScalarConst::m_uNumInPool = 0;
//CFScalarConst CFScalarConst::m_aoPool[CFScalarConst_uPoolSize];
static CFScalarConst s_aoPool[CFScalarConst_uPoolSize];

// =============================================================================================================

CFScalarConst::CFScalarConst()
{
}

// =============================================================================================================

CFScalarConst *CFScalarConst::GetAvailable()
{
	if(m_uBufPos == CFScalarConst_uPoolSize)
	{
		return(NULL);
	}
	CFScalarConst *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFScalarConst_uPoolSize);

	return(poSO);
}

// =============================================================================================================

BOOL CFScalarConst::Init(f32 fValue, BOOL bAutoStart/* = TRUE*/)
{
	m_fValue = fValue;
//	m_bIsActive = TRUE;

	return(TRUE);
}

// =============================================================================================================

void CFScalarConst::Work()
{
}

// =============================================================================================================
