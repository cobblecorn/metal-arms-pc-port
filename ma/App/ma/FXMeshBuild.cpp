//////////////////////////////////////////////////////////////////////////////////////
// FXMeshBuild.cpp - General bot part movement system.
//
// Author: Michael Starich
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/06/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "FXMeshBuild.h"
#include "fres.h"
#include "floop.h"

////////////////////////////
// CFXMeshBuildPart methods
////////////////////////////

CFAlignedPool *CFXMeshBuildPart::m_pAlignedMemPool;

BOOL CFXMeshBuildPart::InitSystem() {
	
	m_pAlignedMemPool = &FAlignedPool_VecPool;
	return TRUE;
}

void CFXMeshBuildPart::UninitSystem() {
	
	m_pAlignedMemPool = NULL;
}

CFXMeshBuildPart::CFXMeshBuildPart() {
	m_nState = STATE_UNUSED;	
	m_pStartPos = NULL;
	m_pDestPos = NULL;
	m_pRotStart = NULL;
	m_pRotDest = NULL;
	m_pAxisToRotateAbout = NULL;
}

CFXMeshBuildPart::~CFXMeshBuildPart() {
	m_nState = STATE_UNUSED;

	ReturnPooledMemory();
}

void CFXMeshBuildPart::ReturnPooledMemory() {
	FASSERT( m_pAlignedMemPool );

	if( m_pDestPos ) {
		m_pAlignedMemPool->ReturnElement( m_pDestPos );
		m_pDestPos = NULL;
	}
	if( m_pStartPos ) {
		m_pAlignedMemPool->ReturnElement( m_pStartPos );
		m_pStartPos = NULL;
	}
	if( m_pRotStart ) {
		m_pAlignedMemPool->ReturnElement( m_pRotStart );
		m_pRotStart = NULL;
	}
	if( m_pRotDest ) {
		m_pAlignedMemPool->ReturnElement( m_pRotDest );
		m_pRotDest = NULL;
	}
	if( m_pAxisToRotateAbout ) {
		m_pAlignedMemPool->ReturnElement( m_pAxisToRotateAbout );
        m_pAxisToRotateAbout = NULL;
	}
}

// will init the part and update the mtx palette to move the part to rFrom
// ( doesn't alter the rotation, that will be needed on ::Start() )
BOOL CFXMeshBuildPart::InitPart( const CFVec3A &rFrom, CFMtx43A **ppMtxPalette, u32 nBoneIndex, f32 fUnitBoneSize ) {
	FASSERT( m_pAlignedMemPool );

	// reset the current state
	ReturnPooledMemory();
	m_nState = STATE_UNUSED;
	m_nBoneIndex = nBoneIndex;
	m_fAnimTime = 0.0f;

	// grab a vec from the pool so that the final destination can be stored
	m_pDestPos = (CFVec3A *)m_pAlignedMemPool->GetElement();
	if( !m_pDestPos ) {
		return FALSE;
	}
	m_pDestPos->Set( ppMtxPalette[m_nBoneIndex]->m_vPos );

	// update the mtx palette's pos without changine the rot
	ppMtxPalette[m_nBoneIndex]->m_vPos.Set( rFrom );

	// compute how much the part will rotate when started
	m_fRadiansToRotate = fmath_RandomBipolarUnitFloatWithGap( 0.5f );
	m_fRadiansToRotate *= (FMATH_2PI * (7.0f - (fUnitBoneSize * 4.0f)) );

	// compute how far the part will travel
	m_fDistSqToTravel = rFrom.DistSq( *m_pDestPos );

	m_nState = STATE_READY;

	return TRUE;
}

