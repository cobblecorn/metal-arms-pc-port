//////////////////////////////////////////////////////////////////////////////////////
// fbitstream.cpp - 
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
// 01/18/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fbitstream.h"
#include "fmath.h"

CFBitStream::CFBitStream() {
	m_pAllocatedMem = NULL;
	Reset();
}

CFBitStream::~CFBitStream() {
	Reset();
}

// Allows the user to specify just a number of bits and will allocate memory
// to support that number of bits.  Or the user can specify a number of bytes 
// and the number of bits will be based on that instead.  And finally the user 
// can supply their own memory and this function won't allocate its own.
BOOL CFBitStream::Init( u32 nNumBits, u32 nBytes/*=0*/, u8 *pMem/*=NULL*/, BOOL bZeroMem/*=TRUE*/ ) {

	Reset();

	if( !nNumBits ) {
		// no bits requested, we can only proceed if a number of bytes has been passed in
		if( !nBytes ) {
			return FALSE;
		}
		nNumBits = nBytes << 3;
	}
	// determine how much memory we need
	m_nNumBytes = nNumBits >> 3;
	if( (m_nNumBytes << 3) < nNumBits ) {
		m_nNumBytes++;
	}
	m_nNumBits = m_nNumBytes << 3;
	// allocate some memory
	if( !pMem ) {
		m_pAllocatedMem = new u8[m_nNumBytes];
		if( !m_pAllocatedMem ) {
			return FALSE;
		}
		m_pBytes = m_pAllocatedMem;
		fang_MemZero( m_pBytes, m_nNumBytes );
	} else {
		// used the passed in memory
		m_pBytes = pMem;
		if( bZeroMem ) {
			fang_MemZero( m_pBytes, m_nNumBytes );
		}
	}	
	m_nNextBit = 0;

	return TRUE;
}

void CFBitStream::Reset() {
	
	if( m_pAllocatedMem ) {
		delete m_pAllocatedMem;
	}
	m_pAllocatedMem = NULL;
	m_pBytes = NULL;
	m_nNumBytes = 0;
	m_nNumBits = 0;
	m_nNextBit = 0;
}

// returns the previous value stored at nBitIndex (only nNumBitsToSet worth)
// and then sets nValue at that location (again only nNumBitsToSet worth)
u32 CFBitStream::Set( u32 nBitIndex, u32 nValue, s32 nNumBitsToSet ) {
		
	if( !m_pBytes ) {
		return 0;
	}
	if( nNumBitsToSet < 1 ) {
		// nothing to set
		return 0;
	}
	FASSERT( nNumBitsToSet <= 32 );

	// store the previous value
	u32 nPrev = Read( nBitIndex, nNumBitsToSet );

	u32 nTemp, nMask, nShift, nByteNum, nLastBitIndex;
	s32 nBitsToSetInByte, nNewBitNumber, nBitsRemainingInByte, nOrigNumBits;
	
	nByteNum = nBitIndex >> 3;
	nBitsRemainingInByte = 8 - (nBitIndex - (nByteNum << 3));
	nLastBitIndex = 0;
	nOrigNumBits = nNumBitsToSet;
	
	while( nNumBitsToSet ) {
		nNewBitNumber = nBitsRemainingInByte - nNumBitsToSet;

		nBitsToSetInByte = (nNewBitNumber >= 0) ? nNumBitsToSet : nBitsRemainingInByte;

		nShift = nBitsRemainingInByte - nBitsToSetInByte;
		nMask = ((1<<nBitsToSetInByte)-1) << nShift;
		nLastBitIndex += nBitsToSetInByte;

		// shift nValue around to get the desired bits so that we can OR them into the current byte
		nTemp = nValue >> (nOrigNumBits - nLastBitIndex);
		nTemp <<= nShift;
		// clear the bits before we set them
		m_pBytes[nByteNum] &= ~nMask;
		m_pBytes[nByteNum] |= (nTemp & nMask);

		if( nNewBitNumber <= 0 ) {
			nBitsRemainingInByte = 8;
			nByteNum++;
		} else {
			nBitsRemainingInByte -= nBitsToSetInByte;
			FASSERT( nBitsRemainingInByte > 0 );
		}
		nNumBitsToSet -= nBitsToSetInByte;
		FASSERT( nNumBitsToSet >= 0 );
	}
	// keep track of how deep we have written into the bit stream
	m_nNextBit = nBitIndex + nOrigNumBits;

	return nPrev;
}

u32 CFBitStream::Read( u32 nBitIndex, s32 nNumBitsToRead ) {
	
	if( !m_pBytes ) {
		return 0;
	}
	if( nNumBitsToRead < 1 ) {
		// nothing to read
		return 0;
	}
	FASSERT( nNumBitsToRead <= 32 );

	u32 nValue, nMask, nShift, nByteNum;
	s32 nBitsToReadFromByte, nNewBitNumber, nBitsRemainingInByte;
	
	nByteNum = nBitIndex >> 3;
	nBitsRemainingInByte = 8 - (nBitIndex - (nByteNum << 3));
	nValue = 0;

	while( nNumBitsToRead ) {
		nNewBitNumber = nBitsRemainingInByte - nNumBitsToRead;

		nBitsToReadFromByte = (nNewBitNumber >= 0) ? nNumBitsToRead : nBitsRemainingInByte;

		nShift = nBitsRemainingInByte - nBitsToReadFromByte;
		nMask = ((1<<nBitsToReadFromByte)-1) << nShift;

		nValue <<= nBitsToReadFromByte;
		nValue |= ((m_pBytes[nByteNum] & nMask) >> nShift);

		if( nNewBitNumber <= 0 ) {
			nBitsRemainingInByte = 8;
			nByteNum++;
		} else {
			nBitsRemainingInByte -= nBitsToReadFromByte;
			FASSERT( nBitsRemainingInByte > 0 );
		}
		nNumBitsToRead -= nBitsToReadFromByte;
		FASSERT( nNumBitsToRead >= 0 );
	}
	return nValue;
}

u32 CFBitStream::GetNumBitsAvailable() {
	if( !m_pBytes ) {
		return 0;
	}
	return (m_nNumBits - m_nNextBit);
}

u32 CFBitStream::GetTotalBits() {
	if( !m_pBytes ) {
		return 0;
	}
	return m_nNumBits;
}

// returns the Bit index after which no bit has been set yet
u32 CFBitStream::GetNextBitIndex() {
	if( !m_pBytes ) {
		return 0;
	}
	return m_nNextBit;
}

u32 CFBitStream::GetNumBytes() {
	if( !m_pBytes ) {
		return 0;
	}
	return m_nNumBytes;
}

// returns the number of number of bytes currently used
u32 CFBitStream::GetNumBytesUsed() {
	u32 nNextByte;

	if( !m_pBytes ) {
		return 0;
	}
	if( m_nNextBit == 0 ) {
		return 0;
	}
	nNextByte = m_nNextBit + 0x7;
	nNextByte &= ~0x7;
	nNextByte >>= 3;

	return nNextByte;
}

void CFBitStream::MoveNextBitIndexToNextByte() {
	
	if( !m_pBytes ) {
		return;
	}
	m_nNextBit += 0x7;
	m_nNextBit &= ~0x7;
	FMATH_CLAMPMAX( m_nNextBit, m_nNumBits );
}
