//////////////////////////////////////////////////////////////////////////////////////
// FV3OCircleXZ.cpp - Class for circular-xz CFVec3 motion object in Fang.
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

#include "FV3OCircleXZ.h"
#include "floop.h"

// =============================================================================================================

u32 CFV3OCircleXZ::m_uBufPos = 0;
u32 CFV3OCircleXZ::m_uNumInPool = 0;
//CFV3OCircleXZ CFV3OCircleXZ::m_aoPool[10];
static CFV3OCircleXZ s_aoPool[10];

// =============================================================================================================

CFV3OCircleXZ::CFV3OCircleXZ()
{
}

// =============================================================================================================

CFV3OCircleXZ *CFV3OCircleXZ::GetAvailable()
{
	CFV3OCircleXZ *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	if(m_uBufPos == 10)
		m_uBufPos = 0;
	FASSERT(m_uBufPos < 10);

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OCircleXZ::Init(CFVec3A &vecCenter, f32 fRadius, f32 fOmegaY, BOOL bAutoStart/* = TRUE*/)
{
	m_vecCenter = vecCenter;
	m_fRadius = fRadius;
	m_fOmegaY = fOmegaY;

	m_vecValue = vecCenter;
	m_vecValue.x += fRadius;

//	AddToWorkList(this);
//	m_bIsActive = TRUE;
	m_uFlags |= FLAG_INITTED;

	Start();

	return(TRUE);
}

// =============================================================================================================

void CFV3OCircleXZ::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;

	f32 fTheta = m_fOmegaY * m_fTimer;
	CFVec3A vecTemp;
	fmath_SinCos(fTheta, &(vecTemp.z), &(vecTemp.x));
	vecTemp.y = 0.0f;
	vecTemp.Mul(m_fRadius);

	m_vecValue = m_vecCenter;
	m_vecValue.Add(vecTemp);
}

// =============================================================================================================
