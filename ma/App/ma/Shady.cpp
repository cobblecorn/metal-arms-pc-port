//////////////////////////////////////////////////////////////////////////////////////
// Shady.cpp - Shady class for Mettle Arms.
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
// 05/30/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "Shady.h"
#include "meshentity.h"
#include "player.h"
#include "floop.h"
#include "fanim.h"
#include "BarterTypes.h"
#include "BarterSystem.h"
#include "AI/aifsm.h"
#include "BotTalkInst.h"


static const cchar *_pszMeshName = "GRDHshady00";
static const cchar *_apszAnimNames[SHADYANIM_COUNT] =
{ 
	"ARDHidle003",// SHADYANIM_IDLE1A
	"ARDHturnR01",// SHADYANIM_LOOKATSLIM
};

const Shady_BotTalkInfo_t Shady_aBotTalkInfo[CShady::SHADYSTATE_COUNT] = {
	"NONE",					CShady::SHADYSTATE_NONE,			0,	0,	0,
	"WORLD_IDLE",			CShady::SHADYSTATE_WORLDIDLE,		0,	0,	0,
	"SEE_PLAYER",			CShady::SHADYSTATE_SEEPLAYER,		0,	0,	0,
	"INTRO_PITCH",			CShady::SHADYSTATE_INTROPITCH,		0,	0,	0,
	"ITEM_PITCH",			CShady::SHADYSTATE_ITEMPITCH,		0,	0,	0,
	"BARTER_IDLE",			CShady::SHADYSTATE_BARTERIDLE,		0,	0,	0,
	"BARTER_IDLE_TALK",		CShady::SHADYSTATE_BARTERIDLETALK,	0,	0,	0,
	"PURCHASE_CONFIRM",		CShady::SHADYSTATE_PURCHASECONFIRM,	0,	0,	0,
	"PURCHASE_ABORT",		CShady::SHADYSTATE_PURCHASEABORT,	0,	0,	0,
	"UPSELL_PITCH",			CShady::SHADYSTATE_UPSELLPITCH,		0,	0,	0,
	"UPSELL_ABORT",			CShady::SHADYSTATE_UPSELLABORT,		0,	0,	0,
	"POST_PURCHASE",		CShady::SHADYSTATE_POSTPURCHASE,	0,	0,	0,
	"LEAVE_NO_PURCHASE",	CShady::SHADYSTATE_LEAVENOPURCHASE,	0,	0,	0,
	"LEAVE_WITH_PURCHASE",	CShady::SHADYSTATE_LEAVEWITHPURCHASE,0,	0,	0,
};


static const f32 _afPlayerSeeTime[2] = { 1.0f, 5.0f };
static const f32 _fAnimBlendTimeSecs = 0.20f;
static const f32 _fOOAnimBlendTimeSecs = 1.0f / _fAnimBlendTimeSecs;
static const f32 _kfOOSlimLookAtSumRate = 1.0f / 0.15f;

BarterTypes_TableData_t LastBarterTableState;

#define SCRAMBLE_BUFF_MAXNUM 10
static s32 _auScrambleBuff[SCRAMBLE_BUFF_MAXNUM];
static BOOL _GetRandomRangeExcept(s32 uMin, s32 uMax, s32* pauExceptions, u32 uNumExceptions, s32 *puResult)
{
	u32 uCount = 0;
	for (s32 i=uMin; i < uMax;i++, uCount < SCRAMBLE_BUFF_MAXNUM)
	{
		u32 j;
		for (j = 0; j < uNumExceptions; j++)
		{
			if (i == pauExceptions[j])
			{
				break;
			}
		}
		if (j == uNumExceptions)
		{
			_auScrambleBuff[uCount] = i;
			uCount++;
		}
	}

	if (uCount)
	{
		*puResult = _auScrambleBuff[fmath_RandomChoice(uCount)];
		return TRUE;
	}
	return FALSE;
}


//
//
//**New Interface functions for BarterSystem
//

BOOL CShady::ReadyToBarter(void)
{
	if (IsBusy())
	{
		int i = 2;
	}
	return !IsBusy();
}


BOOL CShady::Available(void)
{
	return TRUE;
}


BOOL CShady::IsBusy()
{
	if (m_bBSBusyOnPending) 
	{
		return TRUE;
	}
	return FALSE;
}


void CShady::Welcome( const BarterTypes_TableData_t *pNewTable, BOOL bFirstTime/*=FALSE*/ )
{
	LastBarterTableState = *pNewTable;
	if (bFirstTime)
	{
		ChangeState(SHADYSTATE_POSTPURCHASE);
		m_bBSBusyOnPending+=2;				  //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
		m_fDelayBsProgressTimeOut = 0.75f;	  //when
	}
	else
	{
		ChangeState(SHADYSTATE_INTROPITCH);
		m_bBSBusyOnPending+=2;				  //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
		m_fDelayBsProgressTimeOut = 100.0f;
	}
}


void CShady::NewSale(u32 nUpsaleIndex, const BarterTypes_TableData_t *pNewTable )
{
	LastBarterTableState = *pNewTable;
	//Current Item changed
	if (LastBarterTableState.bTableIsAnUpsale)
	{
		ChangeState(SHADYSTATE_UPSELLPITCH);
	}
	else
	{
		ChangeState(SHADYSTATE_ITEMPITCH);
	}
//	m_bBSBusyOnPending+=2;			  //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
//	m_fDelayBsProgressTimeOut = 100.0f;
}


