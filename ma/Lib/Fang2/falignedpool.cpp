//////////////////////////////////////////////////////////////////////////////////////
// falignedPool.cpp - General aligned pool system.
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
#include "falignedpool.h"
#include "fres.h"

CFAlignedPool FAlignedPool_VecPool;

BOOL falignedpool_ModuleStartup( void ) {
	FResFrame_t hResFrame = fres_GetFrame();

	// create the vec pool
	if( !FAlignedPool_VecPool.CreatePool( FALIGNED_VEC3A_POOL_SIZE, 
										  FCLASS_BYTE_ALIGN, 
										  FMATH_MAX( sizeof( CFQuatA ), sizeof( CFVec3A ) ) ) ) {
		goto _EXIT_WITH_ERROR;
	}

	return TRUE;

_EXIT_WITH_ERROR:
	DEVPRINTF( "falignedpool: Couldn't allocated the needed pools.\n" );
	fres_ReleaseFrame( hResFrame );
	return FALSE;
}

void falignedpool_ModuleShutdown( void ) {
#if _LOG_ELEMENT_USAGE
	DEVPRINTF( "falignedpool: Vec Pool stats: %d of %d used at one time, you could save %d bytes by lowering the pool size to %d.\n",
				FAlignedPool_VecPool.m_nMaxElementsUsedAtOneTime,
				FAlignedPool_VecPool.GetNumberOfElements(),
				(FAlignedPool_VecPool.GetNumberOfElements() - FAlignedPool_VecPool.m_nMaxElementsUsedAtOneTime) * FAlignedPool_VecPool.GetSizeOfEachElement(),
				FAlignedPool_VecPool.m_nMaxElementsUsedAtOneTime );
#endif
}


/////////////////////////
// CFAlignedPool methods
/////////////////////////

CFAlignedPool::CFAlignedPool() {
	Reset();
}

CFAlignedPool::~CFAlignedPool() {
	Reset();
}

BOOL CFAlignedPool::CreatePool( u32 nNumElements, u32 nElementByteAlignRequirement, u32 nBytesPerElement ) {
	FASSERT( nNumElements );
	FASSERT( nBytesPerElement );
	FASSERT( fmath_IsPowerOf2( nElementByteAlignRequirement, FALSE ) );
	FASSERT( fmath_IsPowerOf2( nBytesPerElement, FALSE ) );

	if( m_paElements ) {
		Reset();
	}

	FResFrame_t hResFrame = fres_GetFrame();
	
	// compute how much memory will be needed for the array and the bit field
	m_nAlignedBytesPerElement = FMATH_BYTE_ALIGN_UP( nBytesPerElement, nElementByteAlignRequirement );
	m_nNumElements = nNumElements;
	u32 nElementBytesNeeded = m_nAlignedBytesPerElement * m_nNumElements;
	m_nNumBitFields = ElementIndexToBitMaskIndex( m_nNumElements - 1 );
	m_nNumBitFields++;

	// compute the number of unused bits
	u32 nUnusedBits = m_nNumElements - (m_nNumBitFields * 32);
	FASSERT( nUnusedBits < 32 );	

	// allocate memory
	m_paElements = fres_AlignedAllocAndZero( nElementBytesNeeded, nElementByteAlignRequirement );
	if( !m_paElements ) {
		goto _ExitWithError;
	}
	m_panBitField = (u32 *)fres_AlignedAllocAndZero( m_nNumBitFields * sizeof( u32 ), 4 );
	if( !m_panBitField ) {
		goto _ExitWithError;
	}
	// set all bits in the last bit mask that will never be used
	u32 i;
	for( i=0; i < nUnusedBits; i++ ) {
		FMATH_SETBIT( m_panBitField[m_nNumBitFields-1], 31-i );
	}
	
	m_nShiftAmount = fmath_FindFirstSetBit( m_nAlignedBytesPerElement );
	m_nNextSearchIndex = 0;

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( hResFrame );
	Reset();

	return FALSE;
}

void *CFAlignedPool::GetElement() {
	FASSERT( m_paElements );

	// find an available element by searching the bitfield for an unset bit
	u32 nBitMaskIndex = ElementIndexToBitMaskIndex( m_nNextSearchIndex );
	u32 nBitMaskCount = 0;
	u32 *pnBitMask;
	u32 nBitIndex;
	void *pElement;

	do {
		pnBitMask = &m_panBitField[nBitMaskIndex];

		// find the first bit that is not set		
		if( DoesMaskHaveAnyUnsetBits( *pnBitMask ) ) {
			// there is a bit that is not set, find it
			nBitIndex = 0;
			while( *pnBitMask & (1<<nBitIndex) ) {
				nBitIndex++;
			}
			// ok, the nBitIndex'th Bit is not set, this is the first unused element
			m_nNextSearchIndex = ComputeElementIndex( nBitMaskIndex, nBitIndex );
			pElement = (void *)( (u32)m_paElements + ComputeByteOffset( m_nNextSearchIndex ) );
			// mark the bitmask that this element is in use
			FASSERT( !(*pnBitMask & (1<<nBitIndex)) );
			FMATH_SETBIT( *pnBitMask, nBitIndex );

			// update the element that we should check next
			m_nNextSearchIndex++;
			if( m_nNextSearchIndex >= m_nNumElements ) {
				m_nNextSearchIndex = 0;
			}
			// zero the data out
			fang_MemZero( pElement, m_nAlignedBytesPerElement );
#if _LOG_ELEMENT_USAGE
			m_nCurrentElementsUsed++;
			if( m_nCurrentElementsUsed > m_nMaxElementsUsedAtOneTime ) {
				m_nMaxElementsUsedAtOneTime = m_nCurrentElementsUsed;
			}
#endif
			return pElement;
		}

		// move to the next bit mask
		nBitMaskIndex++;
		if( nBitMaskIndex >= m_nNumBitFields ) {
			nBitMaskIndex = 0;
		}
        nBitMaskCount++;
	} while( nBitMaskCount < m_nNumBitFields );

	return NULL;
}

void CFAlignedPool::ReturnElement( void *pElement ) {
	FASSERT( m_paElements );

	// compute the index of pElement
	u32 nIndex = (u32)pElement - (u32)m_paElements;
	nIndex >>= m_nShiftAmount;

	FASSERT( nIndex < m_nNumElements );

	// clear the nIndex'th bit to make pElement available
	u32 *pnBitMask = &m_panBitField[ ElementIndexToBitMaskIndex( nIndex ) ];
	u32 nBitIndex = ElementIndexToBitIndex( nIndex );
	FASSERT( *pnBitMask & (1<<nBitIndex) );
	FMATH_CLEARBIT( *pnBitMask, nBitIndex );
#if _LOG_ELEMENT_USAGE
	m_nCurrentElementsUsed--;
#endif
}

void CFAlignedPool::Reset() {
	m_nNumElements = 0;
	m_nAlignedBytesPerElement = 0;
	m_nNextSearchIndex = 0;
	m_paElements = NULL;
	m_panBitField = NULL;
	m_nShiftAmount = 0;
#if _LOG_ELEMENT_USAGE
	m_nCurrentElementsUsed = 0;
	m_nMaxElementsUsedAtOneTime = 0;
#endif
}