// will start the part animating toward it's final destination
BOOL CFXMeshBuildPart::Start( CFMtx43A **ppMtxPalette ) {
	FASSERT( m_pAlignedMemPool );
	
	if( m_nState != STATE_READY ) {
		FASSERT_NOW;
		return FALSE;
	}

	CFQuatA Quat;

	// grab 1 vec and 2 quats from the mem pool
	m_pStartPos = (CFVec3A *)m_pAlignedMemPool->GetElement();
	m_pRotStart = (CFQuatA *)m_pAlignedMemPool->GetElement();
	m_pRotDest = (CFQuatA *)m_pAlignedMemPool->GetElement();
	m_pAxisToRotateAbout = (CFVec3A *)m_pAlignedMemPool->GetElement();
	if( !m_pStartPos || !m_pRotStart || !m_pRotDest || !m_pAxisToRotateAbout ) {
		goto _EXIT_WITH_ERROR;
	}

	// copy the starting pos
	m_pStartPos->Set( ppMtxPalette[m_nBoneIndex]->m_vPos );

	// copy the dest rot
	m_pRotDest->BuildQuat( *ppMtxPalette[m_nBoneIndex] );

	// set the src rot
	m_pAxisToRotateAbout->Set( fmath_RandomBipolarUnitFloat(), fmath_RandomFloat(), fmath_RandomBipolarUnitFloat() * 1.67f );
	m_pAxisToRotateAbout->Unitize();
	
	Quat.BuildQuat( *m_pAxisToRotateAbout, m_fRadiansToRotate );
	m_pRotStart->Mul( Quat, *m_pRotDest );

	// update the matrix palette with the new rot
	m_pRotStart->BuildMtx( *ppMtxPalette[m_nBoneIndex] );
	ppMtxPalette[m_nBoneIndex]->m_vPos.Set( *m_pStartPos );

	m_fAnimTime = 0.0f;
	m_nState = STATE_MOVE;

	return TRUE;

_EXIT_WITH_ERROR:
	if( m_pStartPos ) {
		m_pAlignedMemPool->ReturnElement( m_pStartPos );
		m_pStartPos = NULL;
	}
	if( m_pRotStart ) {
		m_pAlignedMemPool->ReturnElement( m_pRotStart );
		m_pRotStart = NULL;
	}
	if( m_pRotDest ) {
		m_pAlignedMemPool->ReturnElement( m_pRotDest );
		m_pRotDest = NULL;
	}
	if( m_pAxisToRotateAbout ) {
		m_pAlignedMemPool->ReturnElement( m_pAxisToRotateAbout );
		m_pAxisToRotateAbout = NULL;
	}

	return FALSE;
}

// returns TRUE when the animation is finished
BOOL CFXMeshBuildPart::Work( CFMtx43A **ppMtxPalette, f32 fOOSecsToAnimate, FMeshBone_t *pBoneArray ) {
	FASSERT( m_pAlignedMemPool );
	FASSERT( m_nState == STATE_MOVE );

	m_fAnimTime += FLoop_fPreviousLoopSecs;
	f32 fUnitTime = m_fAnimTime * fOOSecsToAnimate;
	if( fUnitTime >= 0.999f ) {
		// last frame...finish up
		m_nState = STATE_DONE;

		// force the last frame
		m_pRotDest->BuildMtx( *ppMtxPalette[m_nBoneIndex] );
		ppMtxPalette[m_nBoneIndex]->m_vPos.Set( *m_pDestPos );

		// return memory back to the pool
		ReturnPooledMemory();
		return TRUE;
	}
	f32 fPercent = fmath_UnitLinearToSCurve( fUnitTime );
	f32 fOneMinusUnitTime = 1.0f - fUnitTime;

	// generate a pos and rot
	CFVec3A Pos;
	Pos.Lerp( fPercent, *m_pStartPos, *m_pDestPos );

	CFQuatA Quat, CurrentQuat;
#if 0
	Quat.BuildQuat( *m_pAxisToRotateAbout, m_fRadiansToRotate * fOneMinusUnitTime );
	CurrentQuat.Mul( Quat, *m_pRotDest );

	// update the matrix palette
	CurrentQuat.BuildMtx( *ppMtxPalette[m_nBoneIndex] );
	ppMtxPalette[m_nBoneIndex]->m_vPos.Set( Pos );
#else
	Quat.BuildQuat( *m_pAxisToRotateAbout, m_fRadiansToRotate * fOneMinusUnitTime );
	CurrentQuat.Mul( Quat, *m_pRotDest );

	// rotate about the center of the BS
	CFVec3A WSCenterOffset, BSCenter;
	BSCenter.Set( pBoneArray[m_nBoneIndex].SegmentedBoundSphere_BS.m_Pos );
	BSCenter.Negate();
	BSCenter.Mul( 1.0f-(fPercent*fPercent*fPercent) );
	CurrentQuat.MulPoint( WSCenterOffset, BSCenter );
	
	// update the matrix palette
	CurrentQuat.BuildMtx( *ppMtxPalette[m_nBoneIndex] );
	ppMtxPalette[m_nBoneIndex]->m_vPos.Add( Pos, WSCenterOffset );
#endif
	
	// not finished yet
	return FALSE;
}

