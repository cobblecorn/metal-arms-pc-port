//////////////////////////////////////////////////////////////////////////////////////
// AIMemory.cpp - 
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
// 04/25/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIKnowledge.h"
#include "AIGameUtils.h"
#include "flinklist.h"
#include "ftext.h"
#include "FCheckPoint.h"
#include "AIBrainMems.h"  //debug only

extern u8 AIKnowledge_MemoryId_To_MemorySize(u8 uMemoryId);
extern u8 AIKnowledge_ShouldBrainSaveMemoryId(u8 uMemoryId);


void CAIMemorySmall::InitSmall(u8 uMemoryId, u16 uHowLongSecs, u8 uControlFlags /*=0*/)
{
	m_uWhenTimeSecs = aiutils_GetCurTimeSecsU16();
	m_uHowLongSecs = uHowLongSecs;
	m_uMemoryId = uMemoryId;
	m_uControlFlags = uControlFlags | CONTROL_FLAG_NEW_MEMORY;
	m_uSmallData = 0;
}


void CAIMemorySmall::ResetTimer(u16 uHowLongSecs)
{
	m_uControlFlags |= CONTROL_FLAG_NEW_MEMORY;
	m_uControlFlags &= ~CONTROL_FLAG_HAD_ONE_UPDATE;
	m_uControlFlags &= ~CONTROL_FLAG_ACKNOWLEDGED;
	m_uControlFlags &= ~CONTROL_FLAG_NEWLASTWORK_MEMORY;
	m_uWhenTimeSecs = aiutils_GetCurTimeSecsU16();
	m_uHowLongSecs = uHowLongSecs;
}


//
//	Class CAIMemoryMedium
//
void CAIMemoryMedium::InitMedium(u8 uMemoryId, u16 uHowLongSecs, u8 uControlFlags /*=0*/)
{
	FASSERT(AIKnowledge_MemoryId_To_MemorySize(uMemoryId) != MEMORY_SIZE_SMALL);
	
	CAIMemorySmall::InitSmall(uMemoryId, uHowLongSecs, uControlFlags);
	m_uMediumData = 0;
	m_fMediumData = 0.0f;
}


//
//	Class  CAIMemoryLarge
//  - Small + pEntity + Loc + ObjLoc + u16 + u16
//
void CAIMemoryLarge::InitLarge(u8 uMemoryId, u16 uHowLongSecs, u8 uControlFlags /*=0*/)
{
	FASSERT(AIKnowledge_MemoryId_To_MemorySize(uMemoryId) >= MEMORY_SIZE_LARGE);

	CAIMemoryMedium::InitMedium(uMemoryId, uHowLongSecs, uControlFlags);
	m_Loc.Zero();
	m_EntityLoc.Zero();
	m_pEntity = NULL;
	m_fLargeData = 0.0f;
}



//
//	Class  CAIKnowledge
//
CAIKnowledge::CAIKnowledge(void)
{
	if (s_pGlobalPtrNodePool)
	{
		SetPtrListNodePoolUseage(s_pGlobalPtrNodePool);
	}
}


void CAIKnowledge::SetPtrListNodePoolUseage(FLinkRoot_t* pNodePool)
{
	
	FASSERT(m_ListSmall.Size()==0 && m_ListMedium.Size()==0 && m_ListLarge.Size()==0);  //this should only be called right after construction, or not at all!

	m_ListSmall.SetNodePool(pNodePool);
	m_ListMedium.SetNodePool(pNodePool);
	m_ListLarge.SetNodePool(pNodePool);
}


CAIKnowledge::~CAIKnowledge(void)
{
	//nothing to delete, just make sure the memory has been cleaned out
	FASSERT(m_ListSmall.Size()==0);
	FASSERT(m_ListMedium.Size()==0);
	FASSERT(m_ListLarge.Size()==0);
	ForgetAll();
}


