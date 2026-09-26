//////////////////////////////////////////////////////////////////////////////////////
// FixedData.h - 
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
#ifndef _FIXED_DATA_H_
#define _FIXED_DATA_H_ 1

#include "fang.h"

class CFixedSizeDataOb
{
public:
	CFixedSizeDataOb();
	~CFixedSizeDataOb();

	BOOL Allocate( u32 nSizeOfOb, u32 nNumBytes, u8 *pStart=NULL, 
				   u32 nObsInListAlready=0, BOOL bFloatData=FALSE, f32 fTolerance=0.001f );
	void Free();
	u32 GetBytesUsed();
	u32 GetObsUsed();
	s32 ComputeObIndex( u8 *pOb );
	s32 InsertObIntoList( u8 *pOb, BOOL bOnlyIfNotInListAlready=FALSE );
	s32 IsObAlreadyInList( u8 *pOb );
	u32 GetObSize();
	u8 *GetObPtr( u32 nIndex );
	u8 *GetNextFreeOb();
	s32 ComputeByteOffsetFromHead( u8 *pOb );

private:

protected:
	BOOL m_bInitialized;		// are we up and running
	BOOL m_bAllocatedMemory;	// did we allocate memory, if TRUE, then we need to free this memory
	u8 *m_pHeadOb;				// pointer to first ob
	u32 m_n32BitHeadAddress;	// our pre-casted 32 bit head address
	u32 m_nTotalBytesAllocated;	// total memory allocated for use by this ob manager
	u32 m_nSizeOfOb;			// size in bytes of each ob 
	u32 m_nNumBytesUsed;		// how many bytes have been used by all of the obs	
	u8 *m_pCurOb;				// the end of currently allocated memory
	u32 m_nNumObs;				// how many obs have been inserted
	u32 m_nMaxObs;				// how many obs can fit into our memory space
	u8 *m_pEndOfMemRange;		// the 1st byte outside of our managed memory area
	BOOL m_bUse32BitCompare;	// if nSizeOfOb is a multiple of 4 bytes, then we can compare 32 bits at a time
	u32 m_nCompareIterations;	// how many u32s or u8s do we need to compare - relies on m_bUse32BitCompare
	BOOL m_bFloatData;			// this data is floating point data, we have to do a special compare
	f32 m_fTolerance;			// this is the float value that we check against when using float comparisons
};

#endif

