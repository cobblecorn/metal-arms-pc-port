//////////////////////////////////////////////////////////////////////////////////////
// AIFSM.cpp - 
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
// 10/17/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIFSM.h"
#include "AINodepools.h"
#include "fres.h"


CFSMStateDef::CFSMStateDef(void)
{
	_ClearData();
}

CFSMStateDef::CFSMStateDef(cchar* pszStateName, AIFSMCallBack* pInitCB, AIFSMCallBack* pWorkCB, AIFSMCallBack* pUninitCB, FSMStateHandle uHandle, u32 uControlParam, void* pvParam1)
{
	m_pszStateName = pszStateName;
	m_apCBFuncs[AIFSM_CBTYPE_INIT] = pInitCB;
	m_apCBFuncs[AIFSM_CBTYPE_WORK] = pWorkCB;
	m_apCBFuncs[AIFSM_CBTYPE_UNINIT] =	pUninitCB;
	m_uControlParam = uControlParam;
	m_pvParam1 = pvParam1;
	m_uHandle = uHandle;

}

void CFSMStateDef::_ClearData(void)
{
	for (u32 i = 0; i < NUM_AIFSM_CBTYPES; i++)
	{
		m_apCBFuncs[i] = NULL;
	}
	m_uControlParam = 0;
	m_pvParam1 = NULL;
	m_uHandle = FSMSTATEHANDLE_INVALID;
	m_pszStateName = NULL;
}


CFSM::CFSM(void)
{
	m_uActiveStateStage = STAGE_INACTIVE;
	m_PendingStateList.SetNodePool(aimain_pNodePool);
}


CFSM::~CFSM(void)
{
	for (u32 i = 0; i < NUM_AIFSM_CBTYPES; i++)
	{
		m_ActiveState.m_apCBFuncs[i] = NULL;
	}

	FASSERT(m_PendingStateList.Size()==0);

	if (m_PendingStateList.Size())
	{
		RemovePendingAndActiveStates();
	}
}

const cchar* CFSM::GetActiveStateName(void)
{
	cchar* pszName = NULL;
	if (m_uActiveStateStage != STAGE_INACTIVE)
	{
		pszName = m_ActiveState.m_pszStateName;
	}
	return pszName;
}

FSMStateHandle CFSM::GetActiveStateHandle(void)
{
	FSMStateHandle uHandle = FSMSTATEHANDLE_INVALID;
	if (m_uActiveStateStage != STAGE_INACTIVE)
	{
		uHandle = m_ActiveState.m_uHandle;
	}
	return uHandle;
}

FSMStateHandle CFSM::GetPreviouslyActiveStateHandle(void)
{
	return m_PrevActiveStateHandle;
}

void CFSM::RemovePendingAndActiveStates(BOOL bDoActiveStateCleanup /* = FALSE*/)
{
	//give currents state cleanup func a call for the heck of it.
	if (bDoActiveStateCleanup)
	{
		if (m_uActiveStateStage != STAGE_INACTIVE && m_ActiveState.m_apCBFuncs[AIFSM_CBTYPE_UNINIT])
		{
			(m_ActiveState.m_apCBFuncs[AIFSM_CBTYPE_UNINIT])(m_ActiveState.m_uControlParam, m_ActiveState.m_pvParam1, NULL);
		}
	}
	m_uActiveStateStage = STAGE_INACTIVE;

	RemovePendingStates();
}


void CFSM::RemovePendingStates(void)
{
	CNiIterator<CPendingStateChange*> it = m_PendingStateList.Begin();
	FASSERT(s_pCFSMPendingStateBank);
	while (it.IsValid())
	{
		s_pCFSMPendingStateBank->Recycle(it.Get());
		it.Next();
	}
	m_PendingStateList.RemoveAll();
}


BOOL CFSM::PopPendingStateChange(void)
{
	FASSERT(s_pCFSMPendingStateBank);
	CPendingStateChange* pPending = m_PendingStateList.PopHead();
	if (pPending)
	{
		s_pCFSMPendingStateBank->Recycle(pPending);
		return TRUE;
	}
	return FALSE;
}


