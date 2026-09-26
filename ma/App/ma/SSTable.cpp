//////////////////////////////////////////////////////////////////////////////////////
// SSTable.cpp - Slim and Shady Table class for Mettle Arms.
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

#include "SSTable.h"
#include "meshentity.h"
#include "floop.h"
#include "fanim.h"
#include "BarterTypes.h"


static const u32 _SEGMENT0_TEXID	 = 2;
static const u32 _SEGMENT1_TEXID	 = 3;
static const u32 _SEGMENT2_TEXID	 = 1;

#define	_PURCHASE_FLASH_RATE		15.0f
#define	_PURCHASE_NUM_FLASHES		7


static cchar *_pszMeshName = "GODItable00";
static cchar *_apszAnimNames[SSTABLEANIM_COUNT] = { "AODItable01", "AODItable02" };

static cchar *__apszSegNames[BARTER_TYPES_NUM_SLOTS] = { "R_Leaf", "C_Leaf", "L_Leaf" };

static cchar *_pszTableDownFx	= "TableDown";
static cchar *_pszTableUpFx		= "TableUp";
static cchar *_pszTableThudFx	= "TableSet";
static cchar *__pszTableFlipFx = "TableSpin";

static const f32 __fSegTiltAngle = FMATH_DEG2RAD(12.0f);
static const f32 __fSegTiltTime = 0.15f;

static const f32 __fTotalFlipAngle = FMATH_DEG2RAD(360.0f);
static const f32 __fTotalFlipTime = 0.5f;

static const f32 __fOOSegTiltTime = 1.0f / __fSegTiltTime;
static const f32 __fOOTotalFlipTime = 1.0f / __fTotalFlipTime;

#define _SEGMENT_FLASHRATE_SLOW		2.0f
#define _SEGMENT_FLASHRATE_FAST		4.0f

FSndFx_FxHandle_t CSSTable::m_hTableFlip	= FSNDFX_INVALID_FX_HANDLE;
FSndFx_FxHandle_t CSSTable::m_hUnfoldSound	= FSNDFX_INVALID_FX_HANDLE;
FSndFx_FxHandle_t CSSTable::m_hFoldSound	= FSNDFX_INVALID_FX_HANDLE;
FSndFx_FxHandle_t CSSTable::m_hThudSound	= FSNDFX_INVALID_FX_HANDLE;


CSSTable::CSSTable()
{
	m_pMesh = NULL;
	m_pManMtx = NULL;
	m_pRestMtx = NULL;
	m_pAnimCombiner = NULL;

	m_nCurSegIdx = -1;

	m_uFlags = 0;
}



CSSTable::~CSSTable()
{
	m_uFlags = 0;
	m_pMesh = NULL;
}