CAIMemorySmall* CAIKnowledge::RememberSmall(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags /*= 0*/)
{
	CAIMemorySmall* pSmall = s_pBankSmall->Get();
	if (pSmall)
	{
		pSmall->InitSmall(uMemoryId, uForHowLongSecs, uControlFlags);
		m_ListSmall.PushHead(pSmall);
	}
	else
	{
		DEVPRINTF("AIWarn: s_pBankSmall Empty!\n");
	}
	return pSmall;
}


CAIMemoryMedium* CAIKnowledge::RememberMedium(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags /*= 0*/)
{
	CAIMemoryMedium* pMedium = s_pBankMedium->Get();
	if (pMedium)
	{
		pMedium->InitMedium(uMemoryId, uForHowLongSecs, uControlFlags);
		m_ListMedium.PushHead(pMedium);
	}
	else
	{
		DEVPRINTF("AIWarn: s_pBankMedium Empty!\n");
	}
	return pMedium;
}


CAIMemoryLarge* CAIKnowledge::RememberLarge(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags /*= 0*/)
{
	CAIMemoryLarge* pLarge = s_pBankLarge->Get();
	if (pLarge)
	{
		pLarge->InitLarge(uMemoryId, uForHowLongSecs, uControlFlags);
		m_ListLarge.PushHead(pLarge);
	}
	else
	{
		DEVPRINTF("AIWarn: s_pBankLarge Empty!\n");
	}
	return pLarge;
}

CAIMemorySmall* CAIKnowledge::RememberThis(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags /*= 0*/)
{
	CAIMemorySmall* pMem = NULL;
	switch (AIKnowledge_MemoryId_To_MemorySize(uMemoryId))
	{
		case MEMORY_SIZE_SMALL:
			pMem = RememberSmall(uMemoryId, uForHowLongSecs, uControlFlags);
			break;
		case MEMORY_SIZE_MEDIUM:
			pMem = RememberMedium(uMemoryId, uForHowLongSecs, uControlFlags);
			break;
		case MEMORY_SIZE_LARGE:
			pMem = RememberLarge(uMemoryId, uForHowLongSecs, uControlFlags);
			break;
		default:
			FASSERT(0); //unknown sizetype
			break;
	}
	return pMem;

}

void CAIKnowledge::ForgetAll(void)
{
	//small
	{
		CNiIterator<CAIMemorySmall*> it = m_ListSmall.Begin();
		while (it.IsValid())
		{
			s_pBankSmall->Recycle(it.Get());
			it.Next();
		}
		m_ListSmall.RemoveAll();
	}

	//medium
	{
		CNiIterator<CAIMemoryMedium*> it = m_ListMedium.Begin();
		while (it.IsValid())
		{
			s_pBankMedium->Recycle(it.Get());
			it.Next();
		}
		m_ListMedium.RemoveAll();
	}

	 //large
	{
		CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
		while (it.IsValid())
		{
			s_pBankLarge->Recycle(it.Get());
			it.Next();
		}
		m_ListLarge.RemoveAll();
	}
}


u8 CAIKnowledge::GetMemorySize(CAIMemorySmall* pMem)
{
	return AIKnowledge_MemoryId_To_MemorySize(pMem->m_uMemoryId);
}


//size in bytes of this memory
u8 CAIKnowledge::GetSizeBytes(CAIMemorySmall* pMem)
{
	u32 auMemSizeSizes[NUM_MEMORY_SIZES] = {sizeof(CAIMemorySmall), sizeof(CAIMemoryMedium), sizeof(CAIMemoryLarge)};
	return auMemSizeSizes[AIKnowledge_MemoryId_To_MemorySize(pMem->m_uMemoryId)];

}

