//////////////////////////////////////////////////////////////////////////////////////
// AIThought.cpp - 
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
// 04/15/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIThought.h"
#include "AINodePools.h"
#include "AIBrain.h"
#include "AIMover.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions




void CAIThought::Init(	CAIBrain* pBrain,
						u8 uThoughtFlags,
						u8 uThoughtType,
						CAIMemorySmall* pParamPack)
{
	m_pBrain = pBrain;
	m_pParamPack = pParamPack;
	m_uThoughtFlags = uThoughtFlags;
	m_uThoughtType = uThoughtType;
}


void CAIThought::Cleanup(void)
{
	if (m_pBrain && m_pBrain->GetAIMover())
	{
		m_pBrain->GetAIMover()->StopAndClearCmds();
	}
}







CNiBank<CAIThought>* CAIThought::s_pAllocBank = NULL;

BOOL CAIThought::SupportsClassInterface(const s32 uInterfaceId)
{
	return uInterfaceId == CLASS_CAITHOUGHT;
}


s32 CAIThought::GetClassInterface(void)
{
	return CLASS_CAITHOUGHT;
}

BOOL CAIThought::InitBank(s32 nHowMany)
{
	if (!CAIThought::s_pAllocBank)
	{
		CAIThought::s_pAllocBank = APE_NEW CNiBank<CAIThought>(aimain_pNodePool, nHowMany, APE_NEW CAIThought[nHowMany]);
	}

	return CAIThought::s_pAllocBank != NULL;
}


void CAIThought::CleanupBank(void)
{
	APE_DELETE (CAIThought::s_pAllocBank); CAIThought::s_pAllocBank = NULL;
}


CAIThought* CAIThought::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CAIThought::s_pAllocBank->Recycle(pRecycle);
	}
	else
	{
		return CAIThought::s_pAllocBank->Get();
	}
	return NULL;
}
