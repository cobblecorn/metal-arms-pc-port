//////////////////////////////////////////////////////////////////////////////////////
// ff32hash.cpp - 
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
// 11/16/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "ff32hash.h"

CFf32Hash::CFf32Hash() {
	m_fTolerance = 0.0f;
	m_nFloatsPerOb = 0;
	m_bUnitData = FALSE;
	m_fTolerance2 = 0.0f;
}

CFf32Hash::~CFf32Hash() {
}

BOOL CFf32Hash::SetupFloatHashTable( u32 nFloatsPerOb, u32 nTotalDataBytes, BOOL bUnitData/*=FALSE*/,
									 f32 fTolerance/*=0.001f*/, u8 *pData/*=NULL*/, u32 nObsInListAlready/*=0*/ ) {
	m_fTolerance = fTolerance;
	m_fTolerance2 = m_fTolerance * m_fTolerance;// we do a delta^2 test
	m_nFloatsPerOb = nFloatsPerOb;
	m_bUnitData = bUnitData;
	
	return CFHashTable::InitTable( m_nFloatsPerOb * sizeof( f32 ), nTotalDataBytes, pData, nObsInListAlready );
}

BOOL CFf32Hash::AreObsEqual( u8 *pObInList, u8 *pOb ) {

	if( !pObInList || !pOb ) {
		return FALSE;
	}
	f32 *pfSrc = (f32 *)pObInList;
	f32 *pfDst = (f32 *)pOb;
	f32 fTotal( 0.0f ), fDelta;
	
	// check this ob's data
	for( u32 i=0; i < m_nFloatsPerOb; i++, pfSrc++, pfDst++ ) {
		fDelta = *pfSrc - *pfDst;
		fTotal += (fDelta * fDelta);
		if( fTotal > m_fTolerance2 ) {
			// already out of range
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CFf32Hash::DoesObMatchKey( u8 *pObInList, u8 *pKey ) {
	return CFf32Hash::AreObsEqual( pObInList, pKey );
}

u32 CFf32Hash::GenerateHashValueFromKey( u8 *pKey ) {
	
	FASSERT( pKey );
	f32 *pfData = (f32 *)pKey;
	// start off by doing some bit twiddling on the first 32 bits
	u32 nHashValue = pKey[0] + pKey[3] + 1;
	nHashValue += (pKey[1] << 4);
	nHashValue *= pKey[2] & 0xF8;
	nHashValue |= pKey[2] & 0x7;

	if( !m_bUnitData ) {
		// generally not unit float data
		f32 fTotal = 1.5678f;
		u8 *pnBits = &pKey[2];
		
		for( u32 i=0; i < m_nFloatsPerOb; i++, pfData++, pnBits+=4 ) {
			fTotal *= *pfData;
			nHashValue += (*pnBits << (i&0x3));
		}
		nHashValue += (u32)(fTotal + 8642369.123f);
	} else {
		// generally unit float data
		f32 fTotal = 1234.5678f;
		u8 *pnBits = &pKey[3];
				
		for( u32 i=0; i < m_nFloatsPerOb; i++, pfData++, pnBits+=4 ) {
			fTotal += (*pfData * 369.02468f);
			nHashValue += (*pnBits * (i+1));
		}
		nHashValue += (u32)(fTotal + 8642369.123f);
	}
	return nHashValue;
}

u32 CFf32Hash::GenerateHashValueFromOb( u8 *pOb ) {
	return CFf32Hash::GenerateHashValueFromKey( pOb );
}