void CAIKnowledge::ForgetThis(CAIMemorySmall* pMemory)
{
	if (!pMemory)
		return;

	switch (GetMemorySize(pMemory))
	{
	case MEMORY_SIZE_SMALL:
		if (m_ListSmall.Remove(pMemory))
		{
			s_pBankSmall->Recycle(pMemory);
		}
		else
		{
			FASSERT(0 && "CAIKnowledge::ForgetThis was told to remove something it doesn't know about"); 
		}
		break;
	case MEMORY_SIZE_MEDIUM:
		if (m_ListMedium.Remove((CAIMemoryMedium*) pMemory))
		{
			s_pBankMedium->Recycle((CAIMemoryMedium*) pMemory);
		}
		else
		{
			FASSERT(0 && "CAIKnowledge::ForgetThis was told to remove something it doesn't know about"); 
		}
		break;
	case MEMORY_SIZE_LARGE:
		if (m_ListLarge.Remove((CAIMemoryLarge*) pMemory))
		{
			s_pBankLarge->Recycle((CAIMemoryLarge*) pMemory);
		}
		else
		{
			FASSERT(0 && "CAIKnowledge::ForgetThis was told to remove something it doesn't know about"); 
		}
		break;
	default:
		FASSERT(0); //unknown sizetype
		break ;
	}
}


void CAIKnowledge::ForgetAllId(u8 nMemoryId)
{
	//small
	{
		CNiIterator<CAIMemorySmall*> it = m_ListSmall.Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pSmall = it.Get();
			if (pSmall->m_uMemoryId==nMemoryId)
			{
				it.RemoveThenNext();
				s_pBankSmall->Recycle(pSmall);
			}
			else
			{
				it.Next();
			}
		}
	}

	//medium
	{
		CNiIterator<CAIMemoryMedium*> it = m_ListMedium.Begin();
		while (it.IsValid())
		{
			CAIMemoryMedium* pMedium = it.Get();
			if (pMedium->m_uMemoryId==nMemoryId)
			{
				it.RemoveThenNext();
				s_pBankMedium->Recycle(pMedium);
			}
			else
			{
				it.Next();
			}
		}
	}

	//large
	{
		CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
		while (it.IsValid())
		{
			CAIMemoryLarge* pLarge = it.Get();
			if (pLarge->m_uMemoryId==nMemoryId)
			{
				it.RemoveThenNext();
				s_pBankLarge->Recycle(pLarge);
			}
			else
			{
				it.Next();
			}
		}
	}
}


void CAIKnowledge::ForgetAllExceptId(u8 nMemoryId)
{
	//small
	{
		CNiIterator<CAIMemorySmall*> it = m_ListSmall.Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pSmall = it.Get();
			if (pSmall->m_uMemoryId!=nMemoryId)
			{
				it.RemoveThenNext();
				s_pBankSmall->Recycle(pSmall);
			}
			else
			{
				it.Next();
			}
		}
	}

	//medium
	{
		CNiIterator<CAIMemoryMedium*> it = m_ListMedium.Begin();
		while (it.IsValid())
		{
			CAIMemoryMedium* pMedium = it.Get();
			if (pMedium->m_uMemoryId!=nMemoryId)
			{
				it.RemoveThenNext();
				s_pBankMedium->Recycle(pMedium);
			}
			else
			{
				it.Next();
			}
		}
	}

	//large
	{
		CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
		while (it.IsValid())
		{
			CAIMemoryLarge* pLarge = it.Get();
			if (pLarge->m_uMemoryId!=nMemoryId)
			{
				it.RemoveThenNext();
				s_pBankLarge->Recycle(pLarge);
			}
			else
			{
				it.Next();
			}
		}
	}
}


void CAIKnowledge::ForgetOldestSmall(void)				// kill the oldest memory
{
	if (CAIMemorySmall* pSmall = m_ListSmall.PopTail())
	{
		s_pBankSmall->Recycle(pSmall);
	}
}


void CAIKnowledge::ForgetOldestMedium(void)				// kill the oldest memory
{
	if (CAIMemoryMedium* pMedium = m_ListMedium.PopTail())
	{
		s_pBankMedium->Recycle(pMedium);
	}
}