// returns TRUE if the matrix palette was updated
BOOL CFXMeshBuildPart::ForceFinish( CFMtx43A **ppMtxPalette ) {
	FASSERT( m_pAlignedMemPool );
	
	switch( m_nState ) {

	default:
	case STATE_UNUSED:
	case STATE_DONE:
		// nothing to do
		return FALSE;
		break;

	case STATE_READY:
		// only need to update the matrix palette position
		ppMtxPalette[m_nBoneIndex]->m_vPos.Set( *m_pDestPos );
		// clean up after ourself
		ReturnPooledMemory();
		m_nState = STATE_DONE;
		break;

	case STATE_MOVE:
		// we need to update the entire matrix
		m_pRotDest->BuildMtx( *ppMtxPalette[m_nBoneIndex] );
		ppMtxPalette[m_nBoneIndex]->m_vPos.Set( *m_pDestPos );
		// clean up after ourself
		ReturnPooledMemory();
		m_nState = STATE_DONE;
		break;		
	}

	return TRUE;
}


///////////////////////////////
// CFXMeshBuildPartMgr methods
///////////////////////////////

u32 CFXMeshBuildPartMgr::m_nSizeOfMgrPool = 0;
CFXMeshBuildPartMgr *CFXMeshBuildPartMgr::m_paMgrPool = NULL;

u32 CFXMeshBuildPartMgr::m_nRequestedMaxBones;
u32 CFXMeshBuildPartMgr::m_nRequestedMaxSimultaneousMangers;

CFXMeshBuildPartMgr::CFXMeshBuildPartMgr() {
	m_nMaxParts = 0;
    m_paParts = NULL;
	m_papSortedParts = NULL;
	m_nState = STATE_NEED_INIT;

    m_nNumParts = 0;	
    m_nNextBoneToStart = 0;
	m_nOldestWorkingBone = 0;

	m_fOOPartSecsToComplete = 0.0f;
	m_fSecsTillNextStart = 0.0f;
	m_fSecsBetweenStarts = 0.0f;
	m_pWorldMesh = NULL;

	m_fTimeSoFar = 0.0f;
	m_fOOTotalSecsToComplete = 0.0f;
}

CFXMeshBuildPartMgr::~CFXMeshBuildPartMgr() {

	ReleaseMemory();
}

BOOL CFXMeshBuildPartMgr::InitSystem() {
	m_nSizeOfMgrPool = 0;
	m_paMgrPool = NULL;

	m_nRequestedMaxBones = 0;
	m_nRequestedMaxSimultaneousMangers = 0;

	return TRUE;
}

void CFXMeshBuildPartMgr::UninitSystem() {
	UninitLevel();
}

void CFXMeshBuildPartMgr::SubmitInitNeeds( u8 nMaxBones, u32 nMaxSimultaneousMangers ) {
	
	FASSERT( nMaxBones );
	FASSERT( nMaxSimultaneousMangers );
    m_nRequestedMaxBones = FMATH_MAX( nMaxBones, m_nRequestedMaxBones );
	m_nRequestedMaxSimultaneousMangers = FMATH_MAX( nMaxSimultaneousMangers, m_nRequestedMaxSimultaneousMangers );
}