//player changed the current selection
void CShady::SetCurrentSlot( u32 nSlotIndex, CBarterItemInst *pNewBII )
{
	LastBarterTableState.nCurrentSlot = nSlotIndex;
	if (pNewBII)
	{
		LastBarterTableState.apSlots[LastBarterTableState.nCurrentSlot] = pNewBII;
	}
	
	//Current Item changed
	if (LastBarterTableState.bTableIsAnUpsale)
	{
		ChangeState(SHADYSTATE_UPSELLPITCH);
	}
	else
	{
		ChangeState(SHADYSTATE_ITEMPITCH);
	}
//	m_bBSBusyOnPending+=2;				//2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
//	m_fDelayBsProgressTimeOut = 100.0f;
}


//player clicked buy and had enough money
void CShady::Purchase( u32 nUpsaleIndex )
{
	ChangeState(SHADYSTATE_PURCHASECONFIRM);
	m_bBSBusyOnPending+=2;			  //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
	m_fDelayBsProgressTimeOut = 100.0f;
}


//player clicked buy, but didn't have enough washers!
void CShady::FailedPurchase( u32 nUpsaleIndex )
{
	ChangeState(SHADYSTATE_PURCHASEABORT);
	m_bBSBusyOnPending+=2;			   //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
	m_fDelayBsProgressTimeOut = 100.0f;
}


//player was offered an upsell, but said NO
void CShady::RejectedItem( u32 nUpsaleIndex )
{
	FASSERT(nUpsaleIndex>0);
	ChangeState(SHADYSTATE_UPSELLABORT);
	m_bBSBusyOnPending+=2;			  //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
	m_fDelayBsProgressTimeOut = 100.0f;
}


void CShady::Exit( u32 nItemsPurchased )
{
	if (nItemsPurchased)
	{
		ChangeState(SHADYSTATE_LEAVEWITHPURCHASE);
	}
	else
	{
		ChangeState(SHADYSTATE_LEAVENOPURCHASE);
	}
//	m_bBSBusyOnPending+=2;			  //2 is because 2 things need to happen.  1st)this state is de-queued from 2n2 the pending que and then it needs to end
//	m_fDelayBsProgressTimeOut = 100.0f;
}

//when I get avents
void CShady::EventNotify( u32 nEvent, f32 fVal )
{
	if (nEvent == BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM)
	{
		BeginSlimLookAt();
	}
}

void CShady::ResetToPostCreateState( void )
{
	StopActiveResponses();

	u32 i = 0;
	m_pFsm->RemovePendingAndActiveStates();
	if (m_pOldInst)
	{
		m_pOldInst->End();
		m_pOldInst = NULL;
	}
	if (m_pCurInst)
	{
		m_pCurInst->End();
		m_pCurInst = NULL;
	}

	m_fNextIdle2Cntdn = 0.0f;
	m_fNextSeePlayerCntdn = 0.0f;
	m_fNextBarterIdleAudio = 0.0f;
	m_fWorldIdleTimeOut = 0.0f;
	m_bStateIsTalking = FALSE;
	m_bBSBusyOnPending = 0;
	m_fDelayBsProgressTimeOut = 0.0f;
	m_auMinTimeBetweenResp[SHADYSTATE_BARTERIDLE] = 0;
	m_auMaxTimeBetweenResp[SHADYSTATE_BARTERIDLE] = 2;
	m_fDistToPlayer = 0.0f;
	ChangeState(SHADYSTATE_WORLDIDLE);
	
	m_uLookAtSlimState = LOOKATSLIMSTATE_NONE;
	m_fLookAtSlimSumPct = 0.0f;
	m_fLookAtTimeOut = 0.0f;
	
	for (i = 0; i < SHADYSTATE_COUNT; i++)
	{
		m_afRespDelayTimeOut[i] = 0.0f;
		m_apLastResp[i] = NULL;
		m_auResponseCountThisState[i] = 0;
	}

	m_uFlags = SHADYFLAG_CREATED;
}

//**New Interface functions for BarterSystem
//
//


// Creation, initialization, and destruction.
CShady::CShady()
{
	m_pMeshEntity = NULL;
	m_uFlags = 0;
	m_fDistToPlayer = 0.0f;
	m_fDelayBsProgressTimeOut = 0.0f;
	m_fNextIdle2Cntdn = 0.0f;
	m_fNextSeePlayerCntdn = 0.0f;
	m_fNextBarterIdleAudio = 0.0f;
	m_fWorldIdleTimeOut = 0.0f;
	m_fLookAtTimeOut = 0.0f;
	m_uPlayerId = 0;
	m_pFsm = NULL;
	m_pOldInst = NULL;
	m_pCurInst = NULL;
	m_nCurInLowHi = 0;
	m_nOldInLowHi = 1-m_nCurInLowHi;
	m_uLookAtSlimState = LOOKATSLIMSTATE_NONE;
	m_fOOBlendTimeSecs = 0.0f;
	m_fLookAtSlimSumPct = 0.0f;
	m_fLookAtAnimTime = 0.0f;
	m_bStateIsTalking = FALSE;
	m_bBSBusyOnPending = 0;

	u32 i;
	for (i = 0; i < SHADYSTATE_COUNT; i++)
	{
		m_auMinTimeBetweenResp[i] = 0;
		m_auMaxTimeBetweenResp[i] = 0;
		m_anAutoChainState[i] = 0;
		m_auAutoChainAfterNResponses[i] = 0;
		m_auResponseCountThisState[i] = 0;
		m_afRespDelayTimeOut[i] = 0.0f;
		m_apLastResp[i] = NULL;
	}

	for (i = 0; i < 2; i++)
	{
		m_anLowHiControlId[i] = 0;
		m_apaBtControlId[i] =  0;
		m_apaBtTapId[i] = 0;
	}
}