void CAIKnowledge::ForgetOldestLarge(void)				// kill the oldest memory
{
	if (CAIMemoryLarge* pLarge = m_ListLarge.PopTail())
	{
		s_pBankLarge->Recycle(pLarge);
	}
}


void CAIKnowledge::ForgetOldestMatchSize(u8 uMemoryId)	// kill the oldest memory in the list of size that matches nMemory
{
	switch (AIKnowledge_MemoryId_To_MemorySize(uMemoryId))
	{
		case MEMORY_SIZE_SMALL:
			ForgetOldestSmall();
			break;
		case MEMORY_SIZE_MEDIUM:
			ForgetOldestMedium();
			break;
		case MEMORY_SIZE_LARGE:
			ForgetOldestLarge();
			break;
		default:
			FASSERT(0 && "ForgetOldestMatchSize");
	}
}


BOOL CAIKnowledge::IsInMemory(CAIMemorySmall* pMemory)
{
	BOOL bCan = FALSE;

	//small
	{
		CNiIterator<CAIMemorySmall*> it = m_ListSmall.Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pSmall = it.Get();
			if (pSmall == pMemory)
			{
				bCan = TRUE;
				break;
			}
			else
			{
				it.Next();
			}
		}
	}

	//medium
	if (!bCan)
	{
		CNiIterator<CAIMemoryMedium*> it = m_ListMedium.Begin();
		while (it.IsValid())
		{
			CAIMemoryMedium* pMedium = it.Get();
			if (pMedium == pMemory)
			{
				bCan = TRUE;
				break;
			}
			else
			{
				it.Next();
			}
		}
	}

	//large
	if (!bCan)
	{
		CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
		while (it.IsValid())
		{
			CAIMemoryLarge* pLarge = it.Get();
			if (pLarge == pMemory)
			{
				bCan = TRUE;
				break;
			}
			else
			{
				it.Next();
			}
		}
	}
	return bCan;
}


BOOL CAIKnowledge::CanRememberEntity(u8 nMemoryId, CEntity* pEntity, CAIMemoryLarge** ppMemory /*= NULL*/)
{
	BOOL bCan = FALSE;


	FASSERT(AIKnowledge_MemoryId_To_MemorySize(nMemoryId) == MEMORY_SIZE_LARGE);


	if (ppMemory)
	{
		*ppMemory = NULL;
	}

	//entities can only be in large memory list

	CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
	while (it.IsValid())
	{
		CAIMemoryLarge* pLarge = it.Get();
		if (pLarge->m_uMemoryId==nMemoryId && pLarge->m_pEntity == pEntity)
		{
			bCan = TRUE;
			if (ppMemory)
			{
				*ppMemory = pLarge;
			}
			break;
		}
		else
		{
			it.Next();
		}
	}
	return bCan;
}


BOOL CAIKnowledge::CanRememberAny(u8 uMemoryId, CAIMemorySmall** ppMemory /*= NULL*/)
{
	BOOL bCan = FALSE;
	if (ppMemory)
	{
		*ppMemory = NULL;
	}

	u8 uMemorySize = AIKnowledge_MemoryId_To_MemorySize(uMemoryId);

	switch(uMemorySize)
	{
		case MEMORY_SIZE_SMALL:

			//small
			{
				CNiIterator<CAIMemorySmall*> it = m_ListSmall.Begin();
				while (it.IsValid())
				{
					CAIMemorySmall* pSmall = it.Get();
					if (pSmall->m_uMemoryId==uMemoryId)
					{
						bCan = TRUE;
						if (ppMemory)
						{
							*ppMemory = pSmall;
						}
						break;
					}
					else
					{
						it.Next();
					}
				}
			}
			break;
		case MEMORY_SIZE_MEDIUM:
			//medium
			if (!bCan)
			{
				CNiIterator<CAIMemoryMedium*> it = m_ListMedium.Begin();
				while (it.IsValid())
				{
					CAIMemoryMedium* pMedium = it.Get();
					if (pMedium->m_uMemoryId==uMemoryId)
					{
						bCan = TRUE;
						if (ppMemory)
						{
							*ppMemory = pMedium;
						}
						break;
					}
					else
					{
						it.Next();
					}
				}
			}
			break;
	case MEMORY_SIZE_LARGE:
		//large
			if (!bCan)
			{
				CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
				while (it.IsValid())
				{
					CAIMemoryLarge* pLarge = it.Get();
					if (pLarge->m_uMemoryId==uMemoryId)
					{
						bCan = TRUE;
						if (ppMemory)
						{
							*ppMemory = pLarge;
						}
						break;
					}
					else
					{
						it.Next();
					}
				}
			}
			break;
	}

	return bCan;
}

