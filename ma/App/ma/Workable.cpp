//////////////////////////////////////////////////////////////////////////////////////
// Workable.cpp - Workable object class for Mettle Arms.
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
// 03/26/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "Workable.h"
#include "Actor.h"

// =============================================================================================================

CWorkable *CWorkable::m_apWorkList[CWorkable_uWorkListMaxSize];
u32 CWorkable::m_uWLSize;
BOOL CWorkable::m_bSystemInitted = FALSE;

// =============================================================================================================

BOOL CWorkable::InitSystem()
{
	m_bSystemInitted = TRUE;

	ClearWorkList();

	//// Add calls to InitSystem()s of derived classes here.
	//
	CActor::InitSystem();
	//
	////

	return(TRUE);
}

// =============================================================================================================

void CWorkable::UninitSystem()
{
	//// Add calls to UninitSystem()s of derived classes here.
	//
	CActor::UninitSystem();
	//
	////

	ClearWorkList();

	m_bSystemInitted = FALSE;
}

// =============================================================================================================

BOOL CWorkable::LevelInit()
{
	ClearWorkList();
	CActor::LevelInit();

	return(TRUE);
}

// =============================================================================================================

void CWorkable::LevelUninit()
{
	CActor::LevelUninit();
	ClearWorkList();
}

// =============================================================================================================

void CWorkable::Reset()
{
	ClearWorkList();

	//// Add calls to Reset()s of derived classes here.
	//
	CActor::Reset();
	//
	////
}

// =============================================================================================================

void CWorkable::SystemWork()
{
	if(m_uWLSize == 0)
	{
		return;
	}

	u32 uCurWLIdx;
	for(uCurWLIdx = 0; uCurWLIdx < m_uWLSize; ++uCurWLIdx)
	{
		FASSERT(m_apWorkList[uCurWLIdx] != NULL);
		m_apWorkList[uCurWLIdx]->Work();
	}
}

// =============================================================================================================

void CWorkable::ClearWorkList()
{
	u32 uWLIdx;
	for(uWLIdx = 0; uWLIdx < CWorkable_uWorkListMaxSize; ++uWLIdx)
	{
		m_apWorkList[uWLIdx] = NULL;
	}
	m_uWLSize = 0;
}

// =============================================================================================================

s32 CWorkable::AddToWorkList(CWorkable *pWorkable)
{
	if(m_uWLSize == CWorkable_uWorkListMaxSize)
	{
		return(-1);
	}

	m_apWorkList[m_uWLSize] = pWorkable;
	++m_uWLSize;
	return((s32)(m_uWLSize - 1)); //RAF -- Subtracted one because this is supposed to return an INDEX, not the size of the work list
}

// =============================================================================================================

void CWorkable::RemoveFromWorkList(s32 nWorkListIdx)
{
	FASSERT(nWorkListIdx >= -1);
	FASSERT(nWorkListIdx < (s32)(CWorkable_uWorkListMaxSize));

	if(nWorkListIdx == -1)
	{
		// It was never added to the work list.
		return;
	}

	if(m_apWorkList[nWorkListIdx] != NULL)
	{
		--m_uWLSize;
		m_apWorkList[nWorkListIdx] = m_apWorkList[m_uWLSize];
		m_apWorkList[m_uWLSize] = NULL;
	}
}

// =============================================================================================================

void CWorkable::RemoveFromWorkList(CWorkable *pWorkable)
{
	// Find the object in the work list.
	u32 uWLIdx ;
	for(uWLIdx = 0; uWLIdx < CWorkable_uWorkListMaxSize; ++uWLIdx)
	{
		if(m_apWorkList[uWLIdx] == pWorkable)
			break;
	}

	FASSERT(uWLIdx < CWorkable_uWorkListMaxSize);
	RemoveFromWorkList((s32)(uWLIdx));
}

// =============================================================================================================
