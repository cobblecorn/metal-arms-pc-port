//////////////////////////////////////////////////////////////////////////////////////
// AIHazard.cpp - 
//
// Author: Pat MacKellar 
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
// 05/24/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIHazard.h"
#include "APEnew.h"

CAIHazardReg::CAIHazardReg(void)
{
	fang_MemZero(m_aRegistry, 256*sizeof(void*));
	m_uNextHazardId = 1;
}


CAIHazardReg::~CAIHazardReg(void)
{
}


void CAIHazardReg::Work(void)
{
}


void CAIHazardReg::DebugRender(void)
{
}



u8 CAIHazardReg::Register(void* pHazard)
{
	u8 i;
	for (i = 1; i < m_uNextHazardId && pHazard !=  m_aRegistry[i]; i++);

	if (i < 255 && i >= m_uNextHazardId)
	{
	   m_aRegistry[m_uNextHazardId++] = pHazard;
	   return m_uNextHazardId-1;
	}

	return i;
}


void* CAIHazardReg::GetRegistered(u8 uHazardId)
{
	return m_aRegistry[uHazardId];
}
