//////////////////////////////////////////////////////////////////////////////////////
// fdatapool.cpp - 
//
// Author: Michael Starich   
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
// 08/18/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fdatapool.h"
#include "fmath.h"
#include "fres.h"

CFDataPool::CFDataPool() {
	m_nTotalSize = 0;
	m_nUsedCount = 0;
	m_nNumBlocks = 0;
	m_nNumElementsPerBlock = 0;
	m_nBytesPerElement = 0;
	m_nByteAlignment = 0;

	m_pPool = NULL;
	m_pBlocks = NULL;
}	

BOOL CFDataPool::Create( u32 nBytesPerElement, u32 nNumElements/*=0*/, u32 nAlignment/*=0*/, 
						 u32 nMinElementsPerBlock/*=8*/, u32 nMaxBlocks/*=256*/ ) {
	FResFrame_t ResFrame;
	u32 i;

	if( nNumElements == 0 ) {
		return TRUE;
	}

	FASSERT( nMinElementsPerBlock > 0 );
	FASSERT( nMaxBlocks > 0 );

	ResFrame = fres_GetFrame();

	m_nBytesPerElement = nBytesPerElement;
	m_nByteAlignment = nAlignment;

	// figure out how many blocks we need
	m_nNumBlocks = nNumElements / nMinElementsPerBlock;
	FMATH_CLAMPMAX( m_nNumBlocks, nMaxBlocks );
	
	m_nNumElementsPerBlock = nNumElements / m_nNumBlocks;
	
	// make sure that the min number of sprites are under management
	if( (m_nNumElementsPerBlock * m_nNumBlocks) < nNumElements ) {
		m_nNumElementsPerBlock++;
	}
	FMATH_CLAMPMIN( m_nNumElementsPerBlock, nMinElementsPerBlock );
	
	m_nTotalSize = m_nNumBlocks * m_nNumElementsPerBlock;
	m_nUsedCount = 0;
	
	// allocate the element pool
	m_pPool = fres_AlignedAllocAndZero( m_nTotalSize * m_nBytesPerElement, m_nByteAlignment );
	if( !m_pPool ) {
		goto _ExitCreateWithError;
	}
	// allocate the block array
	m_pBlocks = (u8 *)fres_AlignedAllocAndZero( m_nNumBlocks, m_nByteAlignment );
	if( !m_pBlocks ) {
		goto _ExitCreateWithError;
	}
	// mark all of the blocks as free
	for( i=0; i < m_nNumBlocks; i++ ) {
		m_pBlocks[i] = BLOCK_FREE;
	}

	return TRUE;

_ExitCreateWithError:
	// Failure...
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

// Returns the number of blocks that will be needed for nNumElementsWanted.
// Will also fill in rnNumElementsContained with that actual number of 
// elements that are contained in the returned blocks.
u32 CFDataPool::NumBlockesNeeded( u32 nNumElementsWanted, u32 &rnNumElementsContained ) {
	
	u32 nNumBlocksRequired = nNumElementsWanted / m_nNumElementsPerBlock;
	rnNumElementsContained = nNumBlocksRequired * m_nNumElementsPerBlock;
	if( rnNumElementsContained < nNumElementsWanted ) {
		// add an extra block to cover for the fractional block
		nNumBlocksRequired++;
		rnNumElementsContained += m_nNumElementsPerBlock;
	}
	return nNumBlocksRequired;
}

void *CFDataPool::GetArray( u32 nNumNeeded ) {
	u32 nNumBlocksRequired, i, nSeriesStart, nNumInARow, nNumElementsReturned;
	u8 *pArray = NULL;

	if( !m_nTotalSize || ( nNumNeeded > GetNumFree() ) ) {
		return NULL;
	}
	// find out how many blocks are needed to used to hold nNumElements
	nNumBlocksRequired = NumBlockesNeeded( nNumNeeded, nNumElementsReturned );
	
	// look for nNumBlocksRequired number of continous blocks
	nSeriesStart = 0;
	nNumInARow = 0;
	for( i=0; i < m_nNumBlocks; i++ ) {
		if( m_pBlocks[i] == BLOCK_FREE ) {
			// is this the start of a new run
			if( !nNumInARow ) {
				nSeriesStart = i;
			}
			nNumInARow++;

			if( nNumInARow == nNumBlocksRequired ) {
				pArray = (u8 *)m_pPool;
				pArray += (nSeriesStart * m_nNumElementsPerBlock * m_nBytesPerElement);
				break;
			}
		} else {
			// start a new run at the next free block
			nNumInARow = 0;
		}
	}
	if( !pArray ) {
		// we never found a large enough run of free blocks
		return NULL;
	}

	// mark the blocks as used
	for( i=0; i < nNumBlocksRequired; i++ ) {
		m_pBlocks[ nSeriesStart + i ] = BLOCK_USED;
	}
	// update our counts
	m_nUsedCount += nNumElementsReturned;

	return pArray;
}

void CFDataPool::ReturnToPool( void *pHead, u32 nCount ) {
	u32 i, nElementIndex, nBlockIndex, nBlocksUsed, nElementsUsed;

	if( !m_nTotalSize || !pHead || !nCount ) {
		// the system isn't up and running
		return;
	}
	// compute the index
	nElementIndex = (u32)pHead - (u32)m_pPool;
	nElementIndex /= m_nBytesPerElement;
	FASSERT( nElementIndex < m_nTotalSize );
	// compute the starting block index
	nBlockIndex = nElementIndex / m_nNumElementsPerBlock;
	FASSERT( (nBlockIndex * m_nNumElementsPerBlock) == nElementIndex );
	// compute the number of blocks that nCount uses
	nBlocksUsed = NumBlockesNeeded( nCount, nElementsUsed );
	// mark the blocks as FREE
	for( i=0; i < nBlocksUsed; i++ ) {
		m_pBlocks[ nBlockIndex + i ] = BLOCK_FREE;
	}
	// update our counts
	m_nUsedCount -= nElementsUsed;
}