CShady::~CShady()
{
	FASSERT(m_pMeshEntity == NULL);
	m_pMeshEntity = NULL;

	FASSERT(m_pFsm==NULL);
}


CFSMStateDef s_aShadyFSMStateDefs[] = 
{
	CFSMStateDef("StateNone",			CShady::_InitCB_StateNone,			CShady::_WorkCB_StateNone,			CShady::_CleanupCB_StateNone,					CShady::SHADYSTATE_NONE,				0, NULL),
	CFSMStateDef("WorldIdle",			CShady::_InitCB_WorldIdle,			CShady::_WorkCB_WorldIdle,			CShady::_CleanupCB_WorldIdle,					CShady::SHADYSTATE_WORLDIDLE ,			0, NULL),
	CFSMStateDef("SeePlayer",			CShady::_InitCB_SeePlayer,			CShady::_WorkCB_SeePlayer,			CShady::_CleanupCB_SeePlayer,					CShady::SHADYSTATE_SEEPLAYER,			0, NULL),
	CFSMStateDef("IntroPitch",			CShady::_InitCB_IntroPitch,			CShady::_WorkCB_IntroPitch,			CShady::_CleanupCB_IntroPitch,					CShady::SHADYSTATE_INTROPITCH,			0, NULL),
	CFSMStateDef("ItemPitch",			CShady::_InitCB_ItemPitch,			CShady::_WorkCB_ItemPitch,			CShady::_CleanupCB_ItemPitch,					CShady::SHADYSTATE_ITEMPITCH,			0, NULL),
	CFSMStateDef("BarterIdle",			CShady::_InitCB_BarterIdle,			CShady::_WorkCB_BarterIdle,			CShady::_CleanupCB_BarterIdle,					CShady::SHADYSTATE_BARTERIDLE,			0, NULL),
	CFSMStateDef("BarterIdleTalk",		CShady::_InitCB_BarterIdleTalk,		CShady::_WorkCB_BarterIdleTalk,		CShady::_CleanupCB_BarterIdleTalk,				CShady::SHADYSTATE_BARTERIDLETALK,		0, NULL),
	CFSMStateDef("PurchaseConfirm",		CShady::_InitCB_PurchaseConfirm,	CShady::_WorkCB_PurchaseConfirm,	CShady::_CleanupCB_PurchaseConfirm,				CShady::SHADYSTATE_PURCHASECONFIRM,		0, NULL),
	CFSMStateDef("PurchaseAbort",		CShady::_InitCB_PurchaseAbort,		CShady::_WorkCB_PurchaseAbort,		CShady::_CleanupCB_PurchaseAbort,				CShady::SHADYSTATE_PURCHASEABORT,		0, NULL),
	CFSMStateDef("UpsellPitch",			CShady::_InitCB_UpsellPitch,		CShady::_WorkCB_UpsellPitch,		CShady::_CleanupCB_UpsellPitch,					CShady::SHADYSTATE_UPSELLPITCH,			0, NULL),
	CFSMStateDef("UpsellAbort",			CShady::_InitCB_UpsellAbort,		CShady::_WorkCB_UpsellAbort,		CShady::_CleanupCB_UpsellAbort,					CShady::SHADYSTATE_UPSELLABORT,			0, NULL),
	CFSMStateDef("PostPurchase",		CShady::_InitCB_PostPurchase,		CShady::_WorkCB_PostPurchase,		CShady::_CleanupCB_PostPurchase,				CShady::SHADYSTATE_POSTPURCHASE,		0, NULL),
	CFSMStateDef("LeaveNoPurchase",		CShady::_InitCB_LeaveNoPurchase,	CShady::_WorkCB_LeaveNoPurchase,	CShady::_CleanupCB_LeaveNoPurchase,				CShady::SHADYSTATE_LEAVENOPURCHASE,		0, NULL),
	CFSMStateDef("LeaveWithPurchase",	CShady::_InitCB_LeaveWithPurchase,	CShady::_WorkCB_LeaveWithPurchase,	CShady::_CleanupCB_LeaveWithPurchase,			CShady::SHADYSTATE_LEAVEWITHPURCHASE,	0, NULL),
};