CAIMemorySmall* CAIKnowledge::GetNextOldestOfId(CAIMemorySmall* pMemory)
{
	u8 uMemorySize = AIKnowledge_MemoryId_To_MemorySize(pMemory->m_uMemoryId);
	BOOL bFoundOrig = FALSE;

	switch(uMemorySize)
	{
		case MEMORY_SIZE_SMALL:
			{
				CNiIterator<CAIMemorySmall*> it = m_ListSmall.Begin();
				while (it.IsValid())
				{
					CAIMemorySmall* pSmall = it.Get();
					if (pSmall == pMemory)
					{
						bFoundOrig = TRUE;
						it.Next();
					}
					else if (bFoundOrig && pSmall->m_uMemoryId == pMemory->m_uMemoryId)
					{
						return pSmall;
					}
					else
					{
						it.Next();
					}
				}
			}
			break;
		case MEMORY_SIZE_MEDIUM:
			//medium
			{
				CNiIterator<CAIMemoryMedium*> it = m_ListMedium.Begin();
				while (it.IsValid())
				{
					CAIMemoryMedium* pMedium = it.Get();
					if (pMedium == pMemory)
					{
						bFoundOrig = TRUE;
						it.Next();
					}
					else if (bFoundOrig && pMedium->m_uMemoryId == pMemory->m_uMemoryId)
					{
						return pMedium;
					}
					else
					{
						it.Next();
					}
				}
			}
			break;
	case MEMORY_SIZE_LARGE:
		//large
			{
				CNiIterator<CAIMemoryLarge*> it = m_ListLarge.Begin();
				while (it.IsValid())
				{
					CAIMemoryLarge* pLarge = it.Get();
					if (pLarge == pMemory)
					{
						bFoundOrig = TRUE;
						it.Next();
					}
					else if (bFoundOrig && pLarge->m_uMemoryId == pMemory->m_uMemoryId)
					{
						return pLarge;
					}
					else
					{
						it.Next();
					}
				}
			}
			break;
	}

	return NULL;
}



