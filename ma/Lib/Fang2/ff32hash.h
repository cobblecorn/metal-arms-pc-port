//////////////////////////////////////////////////////////////////////////////////////
// ff32hash.h - "float hashing class derived from CFHashTable"
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
#ifndef _FF32HASH_H_
#define _FF32HASH_H_ 1

#include "fang.h"
#include "fhash.h"

class CFf32Hash : public CFHashTable
{
public:
	CFf32Hash();
	~CFf32Hash();

	BOOL SetupFloatHashTable( u32 nFloatsPerOb, u32 nTotalDataBytes, BOOL bUnitData=FALSE, 
							  f32 fTolerance=0.001f, u8 *pData=NULL, u32 nObsInListAlready=0 );
	f32 GetToleranceLevel()						{ return m_fTolerance; }
	void SetToleranceLevel( f32 fNewTolerance ) { m_fTolerance = fNewTolerance; }

	virtual BOOL AreObsEqual( u8 *pObInList, u8 *pOb );
	virtual BOOL DoesObMatchKey( u8 *pObInList, u8 *pKey );

	virtual u32 GenerateHashValueFromKey( u8 *pKey );
	virtual u32 GenerateHashValueFromOb( u8 *pOb );

private:

protected:
	f32 m_fTolerance;
	u32 m_nFloatsPerOb;
	BOOL m_bUnitData;
	f32 m_fTolerance2;
};

#endif

