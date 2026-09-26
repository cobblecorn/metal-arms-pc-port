//////////////////////////////////////////////////////////////////////////////////////
// FixedData.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/09/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FixedData.h"



CFixedSizeDataOb::CFixedSizeDataOb() {
	m_bInitialized = FALSE;
	m_bAllocatedMemory = FALSE;
	m_pHeadOb = NULL;
	m_n32BitHeadAddress = 0;
	m_nTotalBytesAllocated = 0;
	m_nSizeOfOb = 0;
	m_nNumBytesUsed = 0;
	m_pCurOb = NULL;
	m_nNumObs = 0;
	m_nMaxObs = 0;
	m_pEndOfMemRange = NULL;
	m_bUse32BitCompare = FALSE;
	m_nCompareIterations = 0;
	m_bFloatData = FALSE;
	m_fTolerance = 0.0f;
}

CFixedSizeDataOb::~CFixedSizeDataOb() {
	Free();
}

BOOL CFixedSizeDataOb::Allocate( u32 nSizeOfOb, u32 nNumBytes, 
								 u8 *pStart/*=NULL*/, u32 nObsInListAlready/*=0*/,
								 BOOL bFloatData/*=FALSE*/,
								 f32 fTolerance/*=0.001f*/ ) {
	
	FASSERT( nSizeOfOb );
	FASSERT( nNumBytes );

	Free();
	
	// allocate our memory if we need to
	if( !pStart ) {
		m_pHeadOb = new u8[nNumBytes];
		if( !m_pHeadOb ) {
			return FALSE;
		}
		m_bAllocatedMemory = TRUE;
	} else {
		m_pHeadOb = pStart;
	}
	m_n32BitHeadAddress = (u32)m_pHeadOb;
	m_nTotalBytesAllocated = nNumBytes;
	m_nSizeOfOb = nSizeOfOb;
	if( nObsInListAlready ) {
		m_nNumBytesUsed = nObsInListAlready * m_nSizeOfOb;
		m_nNumObs = nObsInListAlready;
		m_pCurOb = &m_pHeadOb[m_nNumObs];
	} else {
		m_nNumBytesUsed = 0;
		m_nNumObs = 0;
		m_pCurOb = m_pHeadOb;
	}
	m_nMaxObs = m_nTotalBytesAllocated / m_nSizeOfOb;
	m_pEndOfMemRange = &m_pHeadOb[m_nTotalBytesAllocated];
	m_bUse32BitCompare = (m_nSizeOfOb & 0x3) ? FALSE : TRUE;
	m_nCompareIterations = (m_bUse32BitCompare) ? m_nSizeOfOb >> 2 : m_nSizeOfOb;
	m_bFloatData = bFloatData;
	m_fTolerance = fTolerance;
	m_bInitialized = TRUE;
	return TRUE;
}

void CFixedSizeDataOb::Free() {
	if( m_bAllocatedMemory ) {
		delete[] (u8 *)m_pHeadOb;
		m_pHeadOb = NULL;
	}
	m_bAllocatedMemory = FALSE;
	m_bInitialized = FALSE;
}

u32 CFixedSizeDataOb::GetBytesUsed() {
	if( !m_bInitialized ) {
		return 0;
	}
	return m_nNumBytesUsed;
}

u32 CFixedSizeDataOb::GetObsUsed() {
	if( !m_bInitialized ) {
		return 0;
	}
	return m_nNumObs;
}

// returns the ob index of pOb.
// will return -1 if:
// system is not initialized
// pOb is not a pointer into the managed memory region
// pOb does not fall on a Ob memory boundary
// pOb is NULL
s32 CFixedSizeDataOb::ComputeObIndex( u8 *pOb ) {
	if( !m_bInitialized || !pOb ) {
		return -1;
	}
	s32 nByteOffset = ComputeByteOffsetFromHead( pOb );
	if( nByteOffset < 0 ) {
		return -1;
	}
	u32 nIndex = nByteOffset / m_nSizeOfOb;
	if( (s32)(nIndex * m_nSizeOfOb) == nByteOffset ) {
		return nIndex;
	}
	return -1;// not on an ob boundary
}

// inserts an object into our list and returns the index where it was inserted.
// if bOnlyIfNotInListAlready is TRUE we will search for an object identical to 
// *pOb already in the list, if we find it we'll return the index of it, other wise 
// pOb will be inserted at the end of our list, if there is room enough for it.
// will return -1 if:
// system is not initialized
// pOb is NULL
// there is no more room for obs in the list
s32 CFixedSizeDataOb::InsertObIntoList( u8 *pOb, BOOL bOnlyIfNotInListAlready/*=FALSE*/ ) {
	if( !m_bInitialized || !pOb ) {
		return -1;
	}
	if( bOnlyIfNotInListAlready ) {
		// search the list for this object
		s32 nIndex = IsObAlreadyInList( pOb );
		if( nIndex >= 0 ) {
			// already in the list
			return nIndex;
		}
	}
	// need to add this ob, make sure we have room
	if( m_nNumObs >= m_nMaxObs ) {
		return -1;
	}
	// copy this object into the list
	fang_MemCopy( m_pCurOb, pOb, m_nSizeOfOb );
	// move our current pointer and increase our number of obs
	m_pCurOb += m_nSizeOfOb;
	m_nNumBytesUsed += m_nSizeOfOb;
	m_nNumObs++;

	return (m_nNumObs - 1);
}