BOOL CSSTable::Create() {
	// Get a frame.
	FResFrame_t hResFrame = fres_GetFrame();

	m_pMesh = fnew CMeshEntity;
	if(m_pMesh == NULL)
	{
		DEVPRINTF("CSSTable::Create() : Out of memory.\n");
		return(FALSE);
	}
	if(!m_pMesh->Create(_pszMeshName, SSTABLEANIM_COUNT, _apszAnimNames, "SSTable", NULL))
	{
		DEVPRINTF("CSSTable::Create() : Error during creation of Slim & Shady Table.\n");
		return(FALSE);
	}
	m_pMesh->SetMeshInstFlags( FMESHINST_FLAG_ACCEPT_SHADOWS );

	// Check the CFMeshInst.
	CFMeshInst *pMI = m_pMesh->GetMeshInst();
	if(pMI == NULL)
	{
		return(FALSE);
	}
	
	// Get the dust bone stuff set up.
	const char *apszDustBoneNames[4] = { "foot1", "foot2", "foot3", "foot4" };
	u32 uDustBoneAryIdx;
	for(uDustBoneAryIdx = 0; uDustBoneAryIdx < 4; ++uDustBoneAryIdx)
	{
		m_anDustBoneIdx[uDustBoneAryIdx] = pMI->FindBone(apszDustBoneNames[uDustBoneAryIdx]);
	}

	// Set up what we need to make the table segments tilt downward.
	m_pManMtx = fnew CFAnimManMtx;
	if(m_pManMtx == NULL)
	{
		DEVPRINTF("CSSTable::Create() : Out of memory.\n");
		return(FALSE);
	}
	if( !m_pManMtx->Create(3, __apszSegNames) ) {
		DEVPRINTF("CSSTable::Create() : Could not Create() CFAnimManMtx.\n");
		goto _ExitWithError;
	}

	m_pRestMtx = fnew CFAnimMeshRest;
	if(m_pRestMtx == NULL)
	{
		DEVPRINTF("CSSTable::Create() : Out of memory.\n");
		return(FALSE);
	}
	if(!m_pRestMtx->Create(pMI->m_pMesh))
	{
		DEVPRINTF("CSSTable::Create() : Could not Create() CFAnimMeshRest.\n");
		goto _ExitWithError;
	}

	// Create an animation combiner object.
	m_pAnimCombiner = fnew CFAnimCombiner;
	if(m_pAnimCombiner == NULL)
	{
		DEVPRINTF("CSSTable::Create() : Could not create CFAnimCombiner.\n");
		goto _ExitWithError;
	}
	m_pAnimCombiner->CreateSummer(m_pMesh->GetMeshInst());
	m_pAnimCombiner->AttachToTap(1, m_pManMtx);
	m_pAnimCombiner->AttachToTap(0, m_pRestMtx);

	m_pMesh->DriveMeshWithAnim(TRUE);

	u32 uCurSegIdx;
	for(uCurSegIdx = 0; uCurSegIdx < BARTER_TYPES_NUM_SLOTS; ++uCurSegIdx)
	{
		m_anSegBoneIdx[uCurSegIdx] = pMI->FindBone(__apszSegNames[uCurSegIdx]);
		m_apBII[uCurSegIdx] = NULL;
	}
	
	// Load any needed sounds.
	m_hUnfoldSound = fsndfx_GetFxHandle(_pszTableDownFx); 
	if(m_hUnfoldSound == FSNDFX_INVALID_FX_HANDLE)
	{
		DEVPRINTF("CSSTable::Create() : Could not load sound effect '%s'.\n", _pszTableDownFx);
	}

	m_hFoldSound = fsndfx_GetFxHandle(_pszTableUpFx); 
	if(m_hFoldSound == FSNDFX_INVALID_FX_HANDLE)
	{
		DEVPRINTF("CSSTable::Create() : Could not load sound effect '%s'.\n", _pszTableUpFx);
	}

	m_hTableFlip = fsndfx_GetFxHandle(__pszTableFlipFx);
	if( m_hTableFlip == FSNDFX_INVALID_FX_HANDLE ) {
		DEVPRINTF("CSSTable::Create() : Could not load sound effect '%s'.\n", __pszTableFlipFx);
	}

	m_hThudSound = fsndfx_GetFxHandle(_pszTableThudFx);
	if( m_hThudSound == FSNDFX_INVALID_FX_HANDLE ) {
		DEVPRINTF("CSSTable::Create() : Could not load sound effect '%s'.\n", _pszTableThudFx);
	}

	m_ahSegmentTex[0] = m_pMesh->GetMeshInst()->GetTexLayerHandle( _SEGMENT1_TEXID );
	m_ahSegmentTex[1] = m_pMesh->GetMeshInst()->GetTexLayerHandle( _SEGMENT0_TEXID );
	m_ahSegmentTex[2] = m_pMesh->GetMeshInst()->GetTexLayerHandle( _SEGMENT2_TEXID );

	if( m_ahSegmentTex[0] == -1 ||
		m_ahSegmentTex[1] == -1 ||
		m_ahSegmentTex[2] == -1 ) {
	  DEVPRINTF("CSSTable::Create() : Could not retrieve segment tex handles.\n");
	  goto _ExitWithError;
	}

	
	m_pMesh->SetCollisionFlag(FALSE);

	_Clear();

	m_uFlags |= SSTABLEFLAG_CREATED;

	return(TRUE);

_ExitWithError:
	Destroy();
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}