BOOL CShady::Create(f32 fSpeechDistance)
{
	u32 i;


	FASSERT(!(m_uFlags & SHADYFLAG_CREATED));

	FASSERT(!m_pOldInst);
	FASSERT(!m_pCurInst);


	FResFrame_t hResFrame = fres_GetFrame();
	
	m_pFsm = fnew CFSM();
	if (!m_pFsm)
	{
		goto _ExitWithError;
	}
	
	m_pMeshEntity = fnew CMeshEntity;
	if( !m_pMeshEntity  ||
		!m_pMeshEntity->Create(	_pszMeshName,		//cchar *pszMeshName, 
								SHADYANIM_COUNT,	//u32 nAnimCount, 
								_apszAnimNames,		//cchar **ppszAnimNames, 
								"Shady",			//cchar *pszEntityName=NULL, 
								NULL,				//const CFMtx43A *pMtx=NULL, 
								NULL,				//cchar *pszAIBuilderName=NULL, 
								TRUE ) )			//BOOL bUseAnimBlending=FALSE )
	{
		goto _ExitWithError;
	}

	if (!m_ShadyAnim.Create(m_pMeshEntity->GetMeshInst()))
	{
		goto _ExitWithError;
	}
	m_pMeshEntity->SetMeshInstFlags( FMESHINST_FLAG_CAST_SHADOWS );
	m_pMeshEntity->DriveMeshWithAnim(TRUE);
	m_pMeshEntity->SetExternalCombiner(0, m_ShadyAnim.GetCombiner());
	m_ShadyAnim.GetCombiner()->AttachToTap(m_ShadyAnim.m_anIdleTapID[0], m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_IDLE1A));
	m_ShadyAnim.GetCombiner()->AttachToTap(m_ShadyAnim.m_nBTLowIdle, m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_IDLE1A));
	m_ShadyAnim.GetCombiner()->AttachToTap(m_ShadyAnim.m_nBTHighIdle, m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_IDLE1A));
	m_ShadyAnim.GetCombiner()->AttachToTap(m_ShadyAnim.m_anIdleTapID[1], m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM));

	for( i=0; i<4; ++i )
	{
		m_ShadyAnim.GetCombiner()->EnableControlSmoothing( m_ShadyAnim.m_anBTHighControlID[i] );
		m_ShadyAnim.GetCombiner()->EnableControlSmoothing( m_ShadyAnim.m_anBTLowControlID[i] );
	}
	m_ShadyAnim.GetCombiner()->EnableControlSmoothing( m_ShadyAnim.m_nBTHighControlID );
	m_ShadyAnim.GetCombiner()->EnableControlSmoothing( m_ShadyAnim.m_nBTLowControlID );
	
	//lookup tables
	m_anLowHiControlId[0] = m_ShadyAnim.m_nBTLowControlID;
	m_anLowHiControlId[1] = m_ShadyAnim.m_nBTHighControlID;
	m_apaBtControlId[0] = m_ShadyAnim.m_anBTLowControlID;
	m_apaBtControlId[1] = m_ShadyAnim.m_anBTHighControlID;
	m_apaBtTapId[0] = m_ShadyAnim.m_anBTLowTapID;
	m_apaBtTapId[1] = m_ShadyAnim.m_anBTHighTapID;

	for (i = 0; i < SHADYSTATE_COUNT; i++)
	{
		m_auMinTimeBetweenResp[i] = 2;
		m_auMaxTimeBetweenResp[i] = 7;
		m_anAutoChainState[i] = NO_AUTO_CHAIN;
		m_auAutoChainAfterNResponses[i] = 0;
	}
	
	m_anAutoChainState[SHADYSTATE_INTROPITCH] = SHADYSTATE_BARTERIDLE;
	m_anAutoChainState[SHADYSTATE_ITEMPITCH] = SHADYSTATE_BARTERIDLE;
   	m_anAutoChainState[SHADYSTATE_PURCHASECONFIRM] = SHADYSTATE_BARTERIDLE;
	m_anAutoChainState[SHADYSTATE_PURCHASEABORT] = SHADYSTATE_BARTERIDLE;
	m_anAutoChainState[SHADYSTATE_UPSELLPITCH] = SHADYSTATE_BARTERIDLE;			
	m_anAutoChainState[SHADYSTATE_UPSELLABORT] = SHADYSTATE_BARTERIDLE;
	m_anAutoChainState[SHADYSTATE_POSTPURCHASE] = SHADYSTATE_BARTERIDLE;
	m_anAutoChainState[SHADYSTATE_BARTERIDLE] = SHADYSTATE_BARTERIDLETALK;
	m_anAutoChainState[SHADYSTATE_BARTERIDLETALK] = SHADYSTATE_BARTERIDLE;
	m_anAutoChainState[SHADYSTATE_LEAVENOPURCHASE] = SHADYSTATE_WORLDIDLE;
	m_anAutoChainState[SHADYSTATE_LEAVEWITHPURCHASE] = SHADYSTATE_WORLDIDLE;
	m_anAutoChainState[SHADYSTATE_SEEPLAYER] = SHADYSTATE_WORLDIDLE;

	m_auAutoChainAfterNResponses[SHADYSTATE_SEEPLAYER] = 2;
	m_auAutoChainAfterNResponses[SHADYSTATE_INTROPITCH] = 1;
	m_auAutoChainAfterNResponses[SHADYSTATE_ITEMPITCH] = 1;
   	m_auAutoChainAfterNResponses[SHADYSTATE_PURCHASECONFIRM] = 1;
	m_auAutoChainAfterNResponses[SHADYSTATE_PURCHASEABORT] = 1;
	m_auAutoChainAfterNResponses[SHADYSTATE_UPSELLPITCH] = 1;			
	m_auAutoChainAfterNResponses[SHADYSTATE_UPSELLABORT] = 1;
	m_auAutoChainAfterNResponses[SHADYSTATE_POSTPURCHASE] = 1;
	m_auAutoChainAfterNResponses[SHADYSTATE_BARTERIDLE] = 1;
	m_auAutoChainAfterNResponses[SHADYSTATE_BARTERIDLETALK] = 1;
	m_anAutoChainState[SHADYSTATE_LEAVENOPURCHASE] = 1;
	m_anAutoChainState[SHADYSTATE_LEAVEWITHPURCHASE] = 1;

	ResetToPostCreateState();
	
	m_fSpeechRadius = fSpeechDistance;

	return(TRUE);

