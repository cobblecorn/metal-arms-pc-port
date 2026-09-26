//////////////////////////////////////////////////////////////////////////////////////
// Shady.h - Shady class for Mettle Arms.
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
#ifndef _SHADY_H_
#define _SHADY_H_ 1

#include "fang.h"
#include "BarterTypes.h"
#include "ShadyAnim.h"
#include "fworld.h"

class CMeshEntity;
class CFMtx43A;
class CFAudioEmitter;
class CBarterItemInst;
class CFAnimInst;
class CFSM;
class CTalkInst;


enum
{
	SHADYANIM_IDLE1A = 0,
	SHADYANIM_LOOKATSLIM,
	SHADYANIM_COUNT
};


FCLASS_ALIGN_PREFIX class CShady
{
public:
	CShady();
	~CShady();

	BOOL Create(f32 fSpeechDistance);
	void Destroy();

	void Work( BOOL bCanSeePlayer, f32 fDistSqToPlayer, u32 uPlayerId = 0 );

	void AddToWorld();
	void RemoveFromWorld();
	void Relocate_WS(const CFMtx43A *pMtx_WS);
	void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	BOOL IsCreated() { return((m_uFlags & SHADYFLAG_CREATED) != 0); }
	
	//**New Interface functions for BarterSystem
	BOOL ReadyToBarter( void );
	BOOL Available( void );
	BOOL IsBusy(void);
	void Welcome( const BarterTypes_TableData_t *pTable, BOOL bFirstTime=FALSE );
	void SetCurrentSlot( u32 nSlotIndex, CBarterItemInst *pNewBII );
	void NewSale(u32 nUpsaleIndex, const BarterTypes_TableData_t *pNewTable );
	void Purchase( u32 nUpsaleIndex );
	void FailedPurchase( u32 nUpsaleIndex );
	void RejectedItem( u32 nUpsaleIndex );
	void Exit( u32 nItemsPurchased );
	void EventNotify( u32 nEvent, f32 fVal );

	void ResetToPostCreateState( void );
	
	void Set3dAudioRadius( f32 fSpeechRadius) { m_fSpeechRadius = fSpeechRadius;}

	//**End new BarterSystem interface

	const CFSphere& GetBoundingSphere(void);
	
	enum ShadyState_e
	{
		SHADYSTATE_NONE = 0,			// Special state that should only be temporary.
		SHADYSTATE_WORLDIDLE ,			// when Shady is out in the world
		SHADYSTATE_SEEPLAYER,			// when Shady is out in the world and can see the player
		SHADYSTATE_INTROPITCH,			// when the player first activates the barter droids
		SHADYSTATE_ITEMPITCH,			// when the player has just moved the cursor over an item and Shady
		SHADYSTATE_BARTERIDLE,			// mute idle while bartering
		SHADYSTATE_BARTERIDLETALK,		// talking idle while bartering
		SHADYSTATE_PURCHASECONFIRM,
		SHADYSTATE_PURCHASEABORT,
		SHADYSTATE_UPSELLPITCH,			
		SHADYSTATE_UPSELLABORT,
		SHADYSTATE_POSTPURCHASE,
		SHADYSTATE_LEAVENOPURCHASE,
		SHADYSTATE_LEAVEWITHPURCHASE,
		SHADYSTATE_COUNT
	};

	enum
	{
		SHADYFLAG_NONE			= 0x00000000,
		SHADYFLAG_CREATED		= 0x00000001,
		SHADYFLAG_CANSEEPLAYER  = 0x00000002
	};

	enum
	{
	   LOOKATSLIMSTATE_NONE = 0,
	   LOOKATSLIMSTATE_WINDUP,
	   LOOKATSLIMSTATE_THERE,
	   LOOKATSLIMSTATE_UNWIND,
	};

	static BOOL _InitCB_StateNone(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_WorldIdle(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_SeePlayer(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_IntroPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_ItemPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_BarterIdle(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_BarterIdleTalk(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_PurchaseConfirm(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_PurchaseAbort(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_UpsellPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_UpsellAbort(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_PostPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_LeaveNoPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _InitCB_LeaveWithPurchase(u32 uControlParam, void *pvShady, void *pvParam2);

	static BOOL _WorkCB_StateNone(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_WorldIdle(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_SeePlayer(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_IntroPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_ItemPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_BarterIdle(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_BarterIdleTalk(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_PurchaseConfirm(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_PurchaseAbort(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_UpsellPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_UpsellAbort(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_PostPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_LeaveNoPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _WorkCB_LeaveWithPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	
	static BOOL _CleanupCB_StateNone(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_WorldIdle(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_SeePlayer(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_IntroPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_ItemPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_BarterIdle(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_BarterIdleTalk(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_PurchaseConfirm(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_PurchaseAbort(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_UpsellPitch(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_UpsellAbort(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_PostPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_LeaveNoPurchase(u32 uControlParam, void *pvShady, void *pvParam2);
	static BOOL _CleanupCB_LeaveWithPurchase(u32 uControlParam, void *pvShady, void *pvParam2);


protected:

	enum
	{
		NO_AUTO_CHAIN = -1,
	};

	void SetCurItem( CBarterItemInst *pCurBII );
	void BeginSlimLookAt(void);
	void EndSlimLookAt(void);

	void StartTalkInst( CTalkInst* pInst, f32 fBlendInTimeSecs );
	void AnimWork(void);
	void DoDefaultStateWork(void);
	BOOL DoDefaultStateInit(void);
	void HandleLookAtSlim(void);

	
	void ChangeState(s32 nPendingState);
	BOOL IsStateChangePending(void);
	ShadyState_e GetCurState(void);

	CBarterResponse *GetStateResponse(void);
	void StopActiveResponses(void);

	u32 m_uFlags;
	f32 m_fNextIdle2Cntdn;
	f32 m_fNextSeePlayerCntdn;
	f32 m_fNextBarterIdleAudio;
	f32 m_fWorldIdleTimeOut;
	f32 m_fLookAtTimeOut;
	f32 m_fSpeechRadius;
	CMeshEntity *m_pMeshEntity;
	CShadyAnim	m_ShadyAnim;
	f32 m_fDistToPlayer;
	f32 m_fDelayBsProgressTimeOut;
	u32 m_uPlayerId;
	CFSM* m_pFsm;

	//data to drive basic flow for each state
	f32	m_afRespDelayTimeOut[SHADYSTATE_COUNT];
	CBarterResponse *m_apLastResp[SHADYSTATE_COUNT];
	u8 m_auResponseCountThisState[SHADYSTATE_COUNT];
	u8 m_auMinTimeBetweenResp[SHADYSTATE_COUNT];
	u8 m_auMaxTimeBetweenResp[SHADYSTATE_COUNT];
	s8 m_anAutoChainState[SHADYSTATE_COUNT];
	u8 m_auAutoChainAfterNResponses[SHADYSTATE_COUNT];
	u8 m_uLookAtSlimState;


	CTalkInst* m_pOldInst;
	CTalkInst* m_pCurInst;
	s32 m_nOldInLowHi;
	s32 m_nCurInLowHi;
	f32 m_fOOBlendTimeSecs;
	f32 m_fLookAtSlimSumPct;
	f32 m_fLookAtAnimTime;
	s32 m_anLowHiControlId[2];
	s32* m_apaBtControlId[2]; 
	s32* m_apaBtTapId[2];
	BOOL m_bStateIsTalking;
	BOOL m_bBSBusyOnPending;
	
	FCLASS_STACKMEM_NOALIGN(CShady);

} FCLASS_NOALIGN_SUFFIX;



typedef struct {
	cchar *pszKeyword;
	u8 nStateValue;

	u8 anUnused[3];
} Shady_BotTalkInfo_t;

extern const Shady_BotTalkInfo_t Shady_aBotTalkInfo[CShady::SHADYSTATE_COUNT];



#endif