void CSSTable::Destroy() {
	if(m_pMesh != NULL) {
		m_pMesh->Destroy();
	}
	fdelete(m_pMesh);
	m_pMesh = NULL;

	fdelete(m_pManMtx);
	m_pManMtx = NULL;

	fdelete(m_pRestMtx);
	m_pRestMtx = NULL;

	fdelete(m_pAnimCombiner);
	m_pAnimCombiner = NULL;
}

void CSSTable::Work() {

	_MakeDecisions();
	_DoBrainWork();

	_TableSegmentWork();
}

void CSSTable::AddToWorld() {
	u32 i;

	FASSERT(IsCreated());
	m_pMesh->AddToWorld();

	for( i=0; i<BARTER_TYPES_NUM_SLOTS; i++ ) {
		m_pMesh->GetMeshInst()->TexFlip_AnimateFlip( m_ahSegmentTex[i], FALSE );
		m_pMesh->GetMeshInst()->TexFlip_SetFlipPage( m_ahSegmentTex[i], 0 );
	}

	m_uPurchaseFlashCounter = -1;			// < 3 enables the quick flash

	m_uSegFlashPage			= 0;	
	m_eCmdPending = SSTABLECMD_NONE;

	m_pMesh->SetExternalCombiner(0, NULL);
	m_pMesh->DriveMeshWithAnim(TRUE);
	_GotoFolded();
	_Clear();
}

void CSSTable::RemoveFromWorld() {

	FASSERT(IsCreated());
	_Clear();
	m_pMesh->RemoveFromWorld();
}

void CSSTable::Relocate_WS(const CFMtx43A *pMtx_WS) {

	FASSERT(IsCreated());
	m_pMesh->Relocate_RotXlatFromUnitMtx_WS(pMtx_WS);
}

void CSSTable::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	m_pMesh->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}

void CSSTable::StartUnfold() {

	if(m_eCurState == SSTABLESTATE_FOLDED) {
		_StartUnfold();
	} else {
		m_eCmdPending = SSTABLECMD_STARTUNFOLD;
	}
}

void CSSTable::StartFold() {
	if( m_eCurState == SSTABLESTATE_UNFOLDED ) {
		_StartFold();
	} else {
		m_eCmdPending = SSTABLECMD_STARTFOLD;
	}
}

void CSSTable::SetCurrentItemIdx(s32 nTableSegIdx)
{
	FASSERT(nTableSegIdx >= -1);
	FASSERT(nTableSegIdx < BARTER_TYPES_NUM_SLOTS);

	// Set the last one to be untilted.
	if( m_nCurSegIdx != -1 ) {
		m_afDestUnitSegTilt[m_nCurSegIdx] = 0.0f;
		m_pMesh->GetMeshInst()->TexFlip_SetFlipPage( m_ahSegmentTex[m_nCurSegIdx], 0 );
	}

	//reset our timer for the flashing segment
	m_fSegFlashTimer = 1.0f;
	
	// Update our current item, and also set the new segment to be tilted.
	m_nCurSegIdx = nTableSegIdx;
	if( nTableSegIdx != -1 ) {
		m_afDestUnitSegTilt[nTableSegIdx] = 1.0f;
		m_pCurItem = m_apBII[nTableSegIdx];
	} else {
		m_pCurItem = NULL;
	}
}

void CSSTable::SetBarterItemInst( u32 uTableSegIdx, CBarterItemInst *pNewBII, BOOL bFlipDown ) {
	FASSERT(uTableSegIdx < BARTER_TYPES_NUM_SLOTS);

	// If this item is different from what was here before it, let's initiate a segment flip.
	if(m_apBII[uTableSegIdx] != pNewBII)
	{
		m_bPlayFlipSound = TRUE;
		if (bFlipDown)
		{
			m_afCurUnitTableFlip[uTableSegIdx] =  0.0f;
			m_afDestUnitTableFlip[uTableSegIdx] = 1.0f;
		}
		else
		{
			m_afCurUnitTableFlip[uTableSegIdx]  = 2.0f; 
			m_afDestUnitTableFlip[uTableSegIdx] = 1.0f;
		}
		//reset our timer for the flashing segment
		m_fSegFlashTimer = 1.0f - __fTotalFlipTime;
	}

	// remove old item from world
	m_apBII[uTableSegIdx]->RemoveFromWorld();
	m_apBII[uTableSegIdx] = pNewBII;

	if(m_nCurSegIdx == (s32)(uTableSegIdx))
	{
		m_pCurItem = pNewBII;
	}
}