_ExitWithError:
	fdelete(m_pMeshEntity);
	m_pMeshEntity = NULL;

	fdelete (m_pFsm);
	m_pFsm = NULL;

	fres_ReleaseFrame( hResFrame );

	// don't force the whole game to stop, just disable our system and return TRUE
	DEVPRINTF("CShady::Create() : Out of memory.\n");
	return TRUE;	
}

void CShady::StopActiveResponses(void)
{
	CTalkInst* apTalkInsts[2] = {m_pOldInst, m_pCurInst};
	for (u32 i = 0; i <2; i++)
	{
		if (apTalkInsts[i])
		{
			if (apTalkInsts[i]->IsWorking() || !apTalkInsts[i]->IsAtEnd())
			{
				apTalkInsts[i]->End();
			}
		}
		if (m_ShadyAnim.GetCombiner())
		{
			m_ShadyAnim.GetCombiner()->SetControlValue(m_anLowHiControlId[i], 0.0f);
		}
	}
	
	m_pOldInst = NULL;
	m_pCurInst = NULL;
}


void CShady::Destroy()
{
	StopActiveResponses();

	if( m_pMeshEntity )
	{
		m_pMeshEntity->Destroy();
		fdelete( m_pMeshEntity );
		m_pMeshEntity = NULL;
	}
	
	m_ShadyAnim.Destroy();

	if (m_pFsm)
	{
		m_pFsm->RemovePendingAndActiveStates();
	}
	fdelete (m_pFsm);
	m_pFsm = NULL;

	
	m_uFlags &=~SHADYFLAG_CREATED;
}


const CFSphere& CShady::GetBoundingSphere(void)
{
	FASSERT(m_pMeshEntity);
	return m_pMeshEntity->GetBoundingSphere_WS();
}


void CShady::AddToWorld()
{
	FASSERT(IsCreated());
	m_pMeshEntity->AddToWorld();
}


void CShady::RemoveFromWorld()
{
	FASSERT(IsCreated());
	m_pMeshEntity->RemoveFromWorld();
}


void CShady::Relocate_WS( const CFMtx43A *pMtx_WS )
{
	FASSERT(IsCreated());
	m_pMeshEntity->Relocate_RotXlatFromUnitMtx_WS(pMtx_WS);
}


void CShady::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	m_pMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


void CShady::ChangeState(s32 nPendingState)
{
	m_pFsm->ChangeState(s_aShadyFSMStateDefs[nPendingState]);
}


CShady::ShadyState_e CShady::GetCurState(void)
{
	CShady::ShadyState_e uShadyState = (CShady::ShadyState_e) m_pFsm->GetActiveStateHandle();
	if (!(uShadyState >= SHADYSTATE_NONE && uShadyState < SHADYSTATE_COUNT))
	{
		uShadyState	= SHADYSTATE_NONE;
	}

	return uShadyState;
}


BOOL CShady::IsStateChangePending(void)
{
	return m_pFsm->IsStateChangePending();
}


void CShady::BeginSlimLookAt(void)
{
	m_uLookAtSlimState = LOOKATSLIMSTATE_WINDUP;
}


void CShady::EndSlimLookAt(void)
{
	if (m_uLookAtSlimState != LOOKATSLIMSTATE_NONE)
	{
		m_uLookAtSlimState = LOOKATSLIMSTATE_UNWIND;
	}
}


void CShady::StartTalkInst( CTalkInst* pInst, f32 fBlendInTimeSecs )
{
	CTalkInst* pPrevInst = NULL;

	if( pInst == m_pCurInst )
	{
		return;
	}

	if (fBlendInTimeSecs > 0.0f)
	{
		m_fOOBlendTimeSecs = 1.0f/fBlendInTimeSecs;
	}
	else
	{
		m_fOOBlendTimeSecs = 1.0f;
	}

	if( pInst == m_pOldInst && pInst)
	{
		m_pCurInst = m_pOldInst;
		m_nCurInLowHi = m_nOldInLowHi;
		m_nOldInLowHi = 1 - m_nOldInLowHi;
		m_pOldInst = NULL;
		return;
	}
	
	if (m_pCurInst)
	{
		m_pOldInst = m_pCurInst;
		m_nOldInLowHi = m_nCurInLowHi;
		m_nCurInLowHi = 1 - m_nCurInLowHi;
	}
	m_pCurInst = pInst;

	if (m_pCurInst)
	{
		m_pCurInst->Set3DPos( m_pMeshEntity->MtxToWorld()->m_vPos,m_fSpeechRadius );
		m_pCurInst->Start( m_ShadyAnim.GetCombiner(),m_apaBtTapId[m_nCurInLowHi],  m_apaBtControlId[m_nCurInLowHi] );
	}
	m_ShadyAnim.GetCombiner()->SetControlValue(m_anLowHiControlId[m_nCurInLowHi], 0.0f);

}