BOOL CFXMeshBuildPartMgr::InitLevel() {
	FResFrame_t hFrame;
	u32 i;

    FASSERT( m_paMgrPool == NULL );
	FASSERT( m_nSizeOfMgrPool == 0 );

	if( m_nRequestedMaxBones == 0 || m_nRequestedMaxSimultaneousMangers == 0 ) {
		// none requested on this level
		return FALSE;
	}

	hFrame = fres_GetFrame();

	// allocate m_nRequestedMaxSimultaneousMangers each with m_nRequestedMaxBones support
	m_paMgrPool = fnew CFXMeshBuildPartMgr[m_nRequestedMaxSimultaneousMangers];
	if( !m_paMgrPool ) {
		goto _EXIT_WITH_ERROR;
	}

	// run through each manager, allocating the correct memory
	for( i=0; i < m_nRequestedMaxSimultaneousMangers; i++ ) {
		if( !m_paMgrPool[i].AllocateMemory( m_nRequestedMaxBones ) ) {
			goto _EXIT_WITH_ERROR;
			break;
		}
	}

	m_nSizeOfMgrPool = m_nRequestedMaxSimultaneousMangers;

	return TRUE;

_EXIT_WITH_ERROR:
	UninitLevel();
	fres_ReleaseFrame( hFrame );

	return FALSE;
}

void CFXMeshBuildPartMgr::UninitLevel() {
	
	if( m_paMgrPool ) {
		fdelete_array( m_paMgrPool );
		m_paMgrPool = NULL;
	}
	m_nSizeOfMgrPool = 0;

	m_nRequestedMaxBones = 0;
	m_nRequestedMaxSimultaneousMangers = 0;
}

CFXMeshBuildPartMgr *CFXMeshBuildPartMgr::InitMgr( const CFVec3A &rFrom, CFWorldMesh *pWorldMesh,
												  f32 fSecsToComplete, f32 fUnitOverlap/*=0.0f*/,
												  u32 nNumAdditionalBones/*=0*/, u32 *panAdditionalBones/*=NULL*/ ) {
	CFXMeshBuildPartMgr *pMgr;
	u32 i;

	// find a free mgr
	pMgr = NULL;
	for( i=0; i < m_nSizeOfMgrPool; i++ ) {
		if( m_paMgrPool[i].IsMgrAvailable() ) {
			pMgr = &m_paMgrPool[i];
			break;
		}
	}
	if( !pMgr ) {
		return NULL;
	}

	if( !pMgr->Init( rFrom, pWorldMesh, fSecsToComplete, fUnitOverlap, nNumAdditionalBones, panAdditionalBones ) ) {
		return NULL;
	}

	return pMgr;
}

CFXMeshBuildPartMgr *CFXMeshBuildPartMgr::InitMgr( const CFVec3A &rFrom, CEntity *pEntity, CFWorldMesh *pWorldMesh,
												  f32 fSecsToComplete, f32 fUnitOverlap/*=0.0f*/ ) {
	u32 nNumAdditionalBones=0, anAdditionalBones[16], i;
	CEntity *pChild;
	cchar *pszBoneName;
	s32 nBoneIndex;

	// create a list of extra bones that need to be added
	pChild = pEntity->GetFirstChild();
	while( pChild ) {
		pszBoneName = pChild->GetAttachBoneName();
		if( pszBoneName ) {
			nBoneIndex = pWorldMesh->FindBone( pszBoneName );
			if( nBoneIndex >= pWorldMesh->m_pMesh->nUsedBoneCount ) {
				// this is a bone that usually won't get included, add it (if it hasn't already been added)
				for( i=0; i < nNumAdditionalBones; i++ ) {
					if( anAdditionalBones[i] == nBoneIndex ) {
						break;
					}
				}
				if( i == nNumAdditionalBones ) {
					anAdditionalBones[nNumAdditionalBones] = nBoneIndex;
					nNumAdditionalBones++;
					if( nNumAdditionalBones >= 16 ) {
						break;
					}
				}
			}
		}
		
		// grab the next child in the chain
		pChild = pEntity->GetNextChild( pChild );
	}

	return InitMgr( rFrom, pWorldMesh, fSecsToComplete, fUnitOverlap, nNumAdditionalBones, anAdditionalBones );
}