void CAIKnowledge::Work(void)
{
	CNiList<CAIMemorySmall*>* aMemoryLists[3] = {&(m_ListSmall), (CNiList<CAIMemorySmall*>*) &(m_ListMedium), (CNiList<CAIMemorySmall*>*) &(m_ListLarge)};
	u16 nTime = aiutils_GetCurTimeSecsU16();
	for (int i = 0; i < 3; i++)
	{
		CNiIterator<CAIMemorySmall*> it = aMemoryLists[i]->Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pMemory = it.Get();

			if (pMemory->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEWLASTWORK_MEMORY)
			{
				pMemory->m_uControlFlags &= ~CAIMemorySmall::CONTROL_FLAG_NEWLASTWORK_MEMORY;
			}

			if (pMemory->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_HAD_ONE_UPDATE)
			{
				if (pMemory->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
				{
					pMemory->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_NEWLASTWORK_MEMORY;
				}
				pMemory->m_uControlFlags &=~ CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY;
			}
			pMemory->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_HAD_ONE_UPDATE;
			
			if (pMemory->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NOTIMER)
			{
				it.Next();
				continue;
			}
			else if (pMemory->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_FRAMECOUNT)
			{
				pMemory->m_uHowLongSecs--;
				if (pMemory->m_uHowLongSecs == 0xffff)
				{
					it.RemoveThenNext();
					switch (GetMemorySize(pMemory))
					{
						case 0:
							s_pBankSmall->Recycle(pMemory);
							break;
						case 1:
							s_pBankMedium->Recycle((CAIMemoryMedium*) pMemory);
							break;
						case 2:
							s_pBankLarge->Recycle((CAIMemoryLarge*) pMemory);
							break;
					}
				}
				else
				{
					it.Next();
				}
			}
			else if (pMemory->m_uWhenTimeSecs + pMemory->m_uHowLongSecs < nTime)
			{
				it.RemoveThenNext();
				switch (GetMemorySize(pMemory))
				{
					case 0:
						s_pBankSmall->Recycle(pMemory);
						break;
					case 1:
						s_pBankMedium->Recycle((CAIMemoryMedium*) pMemory);
						break;
					case 2:
						s_pBankLarge->Recycle((CAIMemoryLarge*) pMemory);
						break;
				}

			}
			else
			{
				it.Next();
			}
		}
	}
}


void CAIKnowledge::DebugRender(f32 fScreenX, f32 fScreenY)
{
#if !FANG_PRODUCTION_BUILD
	s32 nLine = 0;
	f32 fLineHeight = 0.02f;
	CNiList<CAIMemorySmall*>* aMemoryLists[3] = {&(m_ListSmall), (CNiList<CAIMemorySmall*>*) &(m_ListMedium), (CNiList<CAIMemorySmall*>*) &(m_ListLarge)};
	u16 nTime = aiutils_GetCurTimeSecsU16();
	for (int i = 0; i < 3; i++)
	{
		CNiIterator<CAIMemorySmall*> it = aMemoryLists[i]->Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pSmall = it.Get();
			s32 nTimeLeft;
			if (pSmall->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NOTIMER)
			{
				nTimeLeft = 0;
			}
			else if (pSmall->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_FRAMECOUNT)
			{
				nTimeLeft = pSmall->m_uHowLongSecs;
			}
			else
			{
				nTimeLeft = (pSmall->m_uHowLongSecs+pSmall->m_uWhenTimeSecs) - nTime;
			}
			ftext_DebugPrintf( fScreenX, fScreenY-fLineHeight*(f32)nLine, "~w1%03d:%s", nTimeLeft, AIBrainMem_aMemDefs[pSmall->m_uMemoryId].pszLabel);
			nLine++;
			it.Next();
		}
	}
#endif
}
	
//
//
//  CAIKnowledge
//		static section
//
CNiBank<CAIMemorySmall>* CAIKnowledge::s_pBankSmall = NULL;
CNiBank<CAIMemoryMedium>* CAIKnowledge::s_pBankMedium = NULL;
CNiBank<CAIMemoryLarge>* CAIKnowledge::s_pBankLarge = NULL;
FLinkRoot_t* CAIKnowledge::s_pGlobalPtrNodePool = NULL;
CAIMemorySmall* CNiIterator<CAIMemorySmall*>::s_ReturnError;
CAIMemoryMedium* CNiIterator<CAIMemoryMedium*>::s_ReturnError;
CAIMemoryLarge* CNiIterator<CAIMemoryLarge*>::s_ReturnError;
										 