void CSSTable::AttachToParent(CEntity *pParentEntity, cchar *pszAttachBoneName/* = NULL*/)
{
//	DEVPRINTF( "attaching table\n" );
	m_pMesh->Attach_UnitMtxToParent_PS(pParentEntity, pszAttachBoneName);
}

void CSSTable::DetachFromParent()
{
//	DEVPRINTF( "detaching table\n" );
	m_pMesh->DetachFromParent();
}

void CSSTable::ResetAllSegments() {
	u32 uCurSegIdx;
	for(uCurSegIdx = 0; uCurSegIdx < BARTER_TYPES_NUM_SLOTS; ++uCurSegIdx)
	{
		m_afDestUnitSegTilt[uCurSegIdx] = 0.0f;
		m_afDestUnitTableFlip[uCurSegIdx] = 0.0f;
	}

	m_bPlayFlipSound = TRUE;
}

void CSSTable::_Clear() {

	u32 uCurSegIdx;
	for(uCurSegIdx = 0; uCurSegIdx < BARTER_TYPES_NUM_SLOTS; ++uCurSegIdx)
	{
		m_afCurUnitSegTilt[uCurSegIdx] = 0.0f;
		m_afDestUnitSegTilt[uCurSegIdx] = 0.0f;

		m_afCurUnitTableFlip[uCurSegIdx] = 0.0f;
		m_afDestUnitTableFlip[uCurSegIdx] = 0.0f;

		if( m_apBII[uCurSegIdx] ) {
			m_apBII[uCurSegIdx]->RemoveFromWorld();
		}
		m_apBII[uCurSegIdx] = NULL;
		
	}

	m_pCurItem = NULL;
}

void CSSTable::_GotoFolded() {

	m_pMesh->UserAnim_Select(SSTABLEANIM_FOLD);
	m_pMesh->UserAnim_Pause(TRUE);

	CFAnimInst *pAnimInst;
	pAnimInst = m_pMesh->UserAnim_GetCurrentInst();
	if(pAnimInst != NULL)
	{
		pAnimInst->UpdateUnitTime(1.0f);
	}

	m_eCurState = SSTABLESTATE_FOLDED;
}

void CSSTable::_StartUnfold() {

	m_pMesh->UserAnim_Select(SSTABLEANIM_UNFOLD);
	m_pMesh->UserAnim_Pause(FALSE);

	CFAnimInst *pAnimInst;
	pAnimInst = m_pMesh->UserAnim_GetCurrentInst();
	if(pAnimInst != NULL)
	{
		pAnimInst->UpdateUnitTime(0.0f);
		m_fAnimCntdn = pAnimInst->GetTotalTime();
	}

	fsndfx_Play2D( m_hUnfoldSound );

	m_bSpawnedDust = FALSE;
	m_eCurState = SSTABLESTATE_UNFOLD;
}

void CSSTable::_GotoUnfolded() {

	m_pMesh->UserAnim_Select( SSTABLEANIM_UNFOLD );

	CFAnimInst *pAnimInst;
	pAnimInst = m_pMesh->UserAnim_GetCurrentInst();
	if( pAnimInst != NULL ) {
		pAnimInst->UpdateUnitTime(1.0f);
	}

	m_pMesh->UserAnim_Pause(TRUE);

	///////////////////////////////////////////////////////////////////////////
	// Due to being attached to Slim's hand, our orientation is going to be a
	//   bit wonky, so we'll need to fix up our orientation a bit.
	// Perhaps it might be better to have this inside of DetachFromParent().
	CFMtx43A::m_Temp = *m_pMesh->MtxToWorld();

	CFMtx43A::m_RotX.SetRotationX(FMATH_DEG2RAD(180.0f + 14.0f));
	CFMtx43A::m_Temp.Mul(CFMtx43A::m_RotX);

	CFMtx43A::m_RotY.SetRotationY(FMATH_DEG2RAD(180.0f));
	CFMtx43A::m_Temp.Mul(CFMtx43A::m_RotY);

	Relocate_WS(&CFMtx43A::m_Temp);
	//
	///////////////////////////////////////////////////////////////////////////

	m_pMesh->SetExternalCombiner( 0, m_pAnimCombiner );
	m_pMesh->UserAnim_Select( -1 );

	m_eCurState = SSTABLESTATE_UNFOLDED;
}