// returns an index into our existing list if the data contained in pOb
// exactly matches an ob's data already in the list, 
// will return -1 if:
// system is not initialized
// pOb is NULL
// *pOb is not inserted into the the list of obs
s32 CFixedSizeDataOb::IsObAlreadyInList( u8 *pOb ) {
	if( !m_bInitialized || !pOb ) {
		return -1;
	}
	u32 i, j;

	if( m_bUse32BitCompare ) {
		// compare 32 bits at a time
		if( !m_bFloatData ) {
			u32 *pnSrc = (u32 *)m_pHeadOb;
			u32 *pnDst = (u32 *)pOb;
			u32 *pnSrcTemp;
			for( i=0; i < m_nNumObs; i++ ) {
				pnSrcTemp = pnSrc;
				// check this ob's data
				for( j=0; j < m_nCompareIterations; j++, pnSrcTemp++ ) {
					if( *pnSrcTemp != pnDst[j] ) {
						break;
					}
				}
				if( j == m_nCompareIterations ) {
					// we made it all the way through an entire object, pOb matches
					return i;
				}
				pnSrc += m_nCompareIterations;
			}
		} else {
			// float point data
			f32 *pfSrc = (f32 *)m_pHeadOb;
			f32 *pfDst = (f32 *)pOb;
			f32 fTotal, fDelta;
			for( i=0; i < m_nNumObs; i++ ) {
				fTotal = 0.0f;
				// check this ob's data
				for( j=0; j < m_nCompareIterations; j++ ) {
					fDelta = pfSrc[j] - pfDst[j];
					fTotal += (fDelta * fDelta);
					if( fTotal >= m_fTolerance ) {
						// already out of range
						break;
					}
				}
				if( j == m_nCompareIterations ) {
					// we made it all the way through an entire object, pOb is close enough
					return i;
				}
				pfSrc += m_nCompareIterations;
			}
		}
	} else {
		// compare 8 bits at a time
		u8 *pnSrc = (u8 *)m_pHeadOb;
		for( i=0; i < m_nNumObs; i++ ) {
			// check this ob's data
			for( j=0; j < m_nCompareIterations; j++ ) {
				if( pnSrc[j] != pOb[j] ) {
					break;
				}
			}
			if( j == m_nCompareIterations ) {
				// we made it all the way through an entire object, pOb matches
				return i;
			}
			pnSrc += m_nCompareIterations;
		}
	}
	return -1;
}

u32 CFixedSizeDataOb::GetObSize() {
	if( !m_bInitialized ) {
		return 0;
	}
	return m_nSizeOfOb;
}

// returns a pointer to the nIndex'th ob.
// will return NULL if:
// system is not initialized
// nIndex is an invalid index
u8 *CFixedSizeDataOb::GetObPtr( u32 nIndex ) {
	if( !m_bInitialized || (nIndex >= m_nNumObs) ) {
		return NULL;
	}
	return &m_pHeadOb[nIndex * m_nSizeOfOb];
}

// Returns a pointer to the first available ob,
// THIS FUNCTION ADDS THE OBJECT TO THE LIST
// will return NULL if:
// system is not initialized
// there is no room for another ob
u8 *CFixedSizeDataOb::GetNextFreeOb() {
	if( !m_bInitialized ) {
		return NULL;
	}
	if( m_nNumObs >= m_nMaxObs ) {
		return NULL;
	}
	// move our current pointer and increase our number of obs
	u8 *pOb = m_pCurOb;
	m_pCurOb += m_nSizeOfOb;
	m_nNumObs++;
	m_nNumBytesUsed += m_nSizeOfOb;

	return pOb;
}

// Returns the bytes offset from the head of our managed ob memory.
// will return -1 if:
// system is not initialized
// pOb is not a pointer into the managed memory region
// pOb is NULL
s32 CFixedSizeDataOb::ComputeByteOffsetFromHead( u8 *pOb ) {
	if( !m_bInitialized || !pOb ) {
		return -1;
	}
	u32 n32BitObAddress = (u32)pOb;
	if( (n32BitObAddress < m_n32BitHeadAddress) ||
		(n32BitObAddress >= (u32)m_pEndOfMemRange) ) {
		// pointer is outside of our range
		return -1;
	}
	s32 nOffset = n32BitObAddress - m_n32BitHeadAddress;

	return nOffset; 
}