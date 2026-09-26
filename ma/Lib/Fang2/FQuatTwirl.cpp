//////////////////////////////////////////////////////////////////////////////////////
// FQuatTwirl.cpp - Quaternion object that rotates continuously about an axis.
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
// 04/29/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FQuatTwirl.h"
#include "floop.h"

// =============================================================================================================

u32 CFQuatTwirl::m_uBufPos = 0;
u32 CFQuatTwirl::m_uNumInPool = 0;
//CFQuatTwirl CFQuatTwirl::m_aoPool[CFQuatTwirl_uPoolSize];
static CFQuatTwirl s_aoPool[CFQuatTwirl_uPoolSize];

// =============================================================================================================

CFQuatTwirl::CFQuatTwirl()
{
}

// =============================================================================================================

CFQuatTwirl *CFQuatTwirl::GetAvailable()
{
	if(m_uBufPos == CFQuatTwirl_uPoolSize)
	{
		return(NULL);
	}
	CFQuatTwirl *poQO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatTwirl_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatTwirl::Init(CFQuatA &qInit, CFVec3A &vecAxis, f32 fOmega, BOOL bAutoStart/* = TRUE*/)
{
	m_qValue = qInit;
	m_qInit = qInit;
	m_vecAxis = vecAxis;
	m_fOmega = fOmega;
	m_fTimer = 0.0f;

	m_uFlags |= FLAG_INITTED;
	Start();

	return(TRUE);
}

// =============================================================================================================

void CFQuatTwirl::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;
	f32 fThetaY = m_fOmega * m_fTimer;

	CFQuatA qTemp;
	qTemp.BuildQuat(m_vecAxis, fThetaY);

	m_qValue = m_qInit;
	m_qValue.Mul(qTemp);
}

// =============================================================================================================
