//////////////////////////////////////////////////////////////////////////////////////
// SSTable.h - Slim and Shady Table class for Mettle Arms.
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
// 10/24/02 Elliott		New barter system
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SSTABLE_H_
#define _SSTABLE_H_ 1

#include "fang.h"
#include "fsndfx.h"
#include "BarterTypes.h"

class CMeshEntity;
class CEntity;
class CFMtx43A;

class CFAnimManMtx;
class CFAnimMeshRest;
class CFAnimCombiner;

class CBarterItemInst;

enum SSTableAnim_e {
	SSTABLEANIM_UNFOLD = 0,
	SSTABLEANIM_FOLD,

	SSTABLEANIM_COUNT
};


FCLASS_NOALIGN_PREFIX class CSSTable
{
public:
	CSSTable();
	~CSSTable();

	BOOL Create();
	void Destroy();

	void Work();

	void AddToWorld();
	void RemoveFromWorld();
	void Relocate_WS( const CFMtx43A *pMtx_WS );
	void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	void GotoFolded()	{ _GotoFolded(); }
	void StartUnfold();
	void GotoUnfolded() { _GotoUnfolded(); }

	float LastSpin( void );
	void StartFold();
	void SetCurrentItemIdx( s32 nTableSegIdx );
	void SetBarterItemInst( u32 uTableSegIdx, CBarterItemInst *pNewBII, BOOL bFlipDown );

	void AttachToParent( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL );
	void DetachFromParent();
	void ResetAllSegments();

	void ResetTable( const BarterTypes_TableData_t *pTableData );
	void FlashTable( void );
	void OverRideSegmentFlashState( f32 fNewTime, f32 fNewRate ) { m_fSegFlashTimer = fNewTime; m_fSegFlashRate = fNewRate; };

private:
	
	enum SSTableFlags_e {
		SSTABLEFLAG_CREATED		=	1 << 0
	};
	u32 m_uFlags;

	BOOL IsCreated() { return((m_uFlags & SSTABLEFLAG_CREATED) != 0); }
	
	s32 m_anDustBoneIdx[4];
	
	enum SSTableCommand_e {
		SSTABLECMD_NONE = 0,
		SSTABLECMD_STARTFOLDED,
		SSTABLECMD_STARTUNFOLD,
		SSTABLECMD_STARTUNFOLDED,
		SSTABLECMD_STARTFOLD,

		SSTABLECOMMAND_COUNT
	};
	SSTableCommand_e m_eCmdPending;			// If a command was issued that could not be executed right then,
											//   we keep track of it here.

	// Right now, these all correspond to the animations listed above.
	enum SSTableState_e {
		SSTABLESTATE_FOLDED = 0,
		SSTABLESTATE_UNFOLD,
		SSTABLESTATE_UNFOLDED,
		SSTABLESTATE_FOLD,

		SSTABLESTATE_UNDEFINED,				// Special state that should only be temporary.

		SSTABLESTATE_COUNT
	};
	SSTableState_e m_eCurState;
	f32 m_fAnimCntdn;
	
	void _Clear();
	
	void _GotoFolded();
	void _StartUnfold();
	void _GotoUnfolded();
	void _StartFold();
	void _StartTilt();
	void _EnableSegment( u32 nSegment, BOOL bEnable );
	
	// Brain decision functions.
	void _MakeDecisions();

	void _FoldedBrainDecisions();
	void _UnfoldBrainDecisions();
	void _UnfoldedBrainDecisions();
	void _FoldBrainDecisions();

	// Brain work functions.
	void _DoBrainWork();

	void _FoldedBrainWork();
	void _UnfoldBrainWork();
	void _UnfoldedBrainWork();
	void _FoldBrainWork();

	void _TableSegmentWork();
	void _FlashSegmentLight( u32 uCurSegIdx );




	
	BOOL m_bSpawnedDust;
	
	f32 m_afCurUnitSegTilt[BARTER_TYPES_NUM_SLOTS];
	f32 m_afDestUnitSegTilt[BARTER_TYPES_NUM_SLOTS];
	f32 m_afCurUnitTableFlip[BARTER_TYPES_NUM_SLOTS];
	f32 m_afDestUnitTableFlip[BARTER_TYPES_NUM_SLOTS];

	CBarterItemInst *m_apBII[BARTER_TYPES_NUM_SLOTS];
	CBarterItemInst *m_pCurItem;

	s32 m_anSegBoneIdx[BARTER_TYPES_NUM_SLOTS];

	s32 m_nCurSegIdx;
	u32	m_uPurchaseFlashCounter;					//used to quick flash the table 3 times
	u32 m_uSegFlashPage;			//used to check when we're switching from on to off
	f32 m_fSegFlashTimer;			//used for flashing the table segs
	f32 m_fSegFlashRate;			//can change the flash rate if it's an upsale		

	CFAnimManMtx *m_pManMtx;
	CFAnimMeshRest *m_pRestMtx;
	CFAnimCombiner *m_pAnimCombiner;
	
	static FSndFx_FxHandle_t m_hTableFlip;
	static FSndFx_FxHandle_t m_hUnfoldSound;
	static FSndFx_FxHandle_t m_hFoldSound;
	static FSndFx_FxHandle_t m_hThudSound;

	BOOL m_bPlayFlipSound;
	
	CMeshEntity *m_pMesh;

	FMeshTexLayerHandle_t	m_ahSegmentTex[BARTER_TYPES_NUM_SLOTS];

	FCLASS_STACKMEM_NOALIGN( CSSTable );
} FCLASS_NOALIGN_SUFFIX;

#endif
