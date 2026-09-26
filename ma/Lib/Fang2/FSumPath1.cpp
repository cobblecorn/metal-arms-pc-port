//////////////////////////////////////////////////////////////////////////////////////
// FSumPath1.cpp - Class for sum-based CFVec3 motion object in Fang.
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

#include "FSumPath1.h"

// =============================================================================================================

u32 CFV3OSumPath1::m_uBufPos = 0;
u32 CFV3OSumPath1::m_uNumInPool = 0;
//CFV3OSumPath1 CFV3OSumPath1::m_aoPool[10];
static CFV3OSumPath1 s_aoPool[10];

// =============================================================================================================

CFV3OSumPath1::CFV3OSumPath1()
{
	m_pV3O1 = NULL;
	m_pV3O2 = NULL;
}

// =============================================================================================================

CFV3OSumPath1 *CFV3OSumPath1::GetAvailable()
{
	CFV3OSumPath1 *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	if(m_uBufPos == 10)
		m_uBufPos = 0;
	FASSERT(m_uBufPos < 10);

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OSumPath1::Init(CFVec3AObj *pV3O1, CFVec3AObj *pV3O2)
{
	m_pV3O1 = pV3O1;
	m_pV3O2 = pV3O2;

	m_uFlags |= FLAG_INITTED;

	return(TRUE);
}

// =============================================================================================================

void CFV3OSumPath1::Work()
{
	m_vecValue = m_pV3O1->GetValue();
	m_vecValue.Add(m_pV3O2->GetValue());
//	m_vecValue += m_pV3O2->GetValue();
}

// =============================================================================================================
