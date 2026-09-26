//////////////////////////////////////////////////////////////////////////////////////
// FV3OConst.cpp - Class for constant CFVec3 motion object in Fang.
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

#include "FV3OConst.h"

// =============================================================================================================

u32 CFV3OConst::m_uBufPos = 0;
u32 CFV3OConst::m_uNumInPool = 0;
//CFV3OConst CFV3OConst::m_aoPool[10];
static CFV3OConst s_aoPool[10];

// =============================================================================================================

CFV3OConst::CFV3OConst()
{
}

// =============================================================================================================

CFV3OConst *CFV3OConst::GetAvailable()
{
	CFV3OConst *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	if(m_uBufPos == 10)
		m_uBufPos = 0;
	FASSERT(m_uBufPos < 10);

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OConst::Init(CFVec3A &vecValue, BOOL bAutoStart/* = TRUE*/)
{
	m_vecValue = vecValue;
//	m_bIsActive = TRUE;

	m_uFlags |= FLAG_INITTED;

	return(TRUE);
}

// =============================================================================================================

BOOL CFV3OConst::Init(f32 fX, f32 fY, f32 fZ, BOOL bAutoStart/* = TRUE*/)
{
	m_vecValue.x = fX;
	m_vecValue.y = fY;
	m_vecValue.z = fZ;
//	m_bIsActive = TRUE;

	m_uFlags |= FLAG_INITTED;

	return(TRUE);
}

// =============================================================================================================

void CFV3OConst::Work()
{
}

// =============================================================================================================
