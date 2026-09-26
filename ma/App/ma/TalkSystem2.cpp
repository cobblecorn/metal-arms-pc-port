//////////////////////////////////////////////////////////////////////////////////////
// TalkSystem2.cpp - New Talking System for Mettle Arms.
//
// Author: Justin Link
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 05/28/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "TalkSystem2.h"
#include "FScriptSystem.h"
#include "gstring.h"
#include "fresload.h"
#include "fclib.h"
#include "bot.h"
#include "floop.h"
#include "game.h"
#include "AI/AiApi.h"
#include "BotTalkData.h"
#include "BotTalkInst.h"



FCLASS_NOALIGN_PREFIX class CTS2Resource
{
public:
	CTS2Resource(void);
	~CTS2Resource(void);

	CBotTalkData	m_TalkData;
	FLinkRoot_t		m_AvailableBTIList;
	FLinkRoot_t		m_HandedOutBTIList;
	FLink_t			m_NextResource;

	FCLASS_STACKMEM_NOALIGN(CTS2Resource);

}  FCLASS_NOALIGN_SUFFIX;




u32 CTalkSystem2::s_uNumTalkDatas;
u32 CTalkSystem2::s_uSystemTalkDataCnt;

s32 CTalkSystem2::m_nTalkEvent;

u32 CTalkSystem2::s_uNonAmbientCount;
CFMeshInst *CTalkSystem2::m_pMeshChip = NULL;

CBotTalkInst *CTalkSystem2::m_apBotTalkInst[CTalkSystem2_uMaxSimulBotTalkInst];
CTalkInst *CTalkSystem2::m_apTalkInst[CTalkSystem2_uMaxSimulTalkInst];
FLinkRoot_t CTalkSystem2::m_TalkResourcePool;


static cchar *_pszBankName = "MS11_BoTalk";

CTalkSystem2::CTalkSystem2()
{
}

CTalkSystem2::~CTalkSystem2()
{
}

CFMeshInst* CTalkSystem2::GetChipMeshInst(void)
{
	return m_pMeshChip;
}

BOOL CTalkSystem2::InitSystem()
{
	s_uNumTalkDatas = 0;
	m_nTalkEvent = -1;

	flinklist_InitRoot(&m_TalkResourcePool, FANG_OFFSETOF( CTS2Resource, CTS2Resource::m_NextResource ));
		
	m_pMeshChip = fnew CFMeshInst;
	if(m_pMeshChip == NULL)
	{
		DEVPRINTF("CTalkSystem::InitSystem() : Out of memory.\n");
		return(FALSE);
	}

	FMesh_t *pMesh = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, "gr_1chips01"));
	if(pMesh != NULL)
	{
		FMeshInit_t oMeshInit;
		oMeshInit.fCullDist = FMATH_MAX_FLOAT;
		oMeshInit.Mtx.Identity();
		oMeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE;
		oMeshInit.pMesh = pMesh;
		m_pMeshChip->Init(&oMeshInit);
	}

	return(TRUE);
}



BOOL CTalkSystem2::InitLevel()
{
	// Save off how many were loaded before the level was.
	s_uSystemTalkDataCnt = m_TalkResourcePool.nCount;

	s_uNonAmbientCount = 0;

	// This probably could be in InitSystem(), but I put it here.
	m_nTalkEvent = CFScriptSystem::GetEventNumFromName("talk");	   //Assumes script system has been initialized

	return(TRUE);
}



void CTalkSystem2::UninitLevel()
{
	u32 uCurTIIdx;
	s_uNumTalkDatas = s_uSystemTalkDataCnt;
	FASSERT(s_uNumTalkDatas <= m_TalkResourcePool.nCount);

	//kill all level bta's
	while (m_TalkResourcePool.nCount > s_uSystemTalkDataCnt)
	{
		CTS2Resource* pRes = (CTS2Resource*) flinklist_RemoveHead(&m_TalkResourcePool);
		while (pRes->m_AvailableBTIList.nCount)
		{
			CBotTalkInst* pInst = (CBotTalkInst*) flinklist_RemoveHead(&(pRes->m_AvailableBTIList));
			fdelete(pInst);
		}
		while (pRes->m_HandedOutBTIList.nCount)
		{
			CBotTalkInst* pInst = (CBotTalkInst*) flinklist_RemoveHead(&(pRes->m_HandedOutBTIList));
			fdelete(pInst);
		}
		fdelete(pRes);
	}

	//////////////////////////////////////////////////////////////////////
	// Clear out our list of any talks that may have been working.
	for(uCurTIIdx = 0; uCurTIIdx < CTalkSystem2_uMaxSimulBotTalkInst; ++uCurTIIdx)
	{
		if(m_apBotTalkInst[uCurTIIdx] != NULL)
		{
			m_apBotTalkInst[uCurTIIdx] = NULL;
		}
	}
	//
	//////////////////////////////////////////////////////////////////////
	// Clear out our list of any talks that may have been working.
	for(uCurTIIdx = 0; uCurTIIdx < CTalkSystem2_uMaxSimulTalkInst; ++uCurTIIdx)
	{
		if(m_apTalkInst[uCurTIIdx] != NULL)
		{
			m_apTalkInst[uCurTIIdx] = NULL;
		}
	}

}



