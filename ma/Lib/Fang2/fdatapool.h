//////////////////////////////////////////////////////////////////////////////////////
// fdatapool.h - "manages a pool of data elements, these should not be classes as 
//				  fres_Alloc is used to allocate memory and that doesn't call constructors"
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
#ifndef _FDATAPOOL_H_
#define _FDATAPOOL_H_ 1

#include "fang.h"

// Allocates a pool of data elements and then manages the pool by dividing it
// up into blocks and keeping track of which blocks are used and which are free.

// GetArray() will return a pointer to the head of a continous array of elements.
// When the array is no longer needed, call ReturnToPool() passing in the pointer.

class CFDataPool
{
public:
	CFDataPool();
	BOOL Create( u32 nBytesPerElement, u32 nNumElements=0, u32 nAlignment=0, u32 nMinElementsPerBlock=8, u32 nMaxBlocks=256 );

	BOOL SetSize( u32 nNumElements, u32 nMinElementsPerBlock=8, u32 nMaxBlocks=256 );// allocates a new pool of elements
	void *GetArray( u32 nNumNeeded );// returns a continous array of elements
	void ReturnToPool( void *pHead, u32 nCount );// returns an array of elements back to the pool

	u32 GetSize() const		{ return m_nTotalSize; }
	u32 GetNumUsed() const	{ return m_nUsedCount; }
	u32 GetNumFree() const	{ return m_nTotalSize - m_nUsedCount; }

	FINLINE u32 GetBlockSize( void ) { return m_nNumElementsPerBlock; }
	
private:
	u32 m_nBytesPerElement;	// how many bytes are there per element
	u32 m_nByteAlignment;	// what byte alignment does the data need to be on (default = 0)

	u32 m_nTotalSize;		// the total number of elements that are under management
	u32 m_nUsedCount;		// the number of elements currently allocated
	void *m_pPool;			// the data pool
	
	u32 m_nNumBlocks;		// a block helps us quickly keep track of which m_pPool members are used
	u8 *m_pBlocks;			// the block pool
	u32 m_nNumElementsPerBlock;// number of elements that each block represents

	enum {
		BLOCK_FREE = 0,
		BLOCK_USED,
	};

	u32 NumBlockesNeeded( u32 nNumElementsWanted, u32 &rnNumElementsContained );

protected:
	
	FCLASS_STACKMEM_NOALIGN( CFDataPool );
};

#endif