void CSSTable::_StartFold()
{
	FASSERT( m_nCurSegIdx >= 0 && m_nCurSegIdx < BARTER_TYPES_NUM_SLOTS );

	m_pMesh->SetExternalCombiner(0, NULL);
	m_pMesh->DriveMeshWithAnim(TRUE);

	m_pMesh->UserAnim_Select(SSTABLEANIM_FOLD);
	m_pMesh->UserAnim_Pause(FALSE);

	CFAnimInst *pAnimInst;
	pAnimInst = m_pMesh->UserAnim_GetCurrentInst();
	if(pAnimInst != NULL)
	{
		pAnimInst->UpdateUnitTime(0.0f);
		m_fAnimCntdn = pAnimInst->GetTotalTime();
	}

	
	_Clear();

	fsndfx_Play2D( m_hFoldSound );

	//turn off our light
	m_pMesh->GetMeshInst()->TexFlip_SetFlipPage( m_ahSegmentTex[m_nCurSegIdx], 0 );

	m_eCurState = SSTABLESTATE_FOLD;
}

void CSSTable::_MakeDecisions()
{
	switch(m_eCurState)
	{
		case SSTABLESTATE_FOLDED:
		{
			_FoldedBrainDecisions();
			break;
		}
		case SSTABLESTATE_UNFOLD:
		{
			_UnfoldBrainDecisions();
			break;
		}
		case SSTABLESTATE_UNFOLDED:
		{
			_UnfoldedBrainDecisions();
			break;
		}
		case SSTABLESTATE_FOLD:
		{
			_FoldBrainDecisions();
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

void CSSTable::_FoldedBrainDecisions()
{
	switch(m_eCmdPending)
	{
		case SSTABLECMD_NONE:
		{
			break;
		}
		case SSTABLECMD_STARTUNFOLD:
		{
			_StartUnfold();
			m_eCmdPending = SSTABLECMD_NONE;
			break;
		}
		default:
		{
			FASSERT_NOW;
			break;
		}
	}
}

void CSSTable::_UnfoldBrainDecisions()
{
	switch(m_eCmdPending)
	{
		case SSTABLECMD_NONE:
		{
			m_fAnimCntdn -= FLoop_fPreviousLoopSecs;
			if(m_fAnimCntdn <= 0.0f)
			{
				_GotoUnfolded();
			}

			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

void CSSTable::_UnfoldedBrainDecisions()
{
	switch(m_eCmdPending)
	{
		case SSTABLECMD_NONE:
		{
			break;
		}
		case SSTABLECMD_STARTFOLD:
		{
			_StartFold();
			m_eCmdPending = SSTABLECMD_NONE;
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

void CSSTable::_FoldBrainDecisions()
{
	switch(m_eCmdPending)
	{
		case SSTABLECMD_NONE:
		{
			m_fAnimCntdn -= FLoop_fPreviousLoopSecs;
			if(m_fAnimCntdn <= 0.0f)
			{
				_GotoFolded();
				_Clear();
			}
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

void CSSTable::_DoBrainWork()
{
	switch(m_eCurState)
	{
		case SSTABLESTATE_FOLDED:
		{
			_FoldedBrainWork();
			break;
		}
		case SSTABLESTATE_UNFOLD:
		{
			_UnfoldBrainWork();
			break;
		}
		case SSTABLESTATE_UNFOLDED:
		{
			_UnfoldedBrainWork();
			break;
		}
		case SSTABLESTATE_FOLD:
		{
			_FoldBrainWork();
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

void CSSTable::_FoldedBrainWork() {
}

void CSSTable::_UnfoldBrainWork() {
	// Check for spawning dust.
	if( (m_pMesh->UserAnim_GetCurrentInst()->GetUnitTime() >= 0.76f) && (!m_bSpawnedDust) ) {
		u32 uDustBoneAryIdx;
		CFMtx43A *pMtx;
		CFMeshInst *pMI = m_pMesh->GetMeshInst();

		fsndfx_Play2D( m_hThudSound );


		for( uDustBoneAryIdx = 0; uDustBoneAryIdx < 4; ++uDustBoneAryIdx ) {
			pMtx = pMI->GetBoneMtxPalette()[m_anDustBoneIdx[uDustBoneAryIdx]];
			FASSERT(pMtx != NULL);
	
//			fpart_SpawnParticleEmitter( FPART_TYPE_DUST_PUFF, 0.5f, pMtx->m_vPos, &CFVec3A::m_UnitAxisY, FALSE );
		}

		m_bSpawnedDust = TRUE;
	}	
}

void CSSTable::_UnfoldedBrainWork()
{
}

void CSSTable::_FoldBrainWork()
{
}

void CSSTable::_TableSegmentWork()
{
	if(m_eCurState != SSTABLESTATE_UNFOLDED)
	{
		return;
	}

	if(m_bPlayFlipSound)
	{
		fsndfx_Play2D(m_hTableFlip);
		m_bPlayFlipSound = FALSE;
	}

	CFVec3A vecTemp;

	// Check the dest pos of each segment, if it's not where it wants to be, start moving it there.
	s32 uCurSegIdx;

	m_fSegFlashTimer += FLoop_fPreviousLoopSecs * m_fSegFlashRate;

	for( uCurSegIdx = 0; uCurSegIdx < BARTER_TYPES_NUM_SLOTS; ++uCurSegIdx ) {

		if( m_afDestUnitSegTilt[uCurSegIdx] < m_afCurUnitSegTilt[uCurSegIdx] ) {
			m_afCurUnitSegTilt[uCurSegIdx] -= FLoop_fPreviousLoopSecs * __fOOSegTiltTime;
			FMATH_CLAMP_MIN0( m_afCurUnitSegTilt[uCurSegIdx] );
		} else if( m_afDestUnitSegTilt[uCurSegIdx] > m_afCurUnitSegTilt[uCurSegIdx] ) {
			m_afCurUnitSegTilt[uCurSegIdx] += FLoop_fPreviousLoopSecs * __fOOSegTiltTime;
			FMATH_CLAMP_MAX1(m_afCurUnitSegTilt[uCurSegIdx]);
		}

		if( m_afDestUnitTableFlip[uCurSegIdx] < m_afCurUnitTableFlip[uCurSegIdx] ) {
			m_afCurUnitTableFlip[uCurSegIdx] -= FLoop_fPreviousLoopSecs * __fOOTotalFlipTime;
			FMATH_CLAMP_MIN0(m_afCurUnitTableFlip[uCurSegIdx]);
		} else if( m_afDestUnitTableFlip[uCurSegIdx] > m_afCurUnitTableFlip[uCurSegIdx] ) {
			m_afCurUnitTableFlip[uCurSegIdx] += FLoop_fPreviousLoopSecs * __fOOTotalFlipTime;
			FMATH_CLAMP_MAX1(m_afCurUnitTableFlip[uCurSegIdx]);
		}

		f32 fThetaX = fmath_UnitLinearToSCurve( m_afCurUnitSegTilt[uCurSegIdx] ) * __fSegTiltAngle;
		BOOL bDrawWeapon = FALSE;
		if ( ( m_afCurUnitTableFlip[uCurSegIdx] <= m_afDestUnitTableFlip[uCurSegIdx]) )
		{
			fThetaX += fmath_UnitLinearToSCurve(m_afCurUnitTableFlip[uCurSegIdx]) * __fTotalFlipAngle;
			if( fThetaX >= __fTotalFlipAngle - FMATH_DEG2RAD(40.0f))
				bDrawWeapon =TRUE;
		}
		else
		{
			fThetaX -= fmath_UnitLinearToSCurve(m_afCurUnitTableFlip[uCurSegIdx]) * __fTotalFlipAngle;
			if( fThetaX <= -__fTotalFlipAngle + FMATH_DEG2RAD(40.0f))
				bDrawWeapon =TRUE;
		}

		CFMtx43A::m_RotX.SetRotationX( fThetaX );

		if( m_apBII[uCurSegIdx] != NULL ) {
			if( bDrawWeapon ) {
				if( uCurSegIdx == m_nCurSegIdx ) {
					_FlashSegmentLight( uCurSegIdx  );
				}
				// Let's calculate a place to put the items.
				CFMtx43A::m_Temp = *m_pMesh->MtxToWorld();
				CFMtx43A::m_Temp.m_vPos = m_pMesh->GetMeshInst()->GetBoneMtxPalette()[m_anSegBoneIdx[uCurSegIdx]]->m_vPos;
				CFMtx43A::m_Temp.m_vPos.y += 0.4f;
				m_apBII[uCurSegIdx]->AddToWorld();
				m_apBII[uCurSegIdx]->SetMtx(CFMtx43A::m_Temp);

			} else {
				m_apBII[uCurSegIdx]->RemoveFromWorld();
			}
		}

		m_pManMtx->UpdateFrame(uCurSegIdx, CFMtx43A::m_RotX);
	}	
}



void CSSTable::ResetTable( const BarterTypes_TableData_t *pTableData ) {
//	DEVPRINTF( "Resetting table\n" );

	_Clear();
	
	m_apBII[0]		= pTableData->apSlots[0];
	m_apBII[1]		= pTableData->apSlots[1];
	m_apBII[2]		= pTableData->apSlots[2];
	
	SetCurrentItemIdx( pTableData->nCurrentSlot );

	for( u32 i=0; i<BARTER_TYPES_NUM_SLOTS; i++ ) {
		m_bPlayFlipSound			= TRUE;
		m_afCurUnitTableFlip[i]		= -0.7f * fmath_RandomFloat();
		m_afDestUnitTableFlip[i]	= 1.0f;
		m_pMesh->GetMeshInst()->TexFlip_SetFlipPage( m_ahSegmentTex[i], 0 );
	}

	m_fSegFlashTimer	= 0.0f;
	m_fSegFlashRate		= pTableData->bTableIsAnUpsale ? _SEGMENT_FLASHRATE_FAST : _SEGMENT_FLASHRATE_SLOW;
}


void CSSTable::_FlashSegmentLight( u32 uCurSegIdx ) {
	FASSERT( uCurSegIdx < BARTER_TYPES_NUM_SLOTS );
	u32 uSegFlashPage;

	uSegFlashPage = (((u32)m_fSegFlashTimer) & 0x1);

	if( m_uPurchaseFlashCounter < _PURCHASE_NUM_FLASHES ) {

		if( m_uSegFlashPage != uSegFlashPage )  {
			m_uSegFlashPage = !m_uSegFlashPage;
	
			if( m_uSegFlashPage ) {
				m_uPurchaseFlashCounter++;
			}
		}
		if( m_uPurchaseFlashCounter == (_PURCHASE_NUM_FLASHES - 1) ) {
			m_fSegFlashRate = 0.0f;
			m_uSegFlashPage = 0;
		}
	}
	
	m_pMesh->GetMeshInst()->TexFlip_SetFlipPage( m_ahSegmentTex[uCurSegIdx], uSegFlashPage );
}


float CSSTable::LastSpin( void ) {
	
	_Clear();

	//turn off our light
	m_pMesh->GetMeshInst()->TexFlip_SetFlipPage( m_ahSegmentTex[m_nCurSegIdx], 0 );
	
	for( u32 i=0; i<BARTER_TYPES_NUM_SLOTS; i++ ) {
		m_bPlayFlipSound			= TRUE;
		m_afCurUnitTableFlip[i]		= -0.7f * fmath_RandomFloat();
		m_afDestUnitTableFlip[i]	= 1.0f;
	}


	return 0.5f;
}


void CSSTable::FlashTable( void ) { 
	m_fSegFlashTimer		= 0.0f;
	m_fSegFlashRate			= _PURCHASE_FLASH_RATE;
	m_uPurchaseFlashCounter = 0; 
}