void CShady::AnimWork(void)
{
	f32 fCurCtrlVal = 0.0f;
	f32 fOldCtrlVal = 0.0f;

	//blend out the old
	if (m_pOldInst)
	{
		fOldCtrlVal = m_ShadyAnim.GetCombiner()->GetControlValue(m_anLowHiControlId[m_nOldInLowHi]);
		fOldCtrlVal -= FLoop_fPreviousLoopSecs*m_fOOBlendTimeSecs;
		if (fOldCtrlVal <0.0f)
		{
			if (m_pOldInst->IsWorking() || !m_pOldInst->IsAtEnd())
			{
				m_pOldInst->End();
			}
			m_pOldInst = NULL;
			fOldCtrlVal = 0.0f;
		}
		m_ShadyAnim.GetCombiner()->SetControlValue(m_anLowHiControlId[m_nOldInLowHi], fOldCtrlVal);
	}

	//blend in the new
	fCurCtrlVal = m_ShadyAnim.GetCombiner()->GetControlValue(m_anLowHiControlId[m_nCurInLowHi]);
	if (m_pCurInst && fCurCtrlVal < 1.0f)
	{
		fCurCtrlVal += FLoop_fPreviousLoopSecs*m_fOOBlendTimeSecs;
		if (fCurCtrlVal > 1.0f)
		{
			fCurCtrlVal = 1.0f;
		}
		m_ShadyAnim.GetCombiner()->SetControlValue(m_anLowHiControlId[m_nCurInLowHi], fCurCtrlVal);
	}

	if (m_pOldInst)
	{
		m_pOldInst->Work();

		if (!m_pOldInst->IsWorking() || m_pOldInst->IsAtEnd())
		{
			m_pOldInst = NULL;
		}
	}
	else
	{
		m_ShadyAnim.GetCombiner()->SetControlValue(m_anLowHiControlId[m_nOldInLowHi], 0.0f);
	}

	if (m_pCurInst)
	{
		m_pCurInst->Work();

		if (!m_pCurInst->IsWorking() || m_pCurInst->IsAtEnd())
		{
			m_pCurInst = NULL;
		}
	}
	else
	{
		m_ShadyAnim.GetCombiner()->SetControlValue(m_anLowHiControlId[m_nCurInLowHi], 0.0f);
	}
}



CBarterResponse * CShady::GetStateResponse(void)
{
	ShadyState_e	uCurState = GetCurState();
	CBarterResponse *apaRespTable[2] = {NULL,NULL};
	u32				auRespTableSize[2] = {0,0};
	u8				*apauRespPlayFlags[2] = {NULL,NULL};
	u8				*apauRespSortOrder[2] = {NULL,NULL};
	u32				uTablesToSearch = 0;

	if (uCurState == SHADYSTATE_ITEMPITCH || uCurState == SHADYSTATE_UPSELLPITCH)
	{
		apaRespTable[uTablesToSearch] = LastBarterTableState.apSlots[LastBarterTableState.nCurrentSlot]->m_poItem->m_paResponses;
		auRespTableSize[uTablesToSearch] = LastBarterTableState.apSlots[LastBarterTableState.nCurrentSlot]->m_poItem->m_nNumResponses;
		apauRespPlayFlags[uTablesToSearch] = LastBarterTableState.apSlots[LastBarterTableState.nCurrentSlot]->m_poItem->m_pauPlayFlags;
		apauRespSortOrder[uTablesToSearch] = LastBarterTableState.apSlots[LastBarterTableState.nCurrentSlot]->m_poItem->m_pauSortOrder;
		uTablesToSearch++;	
	}
	apaRespTable[uTablesToSearch]      = CBarterItem::m_paGenericResponses;
	auRespTableSize[uTablesToSearch]   = CBarterItem::m_nNumGenericResponses;
	apauRespPlayFlags[uTablesToSearch] = CBarterItem::m_pauGenericPlayFlags;
	apauRespSortOrder[uTablesToSearch] = CBarterItem::m_pauGenericSortOrder;
	uTablesToSearch++;
	u32 uTable = 0;
	u32 i;
	BOOL bSpinAgain = FALSE;

	while (uTable < uTablesToSearch)
	{
		s32 nNumStateMatches = 0;
		for (i = 0; i < auRespTableSize[uTable]; i++)
		{
			u32 uResp = apauRespSortOrder[uTable][i];
			if (apaRespTable[uTable][uResp].m_nShadyState == uCurState)
			{
				nNumStateMatches++;

				if (!apauRespPlayFlags[uTable][uResp])
				{	//found a match that hasn't been played
					apauRespPlayFlags[uTable][uResp] = TRUE;		   //Mark as played
					if (fmath_RandomChoice(100) < apaRespTable[uTable][uResp].m_nPercentChance)  //dice roll
					{
						return NULL;   //skipped
					}
					apaRespTable[uTable][uResp].m_bEventFired = FALSE;	//clear this whenever starting to use a response
					return &(apaRespTable[uTable][uResp]);
				}
			}
		}

		//got to the end of the table without finding a play-able match
		if (!bSpinAgain && 
			uTable == uTablesToSearch-1)	   //don't clear or scramble the item specific tables (they stay dirty until a new barter session is started)
		{
			if (nNumStateMatches > 1)
			{
				BarterTypes_ScrambleSortList(apauRespSortOrder[uTable], apaRespTable[uTable], auRespTableSize[uTable], uCurState, m_apLastResp[uCurState]);
			}

			//clear the play flags in this state
			if (nNumStateMatches)
			{
				for (i = 0; i < auRespTableSize[uTable]; i++)
				{
					if (apaRespTable[uTable][i].m_nShadyState == uCurState)
					{
						apauRespPlayFlags[uTable][i] = 0;
					}
				}
				bSpinAgain = TRUE;
			}
		}
		else
		{
			bSpinAgain = FALSE;
		}

		if (!bSpinAgain)
		{
			uTable++;
		}
	}

	return NULL;
}