void CTalkSystem2::UninitSystem()
{
	//kill all system bta's
	FASSERT(s_uSystemTalkDataCnt == m_TalkResourcePool.nCount);
	FASSERT(m_TalkResourcePool.nCount >=0 && m_TalkResourcePool.nCount < 1000);
	while (m_TalkResourcePool.nCount)
	{
		CTS2Resource* pRes = (CTS2Resource*) flinklist_RemoveTail(&m_TalkResourcePool);

		while (pRes->m_AvailableBTIList.nCount)
		{
			CBotTalkInst* pInst = (CBotTalkInst*) flinklist_RemoveTail(&(pRes->m_AvailableBTIList));
			fdelete(pInst);
		}
		while (pRes->m_HandedOutBTIList.nCount)
		{
			CBotTalkInst* pInst = (CBotTalkInst*) flinklist_RemoveTail(&(pRes->m_HandedOutBTIList));
			fdelete(pInst);
		}

		fdelete( pRes);
	}
	if (m_pMeshChip)
	{
		fdelete(m_pMeshChip);
		m_pMeshChip = NULL;
	}
}



void CTalkSystem2::Work()
{
	u32 uCurTIIdx;
	//////////////////////////////////////////////////////////////////////
	// Call the work functions of everybody that might be active right now.
	for(uCurTIIdx = 0; uCurTIIdx < CTalkSystem2_uMaxSimulBotTalkInst; ++uCurTIIdx)
	{
		if(m_apBotTalkInst[uCurTIIdx] != NULL)
		{
			m_apBotTalkInst[uCurTIIdx]->Work();
		}
	}
	//
	//////////////////////////////////////////////////////////////////////
	for(uCurTIIdx = 0; uCurTIIdx < CTalkSystem2_uMaxSimulTalkInst; ++uCurTIIdx)
	{
		if(m_apTalkInst[uCurTIIdx] != NULL)
		{
			m_apTalkInst[uCurTIIdx]->Work();
		}
	}
}



// Instructs the system to start pTalker talking his talk.
// Adds the pTalkInst to its internal array, which will ensure that it's work function gets called,
//   and that appropriate people get notified.
// Returns TRUE if the request was accepted and talking will take place.
// At this point, the calling system should be sure that the talking can start right now.
BOOL CTalkSystem2::SubmitTalkRequest(CBotTalkInst *pTalkInst, CBot *pBot)
{
	if(pTalkInst == NULL)
	{
		// For some reason they requested to start talking with no talk instance data.
		return(FALSE);
	}
	//////////////////////////////////////////////////////////////////////
	// Check to see if we can find an available slot.
	u32 uCurTIIdx;
	for(uCurTIIdx = 0; (uCurTIIdx < CTalkSystem2_uMaxSimulBotTalkInst) && (m_apBotTalkInst[uCurTIIdx] != NULL); ++uCurTIIdx)
	{
		if(m_apBotTalkInst[uCurTIIdx] == pTalkInst)
		{
			DEVPRINTF("CTalkSystem2::SubmitTalkRequest() : Tried to play the same talk twice.\n");
			return(FALSE);
		}
	}

	if(uCurTIIdx == CTalkSystem2_uMaxSimulBotTalkInst)
	{
		DEVPRINTF("CTalkSystem2::SubmitTalkRequest() : Too many BotTalks going at once.\n");
		return(FALSE);
	}

	if(pTalkInst->GetTalkReason() == TALKACTIVITY2_NONAMBIENT)
	{
		// If this is the first non-ambient speech to start, tell the game.
		if(s_uNonAmbientCount == 0)
		{
			BOOL bRetVal = game_EnterNonAmbientTalk();
			if(!bRetVal)
			{
				return(FALSE);
			}
		}
		++s_uNonAmbientCount;
	}
	//
	//////////////////////////////////////////////////////////////////////
	if(!pTalkInst->Start(pBot))
	{
		return(FALSE);
	}

	m_apBotTalkInst[uCurTIIdx] = pTalkInst;

	return(TRUE);
}



