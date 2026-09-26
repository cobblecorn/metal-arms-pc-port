//////////////////////////////////////////////////////////////////////////////////////
// FMotionObj.cpp - Script System for Mettle Arms.
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
// 03/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "FMotionObj.h"
#include "FVec3Obj.h"
#include "FQuatObj.h"
#include "FScalarObj.h"
#include "FScriptSystem.h"

// =============================================================================================================

BOOL CFMotionObj::m_bModuleInitted = FALSE;
u32 CFMotionObj::m_uWLSize = 0;
CFMotionObj *CFMotionObj::m_apWorkList[CFMotionObj_uWorkListSize];
s32 CFMotionObj::m_nPathEndedEvent = -1;

// =============================================================================================================

BOOL CFMotionObj::ModuleStartup()
{
	u32 uCurWLIdx;
	for(uCurWLIdx = 0; uCurWLIdx < CFMotionObj_uWorkListSize; ++uCurWLIdx)
	{
		m_apWorkList[uCurWLIdx] = NULL;
	}

	m_uWLSize = 0;

	m_bModuleInitted = TRUE;

	if(!CFVec3AObj::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatObj::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFScalarObj::ModuleStartup())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CFMotionObj::ModuleShutdown()
{
	CFScalarObj::ModuleShutdown();
	CFQuatObj::ModuleShutdown();
	CFVec3AObj::ModuleShutdown();

	u32 uCurWLIdx;
	for(uCurWLIdx = 0; uCurWLIdx < CFMotionObj_uWorkListSize; ++uCurWLIdx)
	{
		m_apWorkList[uCurWLIdx] = NULL;
	}
	m_uWLSize = 0;

	m_bModuleInitted = FALSE;
}

// =============================================================================================================

BOOL CFMotionObj::LevelInit()
{
	m_nPathEndedEvent = CFScriptSystem::GetEventNumFromName("path");

	if(!CFVec3AObj::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatObj::LevelInit())
	{
		return(FALSE);
	}
	if(!CFScalarObj::LevelInit())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CFMotionObj::LevelUninit()
{
	CFScalarObj::LevelUninit();
	CFQuatObj::LevelUninit();
	CFVec3AObj::LevelUninit();

	// This whole loop may seem like an odd way to do things, but the End()
	//   method below calls RemoveFromWorkList, which in turn compresses the
	//   the work list down, and thus at any time, only the first m_uWLSize
	//   entries in the work list are valid.  So we just keep removing element
	//   0 and don't walk the array.
	while(m_uWLSize > 0)
	{
		FASSERT(m_apWorkList[0] != NULL);
		m_apWorkList[0]->End();
		FASSERT(m_apWorkList[m_uWLSize] == NULL);
	}
	m_uWLSize = 0;
}


void CFMotionObj::End()
{ 
	FASSERT(IsInitted());
	Pause();
	if(m_bEndEventEnabled) 
	{
		CFScriptSystem::TriggerEvent(m_bEndEventEnabled, (u32)(this), 0, 0);
	} 
	m_uFlags |= FLAG_DONE;
}


// =============================================================================================================
u32 __uCurWLIdx;	//pgm put this here for debugging, because when one of j.u.s.t.i.n.s work functions fails, the victim programmer can look into the array and maybe find out why
void CFMotionObj::ModuleWork()
{
	FASSERT(m_bModuleInitted);

	CFMotionObj *pCurSO;
	for(__uCurWLIdx = 0; __uCurWLIdx < m_uWLSize; ++__uCurWLIdx)
	{
		pCurSO = m_apWorkList[__uCurWLIdx];
		FASSERT(pCurSO != NULL);
		pCurSO->Work();
	}
}

// =============================================================================================================

s32 CFMotionObj::AddToWorkList(CFMotionObj *poSO)
{
	FASSERT(m_bModuleInitted);
	FASSERT(poSO != NULL);

	if(m_uWLSize == CFMotionObj_uWorkListSize)
		return(-1);

	m_apWorkList[m_uWLSize] = poSO;
	++m_uWLSize;

	poSO->m_uFlags |= (FLAG_WORKING);

	return((s32)(m_uWLSize));
}

// =============================================================================================================

void CFMotionObj::RemoveFromWorkList(s32 nSOIdx)
{
	FASSERT(m_bModuleInitted);

	FASSERT(nSOIdx >= 0);
	FASSERT(nSOIdx < CFMotionObj_uWorkListSize);

	m_apWorkList[nSOIdx]->m_uFlags &= (~FLAG_WORKING);

	--m_uWLSize;
	m_apWorkList[nSOIdx] = m_apWorkList[m_uWLSize];
	m_apWorkList[m_uWLSize] = NULL;
}

// =============================================================================================================

void CFMotionObj::RemoveFromWorkList(CFMotionObj *poSO)
{
	FASSERT(m_bModuleInitted);
	FASSERT(poSO != NULL);

	// Find the object in the work list.
	u32 uWLIdx;
	for(uWLIdx = 0; uWLIdx < CFMotionObj_uWorkListSize; ++uWLIdx)
	{
		if(m_apWorkList[uWLIdx] == poSO)
			break;
	}

//	FASSERT(uWLIdx < CFMotionObj_uWorkListSize);
	if(uWLIdx == CFMotionObj_uWorkListSize)
	{
		DEVPRINTF("CFMotionObj::RemoveFromWorkList() : Tried to remove CFMotionObj from work list that wasn't already working.\n");
		return;
	}

	RemoveFromWorkList((s32)(uWLIdx));
}

// =============================================================================================================

BOOL CFMotionObj::IsInWorkList( const CFMotionObj &roSO )
{
	u32 uWLIdx;

	for(uWLIdx = 0; uWLIdx < CFMotionObj_uWorkListSize; ++uWLIdx)
	{
		if(m_apWorkList[uWLIdx] == &roSO)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// =============================================================================================================