BOOL CShady::DoDefaultStateInit(void)
{
	u32 uCurState = GetCurState();
	m_auResponseCountThisState[uCurState] = 0;
	m_afRespDelayTimeOut[uCurState] = 0.2f;	   //default is to delay the first talk 0.2secs
	
	if (m_pCurInst)
	{
		StartTalkInst(NULL, 0.0f);
		m_bStateIsTalking = FALSE;
	}

	return CFSM::INITSTATE_DONE;
}



void CShady::DoDefaultStateWork(void)
{
	ShadyState_e uCurState = GetCurState();
	CBarterResponse *pResp = NULL;
	BOOL bNoResponceForState = FALSE;
	if (!m_pCurInst && m_afRespDelayTimeOut[uCurState] <= 0.0f)
	{
		pResp = GetStateResponse();
		if (pResp)
		{
			StartTalkInst(pResp->m_pInst, 0.25f);
			m_afRespDelayTimeOut[uCurState] = (f32) m_auMinTimeBetweenResp[uCurState];
			if (m_auMaxTimeBetweenResp[uCurState] > m_auMinTimeBetweenResp[uCurState])
			{
			   m_afRespDelayTimeOut[uCurState] +=	(f32) fmath_RandomChoice(m_auMaxTimeBetweenResp[uCurState] - m_auMinTimeBetweenResp[uCurState]);
			}
			m_bStateIsTalking = TRUE;
		}
		else
		{
			bNoResponceForState = TRUE;
		}

		m_apLastResp[uCurState] = pResp;
	}

	if (!m_pCurInst)
	{
		if (m_bStateIsTalking || bNoResponceForState)
		{
			//A BTA Just ended
			m_bStateIsTalking = FALSE;
			if (m_bBSBusyOnPending)
			{
				m_bBSBusyOnPending--;
			}

			m_auResponseCountThisState[uCurState]++;

			if (!IsStateChangePending()	&&		 //Auto Chains never override any pending state changes
				m_anAutoChainState[uCurState] != NO_AUTO_CHAIN &&
				m_auAutoChainAfterNResponses[uCurState] <= m_auResponseCountThisState[uCurState])
			{
				ChangeState(m_anAutoChainState[uCurState]);
			}
		}

		m_afRespDelayTimeOut[uCurState] -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN(m_afRespDelayTimeOut[uCurState], 0.0f);

	}
	else
	{
		FASSERT(m_pCurInst);
		f32 fTimeUnit = m_pCurInst->GetTimeUnit();
		if (m_bBSBusyOnPending==1 && fTimeUnit > m_fDelayBsProgressTimeOut)
		{
			m_bBSBusyOnPending--;
		}
		
		if (m_apLastResp[uCurState]->m_nEventID &&
			m_apLastResp[uCurState] &&
			!m_apLastResp[uCurState]->m_bEventFired)
		{
			if (fTimeUnit > (m_apLastResp[uCurState]->m_nPercentCompleteToFireOffEvent)*(1.0f/100.0f))
			{
				bartersystem_EventNotify( m_apLastResp[uCurState]->m_nEventID, 0.0f );
				m_apLastResp[uCurState]->m_bEventFired = TRUE;	//will be reset whenever this event is used again
			}
		}

	}
}


void CShady::HandleLookAtSlim(void)
{
	switch (m_uLookAtSlimState)
	{
	   case LOOKATSLIMSTATE_WINDUP:
		   //Increase LookAt Animations contribution. Peg at 1.0f
		   m_fLookAtSlimSumPct += FLoop_fPreviousLoopSecs*_kfOOSlimLookAtSumRate;
		   FMATH_CLAMPMAX(m_fLookAtSlimSumPct, 1.0f);

		   //Increase Animation Time, peg at 1.0
			if (m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM))
			{
				m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM)->DeltaTime(FLoop_fPreviousLoopSecs, TRUE);
				if (m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM)->GetUnitTime() >= 1.0f)
				{	//true means done!
					m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM)->UpdateTime(1.0f);
					m_uLookAtSlimState = LOOKATSLIMSTATE_THERE;
					m_fLookAtTimeOut = 3.0f;
				}
			}
			break;
	   case LOOKATSLIMSTATE_THERE:
			m_fLookAtTimeOut -= FLoop_fPreviousLoopSecs;
		   break;
	   case LOOKATSLIMSTATE_UNWIND:
		   //Decrease LookAt Animations contribution. (Peg at 0.0f
		   m_fLookAtSlimSumPct -= FLoop_fPreviousLoopSecs*_kfOOSlimLookAtSumRate;
		   FMATH_CLAMPMIN(m_fLookAtSlimSumPct, 0.0f);

			if (m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM))
			{
				m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM)->DeltaTime(-FLoop_fPreviousLoopSecs, TRUE);
				if (m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM)->GetUnitTime() <= 0.0f)
				{	//true means done!
					m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_LOOKATSLIM)->UpdateTime(0.0f);
					m_uLookAtSlimState = LOOKATSLIMSTATE_NONE;
				}
			}
		   
		   break;
	}
	m_ShadyAnim.GetCombiner()->SetControlValue(m_ShadyAnim.m_nIdleControlID, m_fLookAtSlimSumPct);
}