BOOL CTalkSystem2::TerminateActiveTalk(CBotTalkInst *pTalkInst)
{
	//////////////////////////////////////////////////////////////////////
	// Check to see if we can find the CTalkInst anywhere.
	u32 uCurTIIdx;
	for(uCurTIIdx = 0; uCurTIIdx < CTalkSystem2_uMaxSimulBotTalkInst; ++uCurTIIdx)
	{
		if(m_apBotTalkInst[uCurTIIdx] == pTalkInst)
		{
			if(pTalkInst->GetTalkReason() == TALKACTIVITY2_NONAMBIENT)
			{
				FASSERT(s_uNonAmbientCount > 0);
				--s_uNonAmbientCount;
			}
			pTalkInst->End();
			m_apBotTalkInst[uCurTIIdx] = NULL;
			return(TRUE);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	return(FALSE);
}

// Instructs the system to start pTalker talking his talk.
// Adds the pTalkInst to its internal array, which will ensure that it's work function gets called,
//   and that appropriate people get notified.
// Returns TRUE if the request was accepted and talking will take place.
// At this point, the calling system should be sure that the talking can start right now.
BOOL CTalkSystem2::SubmitTalkRequest(	CTalkInst *pTalkInst,
										CFAnimCombiner* pCombiner,
										const s32 *panTapIds,
										const s32 *panCtrlIds,
										u32 uUserData,
										TalkInstDoneCallback_t* pfcnTalkDoneCallback /*= NULL */)
{
	if(pTalkInst == NULL)
	{
		// For some reason they requested to start talking with no talk instance data.
		return(FALSE);
	}
	//////////////////////////////////////////////////////////////////////
	// Check to see if we can find an available slot.
	u32 uCurTIIdx;
	for(uCurTIIdx = 0; (uCurTIIdx < CTalkSystem2_uMaxSimulTalkInst) && (m_apTalkInst[uCurTIIdx] != NULL); ++uCurTIIdx)
	{
		if(m_apTalkInst[uCurTIIdx] == pTalkInst)
		{
			DEVPRINTF("CTalkSystem2::SubmitTalkRequest() : Tried to play the same talk twice.\n");
			return(FALSE);
		}
	}

	if(uCurTIIdx == CTalkSystem2_uMaxSimulTalkInst)
	{
		DEVPRINTF("CTalkSystem2::SubmitTalkRequest() : Too many BotTalks going at once.\n");
		return(FALSE);
	}

	if(pTalkInst->GetTalkReason() == TALKACTIVITY2_NONAMBIENT)
	{
		// If this is the first non-ambient speech to start, tell the game.
		if(s_uNonAmbientCount == 0)
		{
			BOOL bRetVal = game_EnterNonAmbientTalk();
			if(!bRetVal)
			{
				return(FALSE);
			}
		}
		++s_uNonAmbientCount;
	}
	//
	//////////////////////////////////////////////////////////////////////
	if(!pTalkInst->Start(pCombiner,
						panTapIds,
						panCtrlIds,
						uUserData,
						pfcnTalkDoneCallback /*= NULL */))
	{
		return(FALSE);
	}

	m_apTalkInst[uCurTIIdx] = pTalkInst;

	return(TRUE);
}



BOOL CTalkSystem2::TerminateActiveTalk(CTalkInst *pTalkInst)
{
	//////////////////////////////////////////////////////////////////////
	// Check to see if we can find the CTalkInst anywhere.
	u32 uCurTIIdx;
	for(uCurTIIdx = 0; uCurTIIdx < CTalkSystem2_uMaxSimulTalkInst; ++uCurTIIdx)
	{
		if(m_apTalkInst[uCurTIIdx] == pTalkInst)
		{
			if(pTalkInst->GetTalkReason() == TALKACTIVITY2_NONAMBIENT)
			{
				FASSERT(s_uNonAmbientCount > 0);
				--s_uNonAmbientCount;
			}
			pTalkInst->End();
			m_apTalkInst[uCurTIIdx] = NULL;
			return(TRUE);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	return(FALSE);
}

















BOOL CTalkSystem2::ControlsAreCaptive()
{
	return(s_uNonAmbientCount > 0);
}



//
//
//  BTA and BTI management
//
//
CBotTalkData *CTalkSystem2::GetTalkDataByFileName(cchar *pszBTAFileName)
{
	CBotTalkData *pBTD = NULL;
	if(pszBTAFileName == NULL)
	{
		return(NULL);
	}

	//////////////////////////////////////////////////////////////////////
	// Let's check to see if we can find it in our already loaded pool
	CTS2Resource* pRes = (CTS2Resource*) flinklist_GetHead(&m_TalkResourcePool);
	while (pRes)
	{
		if (!fclib_stricmp(pRes->m_TalkData.m_pszSourceFileName, pszBTAFileName))
		{
			pBTD = &(pRes->m_TalkData);
			break;
		}
		pRes = (CTS2Resource*) flinklist_GetNext(&m_TalkResourcePool, pRes);
	}

	//
	//////////////////////////////////////////////////////////////////////

	if( !pRes )
	{
		pRes = fnew CTS2Resource();
		if( pRes ) {
			CBotTalkData *pBTD = &(pRes->m_TalkData);
			if(pBTD->LoadFromFile(pszBTAFileName))
			{
				flinklist_AddTail(&m_TalkResourcePool, pRes);
				++s_uNumTalkDatas;
				return(pBTD);
			}
			else
			{
				fdelete( pRes );
				return NULL;
			}
		}
	}

	return(pBTD);
}


CTS2Resource::CTS2Resource(void)
{
	flinklist_InitRoot(&m_AvailableBTIList, FANG_OFFSETOF( CBotTalkInst, CBotTalkInst::m_TS2Next ));
	flinklist_InitRoot(&m_HandedOutBTIList, FANG_OFFSETOF( CBotTalkInst, CBotTalkInst::m_TS2Next ));

	m_NextResource.pPrevLink = NULL;
	m_NextResource.pNextLink = NULL;
}

CTS2Resource::~CTS2Resource(void)
{
}

BOOL CTalkSystem2::BTIPool_Init(cchar *pszBTAFileName, u32 uNumToAlloc)
{
	CTS2Resource* pRes = NULL;
	CBotTalkData * pBTD = GetTalkDataByFileName(pszBTAFileName);

	if (pBTD)
	{
		pRes = (CTS2Resource*) flinklist_GetHead(&m_TalkResourcePool);
		while (pRes)
		{
			if (&(pRes->m_TalkData)== pBTD)
			{
				break;
			}
			pRes = (CTS2Resource*) flinklist_GetNext(&m_TalkResourcePool, pRes);
		}
		
		FASSERT(pRes);

		if (pRes)
		{
			u32 i;
			for ( i = pRes->m_AvailableBTIList.nCount; i < uNumToAlloc; i++)
			{
				CBotTalkInst *pInst = fnew CBotTalkInst();
				if (pInst)
				{
					pInst->Init(pBTD);
					flinklist_AddTail(&(pRes->m_AvailableBTIList), pInst);
				}
			}
			return (i == uNumToAlloc);	  //if all could be allocated, then return TRUE;
		}
	}
	return FALSE;
}


CBotTalkInst *CTalkSystem2::BTIPool_Get(cchar *pszBTAFileName)
{
	CBotTalkInst* pInst = NULL;
	if (pszBTAFileName)
	{
		CTS2Resource* pRes = (CTS2Resource*) flinklist_GetHead(&m_TalkResourcePool);
		while (pRes && fclib_stricmp(pRes->m_TalkData.m_pszSourceFileName, pszBTAFileName))
		{
			pRes = (CTS2Resource*) flinklist_GetNext(&m_TalkResourcePool, pRes);
		}

		if (pRes)
		{
			pInst = (CBotTalkInst*) flinklist_RemoveHead(&(pRes->m_AvailableBTIList));
			if (pInst)
			{
				flinklist_AddHead(&(pRes->m_HandedOutBTIList), pInst);
			}
		}
	}
	return pInst;
}


void CTalkSystem2::BTIPool_Recycle(CBotTalkInst* pInst)
{

	if (pInst)
	{
		FASSERT(pInst->GetTalkData());
		CTS2Resource* pRes = (CTS2Resource*) flinklist_GetHead(&m_TalkResourcePool);
		while (pRes && fclib_stricmp(pRes->m_TalkData.m_pszSourceFileName, pInst->GetTalkData()->m_pszSourceFileName))
		{
			pRes = (CTS2Resource*) flinklist_GetNext(&m_TalkResourcePool, pRes);
		}
		FASSERT(pRes); //returning something I don't know anything about

		if (pRes)
		{
			flinklist_Remove(&(pRes->m_HandedOutBTIList), pInst);
			flinklist_AddHead(&(pRes->m_AvailableBTIList), pInst);
		}
	}
}