BOOL CAIKnowledge::InitSystem(struct FLinkRoot_s* pGlobalPtrNodePool,
							  u32 nSmallBankSize /*= CAIKNOWLEDGE_DEFAULT_SMALLMEMORY_GLOBAL_BANK_SIZE*/,
							  u32 nMediumBankSize /*= CAIKNOWLEDGE_DEFAULT_MEDIUMMEMORY_GLOBAL_BANK_SIZE */,
						  	  u32 nLargeBankSize /*= CAIKNOWLEDGE_DEFAULT_LARGEMEMORY_GLOBAL_BANK_SIZE */)
{
	FASSERT(!(s_pBankSmall || s_pBankMedium || s_pBankLarge));  //better all be null!

	AIBrainMems_Init();

	s_pBankSmall = APE_NEW CNiBank<CAIMemorySmall>(pGlobalPtrNodePool, nSmallBankSize, APE_NEW CAIMemorySmall[nSmallBankSize]);
	s_pBankMedium = APE_NEW CNiBank<CAIMemoryMedium>(pGlobalPtrNodePool, nMediumBankSize, APE_NEW CAIMemoryMedium[nMediumBankSize]);
	s_pBankLarge = APE_NEW CNiBank<CAIMemoryLarge>(pGlobalPtrNodePool, nLargeBankSize, APE_NEW CAIMemoryLarge[nLargeBankSize]);

	if (!(s_pBankSmall && s_pBankMedium && s_pBankLarge))  //better all be valid now
	{
		CAIKnowledge::CleanupSystem();
		return FALSE;
	}

	s32 nSizeOfCAIMemorySmall  = sizeof(CAIMemorySmall);
	s32 nSizeOfCAIMemoryMedium  = sizeof(CAIMemoryMedium);
	s32 nSizeOfCAIMemoryLarge  = sizeof(CAIMemoryLarge);
//	DEVPRINTF("nSizeOfCAIMemorySmall: %d bytes\n", nSizeOfCAIMemorySmall);
//	DEVPRINTF("nSizeOfCAIMemoryMedium: %d bytes\n", nSizeOfCAIMemoryMedium);
//	DEVPRINTF("nSizeOfCAIMemoryLarge: %d bytes\n", nSizeOfCAIMemoryLarge);
	return TRUE;
}


void CAIKnowledge::CleanupSystem(void)
{
//	FASSERT(!s_pBankSmall || s_pBankSmall->Size() ==0);
//	FASSERT(!s_pBankMedium || s_pBankMedium->Size() ==0);
//	FASSERT(!s_pBankLarge || s_pBankLarge->Size() ==0);
	APE_DELETE(s_pBankSmall); s_pBankSmall = NULL;
	APE_DELETE(s_pBankMedium); s_pBankMedium = NULL;
	APE_DELETE(s_pBankLarge); s_pBankLarge = NULL;
}


void CAIKnowledge::SetSystemPtrListNodePoolUseage(FLinkRoot_t* pNodePool)
{
	s_pGlobalPtrNodePool = pNodePool;
}


u8* CAIMemorySmall::PeekU8U8(u8 *pBuff, u8 *puVal1, u8 *puVal2)
{
	FASSERT(pBuff>=(u8*)this);
	if (puVal1)
	{
		*puVal1 = *pBuff;
	}
	if (puVal2)
	{
		*puVal2 = *(pBuff+1);
	}
	return pBuff+2;
}

CAIMemoryLarge* CAIKnowledge::NextUnAcknowledgeMemory(CNiIterator<CAIMemoryLarge*> *pIt)
{
	FASSERT(pIt);

	if (!pIt->IsValid())
	{
		*pIt = SearchLarge();
	}
	else
	{
		pIt->Next();
	}

	CAIMemoryLarge* pMem = NULL;
	while (pIt->IsValid() && (pMem = pIt->Get()) && (pMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED))
	{
		pMem = NULL;
		pIt->Next();
	}

	return pMem;
}