void CShady::Work( BOOL bCanSeePlayer, f32 fDistSqToPlayer, u32 uPlayerId )
{
	BOOL bCouldSeePlayer = m_uFlags & SHADYFLAG_CANSEEPLAYER;
	m_uFlags &= ~SHADYFLAG_CANSEEPLAYER;
	m_uFlags |= SHADYFLAG_CANSEEPLAYER*bCanSeePlayer;
	m_fDistToPlayer = 0.0f;
	if (fDistSqToPlayer > 0.0f)
	{
		m_fDistToPlayer = fmath_Sqrt(fDistSqToPlayer);
	}
	m_uPlayerId = uPlayerId;
	
	if (GetCurState()==SHADYSTATE_WORLDIDLE && bCanSeePlayer && m_fDistToPlayer > 20.0f)
	{
		if (m_fNextSeePlayerCntdn <= 0.0f) 
		{
			if( m_uFlags & SHADYFLAG_CANSEEPLAYER )
			{
				ChangeState(SHADYSTATE_SEEPLAYER);
			}
		}
	}

	m_fNextSeePlayerCntdn -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN(m_fNextSeePlayerCntdn, 0.0f);
	
	m_fNextIdle2Cntdn -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN(m_fNextIdle2Cntdn, 0.0f);

	m_fNextBarterIdleAudio -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN(m_fNextBarterIdleAudio, 0.0f);

	if (m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_IDLE1A))
	{
		m_pMeshEntity->UserAnim_GetInstByIndex(SHADYANIM_IDLE1A)->DeltaTime(FLoop_fPreviousLoopSecs);
	}

	BOOL bOldStatePending = m_pFsm->IsStateChangePending();
	m_pFsm->Work((void*)this);
	if (m_bBSBusyOnPending && bOldStatePending &&! m_pFsm->IsStateChangePending())
	{
		//state popped off pending queue!
		m_bBSBusyOnPending--;
	}


	AnimWork();
	HandleLookAtSlim();

/*	test code to move and rotate shady
	CFVec3A m_MountPos_WS;
	m_MountPos_WS = m_pMeshEntity->MtxToWorld()->m_vPos;
	m_MountPos_WS.y=1.15f;
	
	f32 m_fMountYaw_WS = fmath_Atan( m_pMeshEntity->MtxToWorld()->m_vFront.x, m_pMeshEntity->MtxToWorld()->m_vFront.z );
	m_fMountYaw_WS+=0.10f;
	m_pMeshEntity->MtxToWorld()->SetRotationY( m_fMountYaw_WS );
	m_pMeshEntity->MtxToWorld()->m_vPos = m_MountPos_WS;
	m_pMeshEntity->Relocate_Xlat_WS(&(m_MountPos_WS), FALSE);
*/
}


BOOL CShady::_InitCB_StateNone(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_InitCB_WorldIdle(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_InitCB_SeePlayer(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_InitCB_IntroPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;

	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_InitCB_ItemPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	ShadyState_e uCurState = pS->GetCurState();
	pS->m_afRespDelayTimeOut[uCurState] = 1.0f;	   //always delay the pitch for one second
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_BarterIdle(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();

	ShadyState_e uCurState = pS->GetCurState();
	pS->m_auAutoChainAfterNResponses[uCurState] = 1 + (u8)fmath_RandomChoice(3);

	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_BarterIdleTalk(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	ShadyState_e uCurState = pS->GetCurState();
	pS->m_afRespDelayTimeOut[uCurState] = 1.0f;	   //always delay the pitch for one second
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_PurchaseConfirm(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_PurchaseAbort(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_UpsellPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_UpsellAbort(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_PostPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_LeaveNoPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}

BOOL CShady::_InitCB_LeaveWithPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateInit();
	return CFSM::INITSTATE_DONE;
}




BOOL CShady::_WorkCB_StateNone(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_WorldIdle(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();

	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_SeePlayer(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_IntroPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_ItemPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_BarterIdle(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_BarterIdleTalk(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_PurchaseConfirm(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_PurchaseAbort(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_UpsellPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_UpsellAbort(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_PostPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_LeaveNoPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}


BOOL CShady::_WorkCB_LeaveWithPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	CShady* pS = (CShady*) pvParam2;
	pS->DoDefaultStateWork();
	return CFSM::INITSTATE_NOTDONE;
}

BOOL CShady::_CleanupCB_StateNone(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_WorldIdle(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_SeePlayer(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_IntroPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_ItemPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_BarterIdle(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_BarterIdleTalk(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_PurchaseConfirm(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_PurchaseAbort(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_UpsellPitch(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_UpsellAbort(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_PostPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_LeaveNoPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


BOOL CShady::_CleanupCB_LeaveWithPurchase(u32 uControlParam, void *pvShady, void *pvParam2)
{
	return CFSM::INITSTATE_DONE;
}


