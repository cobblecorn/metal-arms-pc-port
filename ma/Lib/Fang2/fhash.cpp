//////////////////////////////////////////////////////////////////////////////////////
// fhash.cpp - 
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
#include "fang.h"
#include "fhash.h"
#include "fmath.h"

#define _HASH_UNUSED_INDEX					-1
#define _INDEX_MASK							0x00FFFFFF
#define _EXTRACT_KEY( x )					((char)( ( (x) & 0x7F000000) >> 24 ))
#define _SET_KEY_AND_INDEX( nIndex, nKey )	( (((u32)(nKey & 0xFF)) << 24) | (nIndex & _INDEX_MASK) )

CFHashTable::CFHashTable() {
	m_bInitialized = FALSE;
	m_pAllocatedMemory = NULL;
	m_nTotalBytesAllocated = 0;
	m_pHeadOb = NULL;
	m_nTotalDataBytes = 0;
	m_pnHashTable = NULL;
	m_nTotalHashBytes = 0;
	m_nMaxHashEntries = 0;
	m_nSizeOfOb = 0;
	m_nNumDataBytesUsed = 0;
	m_nNumDataObs = 0;
	m_nMaxDataObs = 0;
	m_pAvailDataOb = NULL;
	m_n32BitHeadAddress = 0;
	m_pEndOfMemRange = NULL;
	m_bUse32BitCompare = 0;
	m_nCompareIterations = 0;
	m_nHashValue2HashIndex = 0;
	m_nInfoNumCollisions = 0;
	m_nInfoMaxDepth = 0;
	m_fInfoAvgEntryDepth = 0.0f;
}

CFHashTable::~CFHashTable() {
	Free();
}

BOOL CFHashTable::InitTable( u32 nSizeOfOb, u32 nTotalDataBytes,
							 u8 *pData/*=NULL*/, u32 nObsInListAlready/*=0*/ ) {
	FASSERT( nSizeOfOb );
	FASSERT( nTotalDataBytes );

	Free();

	m_nSizeOfOb = nSizeOfOb;
	m_nTotalDataBytes = nTotalDataBytes;
	m_nMaxDataObs = m_nTotalDataBytes / m_nSizeOfOb;
	m_nMaxHashEntries = fmath_FindNextPowerOf2( m_nMaxDataObs << 1 );
	FASSERT( m_nMaxHashEntries <= FHASH_MAX_HASH_ENTRIES );
	m_nTotalHashBytes = m_nMaxHashEntries * sizeof( s32 );
	// allocate our memory
	if( !pData ) {
		// we need both the hash table and data ob memory
		m_nTotalBytesAllocated = m_nTotalDataBytes + m_nTotalHashBytes;
		m_pAllocatedMemory = new u8[m_nTotalBytesAllocated];
		if( !m_pAllocatedMemory ) {
			return FALSE;
		}
		// fixup our pointers
		m_pHeadOb = m_pAllocatedMemory;
		m_pnHashTable = (s32 *)&m_pAllocatedMemory[m_nTotalDataBytes];
	} else {
		// just allocate enough memory for our hash table
		m_nTotalBytesAllocated = m_nTotalHashBytes;
		m_pAllocatedMemory = new u8[m_nTotalBytesAllocated];
		if( !m_pAllocatedMemory ) {
			return FALSE;
		}
		// fixup our pointers
		m_pHeadOb = pData;
		m_pnHashTable = (s32 *)m_pAllocatedMemory;
	}
	// must set the entire hash table to _HASH_UNUSED_INDEX
	u32 i;
	for( i=0; i < m_nMaxHashEntries; i++ ) {
		m_pnHashTable[i] = _HASH_UNUSED_INDEX;
	}
	// take care of the obs already in the list
	if( nObsInListAlready ) {
		m_nNumDataObs = nObsInListAlready;
		m_nNumDataBytesUsed = m_nNumDataObs * m_nSizeOfOb;
		m_pAvailDataOb = &m_pHeadOb[m_nNumDataObs];
	} else {
		m_nNumDataObs = 0;
		m_nNumDataBytesUsed = 0;
		m_pAvailDataOb = m_pHeadOb;
	}
	// our optimization vars
	m_n32BitHeadAddress = (u32)m_pHeadOb;
	m_pEndOfMemRange = &m_pHeadOb[m_nTotalDataBytes];
	m_bUse32BitCompare = (m_nSizeOfOb & 0x3) ? FALSE : TRUE;
	m_nCompareIterations = (m_bUse32BitCompare) ? m_nSizeOfOb >> 2 : m_nSizeOfOb;
	m_nHashValue2HashIndex = m_nMaxHashEntries - 1;
	// our stat vars
	m_nInfoNumCollisions = 0;
	m_nInfoMaxDepth = 1;
	m_fInfoAvgEntryDepth = 1.0f;
	m_bInitialized = TRUE;

	m_nCurrentCompareKey = 0;
	// now we can hash any objects in the list already
	if( m_nNumDataObs ) {
		u32 nHashIndex;
		u8 *pOb = m_pHeadOb;
		for( i=0; i < m_nNumDataObs; i++, pOb += m_nSizeOfOb ) {
			nHashIndex = GenerateHashValueFromOb( pOb );
			nHashIndex &= m_nHashValue2HashIndex;
			while( m_pnHashTable[nHashIndex] != _HASH_UNUSED_INDEX ) {
				nHashIndex++;
				nHashIndex &= m_nHashValue2HashIndex;
			}
			m_pnHashTable[nHashIndex] = i;
		}
	}	
	return TRUE;
}