CAIMemoryLarge* CAIKnowledge::NextUnAcknowledgeMemory(u8 uMemoryId, CNiIterator<CAIMemoryLarge*> *pIt)
{
	FASSERT(pIt);

	if (!pIt->IsValid())
	{
		*pIt = SearchLarge();
	}
	else
	{
		pIt->Next();
	}

	CAIMemoryLarge* pMem = NULL;
	while (	pIt->IsValid() &&
			(pMem = pIt->Get())	&&
			((pMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED) || (uMemoryId != pMem->m_uMemoryId)))
	{
		pMem = NULL;
		pIt->Next();
	}

	return pMem;
}


CAIMemoryLarge* CAIKnowledge::NextMemory(u8 uMemoryId, CNiIterator<CAIMemoryLarge*> *pIt)
{
	FASSERT(pIt);

	if (!pIt->IsValid())
	{
		*pIt = SearchLarge();
	}
	else
	{
		pIt->Next();
	}

	CAIMemoryLarge* pMem = NULL;
	while (	pIt->IsValid() &&
			(pMem = pIt->Get())	&&
			(uMemoryId != pMem->m_uMemoryId))
	{
		pMem = NULL;
		pIt->Next();
	}

	return pMem;
}


BOOL CAIKnowledge::CheckpointSave(void)
{
	u32 auNumToSave[NUM_MEMORY_SIZES] = {0,0,0};
	u32 auMemSizeSizes[NUM_MEMORY_SIZES] = {sizeof(CAIMemorySmall), sizeof(CAIMemoryMedium), sizeof(CAIMemoryLarge)};
	CNiList<CAIMemorySmall*>* aMemoryLists[3] = {&(m_ListSmall), (CNiList<CAIMemorySmall*>*) &(m_ListMedium), (CNiList<CAIMemorySmall*>*) &(m_ListLarge)};

	int i;
	u16 nTime = aiutils_GetCurTimeSecsU16();
	for (i = 0; i < NUM_MEMORY_SIZES; i++)
	{
		CNiIterator<CAIMemorySmall*> it = aMemoryLists[i]->Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pMemory = it.Get();
			auNumToSave[i]+= AIKnowledge_ShouldBrainSaveMemoryId(pMemory->m_uMemoryId);
			it.Next();
		}
	}

	for (i = 0; i < NUM_MEMORY_SIZES; i++)
	{
		CFCheckPoint::SaveData( auNumToSave[i] );

		CNiIterator<CAIMemorySmall*> it = aMemoryLists[i]->Begin();
		while (it.IsValid())
		{
			CAIMemorySmall* pMemory = it.Get();
			if (AIKnowledge_ShouldBrainSaveMemoryId(pMemory->m_uMemoryId))
			{
				CFCheckPoint::SaveData( pMemory, auMemSizeSizes[i] );	// it's a large even though m_pJobParamPack points to small
			}
			it.Next();
		}
	}

	return TRUE;
}


void CAIKnowledge::CheckpointRestore(void)
{
	u32 auMemSizeSizes[NUM_MEMORY_SIZES] = {sizeof(CAIMemorySmall), sizeof(CAIMemoryMedium), sizeof(CAIMemoryLarge)};
	CNiList<CAIMemorySmall*>* aMemoryLists[3] = {&(m_ListSmall), (CNiList<CAIMemorySmall*>*) &(m_ListMedium), (CNiList<CAIMemorySmall*>*) &(m_ListLarge)};

	u32 auTempMemId[NUM_MEMORY_SIZES] = {MEMORY_SMALL_PARAMPACK, MEMORY_MEDIUM_PARAMPACK, MEMORY_LARGE_PARAMPACK};

	for (u32 i = 0; i < NUM_MEMORY_SIZES; i++)
	{
		u32 uCount = 0;
		CFCheckPoint::LoadData( uCount );

		for (u32 j = 0; j < uCount; j++)
		{
			CAIMemorySmall* pMem;
			pMem = RememberThis(auTempMemId[i], 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
			if( pMem )
			{
				CFCheckPoint::LoadData( pMem, auMemSizeSizes[i] );
			}
			else
			{
				FASSERT( FALSE );
			}
		}
	}
}