BOOL CFSM::ChangeState(	const CFSMStateDef& NewState,
						FSMStateHandle *puStateHandle /*= FSMSTATEHANDLE_INVALID*/,	
						u32* puControlParam /*= NULL*/,	
						void **ppvParam1 /*= NULL */)
{
	FASSERT(s_pCFSMPendingStateBank); //must call CFSM::InitSystem first
	CPendingStateChange* pPending = s_pCFSMPendingStateBank->Get();
	FASSERT(pPending);
	if (pPending)
	{
		pPending->m_StateDef = NewState;  //copy it
		
		//overrides?
		if (puStateHandle)
		{
		  pPending->m_StateDef.m_uHandle = *puStateHandle;
		}

		if (puControlParam)
		{
			pPending->m_StateDef.m_uControlParam = *puControlParam;
		}

		if (ppvParam1)
		{
			pPending->m_StateDef.m_pvParam1 = *ppvParam1;
		}

		m_PendingStateList.PushTail(pPending);
	}

	return pPending!=NULL;
}


void CFSM::Work(void* pParam2)
{
	u8 abStageCount[NUM_STAGES] = {0, 0, 0};
	
	if (m_uActiveStateStage == STAGE_INACTIVE && IsStateChangePending())
	{
	   m_ActiveState = GetPendingStateChange()->GetStateDef();
	   PopPendingStateChange();
	   m_uActiveStateStage = STAGE_INITIALIZING;
	}
	
	while (	m_uActiveStateStage != STAGE_INACTIVE &&
			!abStageCount[m_uActiveStateStage])	 //each stage could be executed up to one time each frame. So, one state can cleanup and the next could init and then work all in one frame.
	{
		abStageCount[m_uActiveStateStage]++;
		
		if (!m_ActiveState.m_apCBFuncs[m_uActiveStateStage] || (m_ActiveState.m_apCBFuncs[m_uActiveStateStage])(m_ActiveState.m_uControlParam, m_ActiveState.m_pvParam1, pParam2))
		{
			//some CB returned FALSE.
			if (m_uActiveStateStage == STAGE_UNINITIALIZING || m_uActiveStateStage == STAGE_INITIALIZING)   //the return value of a STAGE_WORKING CB means nothing
			{  //advance to next stage
				m_uActiveStateStage++;
			}
		}

		if (m_uActiveStateStage == STAGE_WORKING && IsStateChangePending())
		{   //while working, a state change was requested
			m_uActiveStateStage	= STAGE_UNINITIALIZING;
		}

		if (m_uActiveStateStage	== STAGE_INACTIVE && IsStateChangePending())
		{
		   m_ActiveState = GetPendingStateChange()->GetStateDef();
		   PopPendingStateChange();
		   m_uActiveStateStage = STAGE_INITIALIZING;
		}
	}
}


//
// static
//
CNiBank<CPendingStateChange>* CFSM::s_pCFSMPendingStateBank = NULL;
CPendingStateChange *CNiIterator<CPendingStateChange*>::s_ReturnError;
											    
BOOL CFSM::InitSystem(struct FLinkRoot_s* pGlobalPtrNodePool)
{
	FASSERT(s_pCFSMPendingStateBank == NULL);
	s_pCFSMPendingStateBank = APE_NEW CNiBank<CPendingStateChange>(pGlobalPtrNodePool, CFSM_DEFAULT_GLOBAL_PENDINGSTATECHANGE_POOL_COUNT, APE_NEW CPendingStateChange[CFSM_DEFAULT_GLOBAL_PENDINGSTATECHANGE_POOL_COUNT]);

	return s_pCFSMPendingStateBank!=NULL;
}


void CFSM::UninitSystem(void)
{
	APE_DELETE(s_pCFSMPendingStateBank); s_pCFSMPendingStateBank = NULL;
}
