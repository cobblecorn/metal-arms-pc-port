//////////////////////////////////////////////////////////////////////////////////////
// FQuatComp.cpp - Class for composition of quaternion objects.
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
// 04/22/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FQuatComp.h"
#include "floop.h"

// =============================================================================================================

u32 CFQuatComp::m_uBufPos = 0;
u32 CFQuatComp::m_uNumInPool = 0;
//CFQuatComp CFQuatComp::m_aoPool[CFQuatComp_uPoolSize];
CFQuatComp FQuatComp_aoPool[CFQuatComp_uPoolSize];

// =============================================================================================================

BOOL CFQuatComp::LevelInit()
{
	s32 nIndex;
	for( nIndex = 0; nIndex < CFQuatComp_uPoolSize; ++nIndex )
	{
		FQuatComp_aoPool[ nIndex ].m_uFlags = FLAG_DONE;
	}

	m_uBufPos = 0; 

	return TRUE;
}

// =============================================================================================================

CFQuatComp::CFQuatComp()
{
	m_pQOLeft = NULL;
	m_pQORight = NULL;
}

// =============================================================================================================

CFQuatComp *CFQuatComp::GetAvailable()
{
	if(m_uBufPos == CFQuatComp_uPoolSize)
	{
		return(NULL);
	}
	CFQuatComp *poQO = &(FQuatComp_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatComp_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatComp::Init(CFQuatObj *pQOLeft, CFQuatObj *pQORight, BOOL bAutoStart/* = TRUE*/)
{
	m_pQOLeft = pQOLeft;
	m_pQORight = pQORight;

	if(!m_pQOLeft->IsInitted())
	{
		return(FALSE);
	}
	if(!m_pQORight->IsInitted())
	{
		return(FALSE);
	}

	Calculate();
	m_uFlags |= FLAG_INITTED;
	Start();

	return(TRUE);
}

// =============================================================================================================

void CFQuatComp::Work()
{
	Calculate();
}

// =============================================================================================================

void CFQuatComp::Calculate()
{
	m_qValue = m_pQOLeft->GetValue();
	m_qValue.Mul(m_pQORight->GetValue());
}

// =============================================================================================================

CFQuatComp *CFQuatComp::GetPool( void )
{
	return &FQuatComp_aoPool[0];
}

s32 CFQuatComp::GetPoolNumEntries( void )
{
	return CFQuatComp_uPoolSize;
}