void CFHashTable::Free() {
	if( m_pAllocatedMemory ) {
		delete m_pAllocatedMemory;
		m_pAllocatedMemory = NULL;
		m_pHeadOb = NULL;
		m_pnHashTable = NULL;
		m_pAvailDataOb = NULL;
	}
	m_bInitialized = FALSE;
}

// returns the data ob index of pOb.
// will return -1 if:
// system is not initialized
// pOb is not a pointer into the managed data ob memory region
// pOb does not fall on a Ob memory boundary
// pOb is NULL
s32 CFHashTable::ComputeDataIndex( u8 *pOb ) {
	
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
	return -1;// not on an data ob boundary
}

// returns the index into the data ob list where *pOb was either 
// added (to the end) or where it already resided.
// will return -1 if:
// system is not initialized
// pOb is NULL
// there is no more room for any obs in the list
s32 CFHashTable::InsertDataObIntoList( u8 *pOb ) {
	
	if( !m_bInitialized || !pOb ) {
		return -1;
	}
	u32 nHashValue = GenerateHashValueFromOb( pOb );
	u32 nHashIndex = nHashValue & m_nHashValue2HashIndex;
	// see if we are already in the ob list
	u8 *pObInList;
	u32 nCollisions = 0;
	u32 nIndex;
	u8 nKey;
	while( m_pnHashTable[nHashIndex] != _HASH_UNUSED_INDEX ) {
		nIndex = m_pnHashTable[nHashIndex] & _INDEX_MASK;
		nKey = _EXTRACT_KEY( m_pnHashTable[nHashIndex] );			

		pObInList = GetDataObByIndex( nIndex );
		if( !pObInList ) {
			break;
		}
		// compare the 2 obs
		if( nKey == m_nCurrentCompareKey && 
			AreObsEqual( pObInList, pOb ) ) {
			return nIndex;
		}
		nCollisions++;
		// advance our hash index
		nHashIndex++;
		nHashIndex &= m_nHashValue2HashIndex;
	}
	// need to add this ob, make sure we have room
	if( m_nNumDataObs >= m_nMaxDataObs ) {
		return -1;
	}
	// copy this object into the list
	fang_MemCopy( m_pAvailDataOb, pOb, m_nSizeOfOb );
	// update our hash table
	nIndex = m_nNumDataObs;
	m_pnHashTable[nHashIndex] = _SET_KEY_AND_INDEX( nIndex, m_nCurrentCompareKey );
	// move our current pointer and increase our number of obs
	m_pAvailDataOb += m_nSizeOfOb;
	m_nTotalDataBytes += m_nSizeOfOb;
	m_nNumDataObs++;
	// update our stats
	if( nCollisions ) {
		m_nInfoNumCollisions++;
		if( nCollisions > m_nInfoMaxDepth ) {
			m_nInfoMaxDepth = nCollisions;
		}
	}

	return nIndex;
}

// returns a pointer to the nDataIndex'th ob.
// will return NULL if:
// system is not initialized
// nDataIndex is an invalid index
u8 *CFHashTable::GetDataObByIndex( u32 nDataIndex ) {
	
	if( !m_bInitialized || (nDataIndex >= m_nNumDataObs) ) {
		return NULL;
	}
	return &m_pHeadOb[nDataIndex * m_nSizeOfOb];
}

// returns a pointer to the object that pKey hashs to.
// will return NULL if:
// system is not initialized
// pKey hashes to obs that don't contain the pKey
// pKey has not been inserted into the table
// pKey is NULL
u8 *CFHashTable::GetDataObByKey( u8 *pKey ) {

	if( !m_bInitialized || !pKey ) {
		return NULL;
	}
	u32 nHashValue = GenerateHashValueFromKey( pKey );
	u32 nHashIndex = nHashValue & m_nHashValue2HashIndex;
	u8 *pOb;
	u32 nIndex;
	while( m_pnHashTable[nHashIndex] != _HASH_UNUSED_INDEX ) {
		nIndex = m_pnHashTable[nHashIndex] & _INDEX_MASK;
	
		pOb = GetDataObByIndex( nIndex );
		if( !pOb ) {
			return NULL;
		}
		// compare the 2 obs
		if( DoesObMatchKey( pOb, pKey ) ) {
			return pOb;
		}
		// advance our hash index
		nHashIndex++;
		nHashIndex &= m_nHashValue2HashIndex;
	}
	return NULL;
}