BOOL CFXMeshBuildPartMgr::AllocateMemory( u8 nMaxBonesToSupport ) {
	FASSERT( m_nState == STATE_NEED_INIT );
	
	m_paParts = fnew CFXMeshBuildPart[nMaxBonesToSupport];
	if( !m_paParts ) {
		return FALSE;
	}
    m_papSortedParts = (CFXMeshBuildPart **)fres_AlignedAllocAndZero( sizeof( CFXMeshBuildPart * ) * nMaxBonesToSupport, 4 );
	if( !m_papSortedParts ) {
		fdelete_array( m_paParts );
		m_paParts = NULL;
		return FALSE;
	}

	m_nMaxParts = nMaxBonesToSupport;
	m_nState = STATE_AVAILABLE;

	return TRUE;
}

void CFXMeshBuildPartMgr::ReleaseMemory() {
	m_nState = STATE_NEED_INIT;
	
	if( m_paParts ) {
		fdelete_array( m_paParts );
		m_paParts = NULL;
	}
	m_papSortedParts = NULL;
	
	m_nMaxParts = 0;
}

// returns TRUE when the work is finished and the manger has been returned to the pool
BOOL CFXMeshBuildPartMgr::Work( f32 &rfUnitCompletePercent ) {
	FASSERT( m_nState == STATE_IN_USE );

	u32 i;
	CFMtx43A **ppMtxPalette;

	// grab the maxtrix palette and bone array
	ppMtxPalette = m_pWorldMesh->GetBoneMtxPalette();
	FMeshBone_t *pBoneArray = m_pWorldMesh->m_pMesh->pBoneArray;

	// call work on all existing parts
	for( i=m_nOldestWorkingBone; i < m_nNextBoneToStart; i++ ) {
		if( m_papSortedParts[i]->Work( ppMtxPalette, m_fOOPartSecsToComplete, pBoneArray ) ) {
			m_nOldestWorkingBone++;
		}
	}

	// start new parts
	if( m_nNextBoneToStart < m_nNumParts ) {
		m_fSecsTillNextStart -= FLoop_fPreviousLoopSecs;
		while( m_fSecsTillNextStart <= 0.0f && (m_nNextBoneToStart < m_nNumParts) ) {
			if( !m_papSortedParts[m_nNextBoneToStart]->Start( ppMtxPalette ) ) {
				// try again next frame
				break;
			}
			m_nNextBoneToStart++;

			m_fSecsTillNextStart += m_fSecsBetweenStarts;
		}
	}

	UpdateBoundingInfo();

	if( m_nOldestWorkingBone >= m_nNumParts ) {
		// reset the vars that need to be before returning to the free pool
        m_nNumParts = 0;
		m_pWorldMesh = NULL;
        m_nState = STATE_AVAILABLE;		
		rfUnitCompletePercent = 1.0f;
		return TRUE;
	}

	m_fTimeSoFar += FLoop_fPreviousLoopSecs;
	rfUnitCompletePercent = m_fTimeSoFar * m_fOOTotalSecsToComplete;
	FMATH_CLAMPMAX( rfUnitCompletePercent, 1.0f );

	return FALSE;
}

