//////////////////////////////////////////////////////////////////////////////////////
// FQuatConst.cpp - Const quaternion object.
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

#include "FQuatConst.h"

// =============================================================================================================

u32 CFQuatConst::m_uBufPos = 0;
u32 CFQuatConst::m_uNumInPool = 0;
//CFQuatConst CFQuatConst::m_aoPool[CFQuatConst_uPoolSize];
static CFQuatConst s_aoPool[CFQuatConst_uPoolSize];

// =============================================================================================================

CFQuatConst::CFQuatConst()
{
}

// =============================================================================================================

CFQuatConst *CFQuatConst::GetAvailable()
{
	if(m_uBufPos == CFQuatConst_uPoolSize)
	{
		return(NULL);
	}
	CFQuatConst *poQO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatConst_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatConst::Init(CFQuatA &qValue, BOOL bAutoStart/* = TRUE*/)
{
	m_qValue = qValue;

	return(TRUE);
}

// =============================================================================================================

BOOL CFQuatConst::Init(f32 fW, f32 fX, f32 fY, f32 fZ, BOOL bAutoStart/* = TRUE*/)
{
	m_qValue.w = fW;
	m_qValue.x = fX;
	m_qValue.y = fY;
	m_qValue.z = fZ;

	return(TRUE);
}

// =============================================================================================================

void CFQuatConst::Work()
{
}

// =============================================================================================================