// Returns the bytes offset from the head of our managed data ob memory.
// will return -1 if:
// system is not initialized
// pOb is not a pointer into the managed data memory region
// pOb is NULL
s32 CFHashTable::ComputeByteOffsetFromHead( u8 *pOb ) {

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

f32 CFHashTable::GetAverageDepth() {
	
	if( !m_bInitialized ) {
		return 0.0f;
	}
	u32 i;
	BOOL bInCluster( FALSE );
	s32 *pnHashTable = m_pnHashTable;
	f32 fTotalDepth( 0.0f ), fNumClusters( 0.0f );
	
	for( i=0; i < m_nMaxHashEntries; i++, pnHashTable++ ) {
		if( !bInCluster ) {
			// looking for the start of a new cluster
			if( *pnHashTable != _HASH_UNUSED_INDEX ) {
				bInCluster = TRUE;
				fNumClusters += 1.0f;
				fTotalDepth += 1.0f;
			}
		} else {
			// we are in the middle of a cluster
			if( *pnHashTable != _HASH_UNUSED_INDEX ) {
				fTotalDepth += 1.0f;
			} else {
				// this is the end of a cluster
				bInCluster = FALSE;
			}
		}
	}
	// compute our average depth stat
	m_fInfoAvgEntryDepth = (fNumClusters > 0.0f) ? fTotalDepth / fNumClusters : 0.0f;

	return m_fInfoAvgEntryDepth;
}

// returns a hash value based on the assumption that pKey is an ob
// uses every byte to compute this key
u32 CFHashTable::GenerateHashValueFromKey( u8 *pKey ) {
	// so we only implement the default hash value once
	return CFHashTable::GenerateHashValueFromOb( pKey );
}

// returns a hash value using every byte to compute this key
u32 CFHashTable::GenerateHashValueFromOb( u8 *pOb ) {
	u32 nKey, i;
/*
	while( *pszString ) {
		nKey = ( nKey << 4 ) + *pszString++;
		if( i = (nKey & 0xF0000000) ) {
			nKey ^= i >> 24;
		}
		nKey &= ~i;
	}
	return nKey;
*/
#if 0
	// works great for strings, but not too good for general u32 data
	u8 c, cFirstChar;
	// set an initial key value
	cFirstChar = *pOb;
	nKey = 0x9e3779b9;
	for( i=1; i <= m_nSizeOfOb; i++ ) {
		c = *pOb++;
		nKey += (c << (c & 0x3));
		nKey += (c * i);
	}
	nKey += (cFirstChar + c + i) << (cFirstChar & 0x3);
	return nKey;
#else
	u8 c, cFirstChar;
	u32 nTemp;
	// set an initial key value
	cFirstChar = *pOb;
	nKey = 0x9e3779b9;
	for( i=1; i <= m_nSizeOfOb; i++ ) {
		c = *pOb++;
		nTemp = (c + i) * (5 * i);
		nKey += ( nTemp << (nTemp & 0x7));
		nKey ^= nTemp;
		nKey *= i;
	}
	cFirstChar += 23;// ensure this is not zero
	c += 13;// ensure this is not zero
	nKey += (cFirstChar + c) << ( (cFirstChar * c) & 0x3 );
	nKey += m_nMaxHashEntries;
	return nKey;
#endif
}

// returns TRUE if the two obs are bitwise exact
// FALSE otherwise
BOOL CFHashTable::AreObsEqual( u8 *pObInList, u8 *pOb ) {
	
	if( !m_bInitialized || !pObInList || !pOb ) {
		return FALSE;
	}
	u32 i;

	if( m_bUse32BitCompare ) {
		// compare 32 bits at a time
		u32 *pn1 = (u32 *)pObInList;
		u32 *pn2 = (u32 *)pOb;
		for( i=0; i < m_nCompareIterations; i++ ) {
			if( pn1[i] != pn2[i] ) {
				return FALSE;
			}
		}
		// we made it all the way through an entire object, pOb matches
		return TRUE;
	} else {
		// compare 8 bits at a time
		for( i=0; i < m_nCompareIterations; i++ ) {
			if( pObInList[i] != pOb[i] ) {
				return FALSE;
			}
		}
		// we made it all the way through an entire object, pOb matches
		return TRUE;
	}
	return FALSE;
}

// assumes that pKey is an ob
// returns TRUE if the two obs are bitwise exact
// FALSE otherwise
BOOL CFHashTable::DoesObMatchKey( u8 *pObInList, u8 *pKey ) {
	return CFHashTable::AreObsEqual( pObInList, pKey );
}