void CFXMeshBuildPartMgr::ForceFinish() {
	FASSERT( m_nState == STATE_IN_USE );

	u32 i;
	CFMtx43A **ppMtxPalette;

	// grab the maxtrix palette
	ppMtxPalette = m_pWorldMesh->GetBoneMtxPalette();

	// call force finish on all active parts, as well as any that haven't started yet
	for( i=m_nOldestWorkingBone; i < m_nNumParts; i++ ) {
		m_paParts[i].ForceFinish( ppMtxPalette );
	}
	UpdateBoundingInfo();

	// we are done, clean up
	m_nNumParts = 0;
	m_pWorldMesh = NULL;
    m_nState = STATE_AVAILABLE;	
}

u32 CFXMeshBuildPartMgr::GetTotalPartCount() {
	if( m_nState != STATE_IN_USE ) {
		return 0;
	}
	return m_nNumParts;
}

void CFXMeshBuildPartMgr::CalculateDropSecs( f32 fSecsToComplete, f32 fUnitOverlap, f32 fNumParts,
											f32 &rfSecsBetweensParts, f32 &rfSecsPerPart ) {
	f32 fTemp = fmath_Div( fSecsToComplete, fNumParts );	
	rfSecsBetweensParts = fTemp - (fUnitOverlap * fTemp);
	rfSecsPerPart = fSecsToComplete - (rfSecsBetweensParts * (fNumParts-1.0f));
}

BOOL CFXMeshBuildPartMgr::Init( const CFVec3A &rFrom, CFWorldMesh *pWorldMesh,
							   f32 fSecsToComplete, f32 fUnitOverlap/*=0.0f*/,
							   u32 nNumAdditionalBones/*=0*/, u32 *panAdditionalBones/*=NULL*/ ) {

	FASSERT( m_nState == STATE_AVAILABLE );
    FASSERT( pWorldMesh && (pWorldMesh->m_pMesh->nUsedBoneCount <= m_nMaxParts) && (pWorldMesh->m_pMesh->nUsedBoneCount > 0) );
	FASSERT( fSecsToComplete > 0.0f );
	
	FMATH_CLAMP( fUnitOverlap, 0.0f, 1.0f );

	u32 i, j, nBoneIndex;
	f32 fTemp;
	CFMtx43A **ppMtxPalette;
	CFXMeshBuildPart *pTempPart;

	m_pWorldMesh = pWorldMesh;
	m_nNumParts = m_pWorldMesh->m_pMesh->nUsedBoneCount;

	FASSERT( nNumAdditionalBones + m_nNumParts <= m_pWorldMesh->m_pMesh->nBoneCount );
	m_nNumParts += nNumAdditionalBones;

	if( m_nNumParts > m_nMaxParts ) {
		DEVPRINTF( "FXMeshBuildPartMgr::Init() - too many bones, the system was inited with %d bones, not the required %d.\n", m_nMaxParts, m_nNumParts );
		m_nNumParts = 0;
		return FALSE;
	}

	CalculateDropSecs( fSecsToComplete, fUnitOverlap, (f32)m_nNumParts, m_fSecsBetweenStarts, fTemp );
	m_fOOPartSecsToComplete = fmath_Inv( fTemp );
	m_fSecsTillNextStart = 0.0f;

	m_fTimeSoFar = 0.0f;
	m_fOOTotalSecsToComplete = fmath_Inv( fSecsToComplete );

	// grab the maxtrix palette
	ppMtxPalette = m_pWorldMesh->GetBoneMtxPalette();

	// determine the size range of the bones so we can slow down larger ones
	FMeshBone_t *pBone = m_pWorldMesh->m_pMesh->pBoneArray;
	f32 fMinBoneR, fMaxBoneR;
	fMinBoneR = fMaxBoneR = pBone->SegmentedBoundSphere_BS.m_fRadius;
	for( i=1; i < m_pWorldMesh->m_pMesh->nUsedBoneCount; i++ ) {
		pBone++;
		fMinBoneR = FMATH_MIN( fMinBoneR, pBone->SegmentedBoundSphere_BS.m_fRadius );
		fMaxBoneR = FMATH_MAX( fMaxBoneR, pBone->SegmentedBoundSphere_BS.m_fRadius );		
	}
	f32 fOOBoneRadiusDelta = fmath_Inv( fMaxBoneR - fMinBoneR );
	f32 fUnitBoneRadius;

	// reset our bone pointer
	pBone = m_pWorldMesh->m_pMesh->pBoneArray;

	for( i=0; i < m_nNumParts; i++ ) {
		if( i < m_pWorldMesh->m_pMesh->nUsedBoneCount ) {
			nBoneIndex = i;
			fUnitBoneRadius = (pBone[i].SegmentedBoundSphere_BS.m_fRadius - fMinBoneR) * fOOBoneRadiusDelta;
		} else {
			nBoneIndex = panAdditionalBones[i - m_pWorldMesh->m_pMesh->nUsedBoneCount];
			fUnitBoneRadius = 0.0f;// the smallest amount of rotation, since we don't know the size of attached meshes
		}

		if( !m_paParts[i].InitPart( rFrom, ppMtxPalette, nBoneIndex, fUnitBoneRadius ) ) {
			// something went very wrong, clean up and fail
			for( j=0; j < i; j++ ) {
				m_paParts[i].ForceFinish( ppMtxPalette );
			}
			m_nNumParts = 0;
			m_pWorldMesh = NULL;
			return FALSE;
		}
		// store a pointer to this part (we'll sort later)
		m_papSortedParts[i] = &m_paParts[i];
	}
	
	// bubble sort the pointers furthest to nearest
	for( i=0; i < (u32)(m_nNumParts-1); i++ ) {
		for( j=i+1; j < m_nNumParts; j++ ) {
			if( m_papSortedParts[j]->m_fDistSqToTravel > m_papSortedParts[i]->m_fDistSqToTravel ) {
				// swap i and j
				pTempPart = m_papSortedParts[i];
				m_papSortedParts[i] = m_papSortedParts[j];
				m_papSortedParts[j] = pTempPart;
			}
		}
	}

	m_nNextBoneToStart = 0;
	m_nOldestWorkingBone = 0;

	// all done and ready to start
	m_nState = STATE_IN_USE;

	// lastly update the mesh's bounding info
	UpdateBoundingInfo();

	return TRUE;	
}

