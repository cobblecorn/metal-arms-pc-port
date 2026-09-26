//////////////////////////////////////////////////////////////////////////////////////
// fhash.h - "generic hash table routines"
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
// 11/15/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FHASH_H_
#define _FHASH_H_ 1

#include "fang.h"

#define FHASH_MAX_HASH_ENTRIES		( 0x00400000 )

// abstract base class for hash classes, perhaps strings or other types
class CFHashTable
{
public:
	CFHashTable();
	~CFHashTable();

	BOOL InitTable( u32 nSizeOfOb, u32 nTotalDataBytes, u8 *pData=NULL, u32 nObsInListAlready=0 );
	void Free();
	s32 ComputeDataIndex( u8 *pOb );
	s32 InsertDataObIntoList( u8 *pOb );
	u8 *GetDataObByIndex( u32 nDataIndex );
	u8 *GetDataObByKey( u8 *pKey );
	s32 ComputeByteOffsetFromHead( u8 *pOb );
	u32 GetDataBytesUsed()	{ return m_nTotalDataBytes; }
	u32 GetDataObsUsed()	{ return m_nNumDataObs; }
	u32 GetDataObSize()		{ return m_nSizeOfOb; }
	// stats functions
	u32 GetNumCollisions()	{ return m_nInfoNumCollisions; }
	u32 GetMaxDepth()		{ return m_nInfoMaxDepth; }
	f32 GetAverageDepth();
	void IncreaseCompareKey() { m_nCurrentCompareKey++; }
	
	virtual u32 GenerateHashValueFromKey( u8 *pKey );// (DEFAULT) uses every byte to generate a hash value
	virtual u32 GenerateHashValueFromOb( u8 *pOb );// (DEFAULT) uses every byte to generate a hash value
	virtual BOOL AreObsEqual( u8 *pObInList, u8 *pOb );// (DEFAULT) does a byte for byte comparsion
	virtual BOOL DoesObMatchKey( u8 *pObInList, u8 *pKey );// (DEFAULT) assumes that pKey is an ob and 
														   // does a byte for byte comparison
private:

protected:
	BOOL m_bInitialized;
	u8 *m_pAllocatedMemory;		// memory we allocated, this memory needs to be freed by us
	u32 m_nTotalBytesAllocated;	// total memory allocated for use by this ob manager
	
	u8 *m_pHeadOb;				// pointer to first data ob
	u32 m_nTotalDataBytes;		// how many bytes follow m_pHeadOb for the Data Table
	
	s32 *m_pnHashTable;			// pointer to the first hash table entry
	u32 m_nTotalHashBytes;		// how many bytes follow m_pnHashTable for the Hash Table
	u32 m_nMaxHashEntries;		// how many hash entries are there, always a power of 2
		
	u32 m_nSizeOfOb;			// size in bytes of each data ob 
	u32 m_nNumDataBytesUsed;	// how many data bytes have been used by all of the obs
	u32 m_nNumDataObs;			// how many obs have been inserted
	u32 m_nMaxDataObs;			// how many obs can fit into our data ob space
	u8 *m_pAvailDataOb;			// the first available data ob
	// optimizations
	u32 m_n32BitHeadAddress;	// our pre-casted 32 bit head address
	u8 *m_pEndOfMemRange;		// the 1st byte outside of our managed memory area
	BOOL m_bUse32BitCompare;	// if nSizeOfOb is a multiple of 4 bytes, then we can compare 32 bits at a time
	u32 m_nCompareIterations;	// how many u32s or u8s do we need to compare - relies on m_bUse32BitCompare
	u32 m_nHashValue2HashIndex;	// use this to get a hash index from a hashed value (m_nMaxHashEntries-1)
	// stats
	u32 m_nInfoNumCollisions;
	u32 m_nInfoMaxDepth;
	f32 m_fInfoAvgEntryDepth;

	u8 m_nCurrentCompareKey;	// allows us to stop compressing with old entries and only compare against newer groups
};

#endif