// updates the mesh bounding sphere
void CFXMeshBuildPartMgr::UpdateBoundingInfo() {
	CFVec3A Min, Max, Origin_WS, Origin_MS, OriginToCorner_WS, *pBonePos;
	CFSphere BoundSphere_WS;
	CFMtx43A **pBonePalette;
	u32 i, nNumBones = m_pWorldMesh->m_pMesh->nBoneCount;

	FASSERT( m_nState == STATE_IN_USE );

	// grab the mesh's mtx palette	
	pBonePalette = m_pWorldMesh->GetBoneMtxPalette();

	// Compute bounding sphere radius...
	Origin_WS.Set( pBonePalette[0]->m_vPos );
	for( i=1; i<nNumBones; ++i ) {
		Origin_WS.Add( pBonePalette[i]->m_vPos );
	}
	Origin_WS.Mul( fmath_Inv( (f32)nNumBones ) );

	// Compute min/max cube bounds...
	Min.Set( pBonePalette[0]->m_vPos );
	Max.Set( pBonePalette[0]->m_vPos );
	for( i=1; i<nNumBones; ++i ) {
		pBonePos = &pBonePalette[i]->m_vPos;

		Min.ClampMax( *pBonePos );
		Max.ClampMin( *pBonePos );
	}

	// Compute origin in model space...
	m_pWorldMesh->m_Xfm.m_MtxR.MulPoint( Origin_MS, Origin_WS );

	// Compute vector from origin to maxima...
	OriginToCorner_WS.Sub( Max, Origin_WS );

	m_pWorldMesh->m_BoundSphere_MS.m_Pos = Origin_MS.v3;
	m_pWorldMesh->m_BoundSphere_MS.m_fRadius = OriginToCorner_WS.Mag() * m_pWorldMesh->m_Xfm.m_fScaleR;

	m_pWorldMesh->UpdateTracker();
}
