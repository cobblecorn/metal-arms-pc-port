//////////////////////////////////////////////////////////////////////////////////////
// fbitstream.h - 
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
// 01/17/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FBIT_STREAM_H_
#define _FBIT_STREAM_H_ 1

#include "fang.h"

class CFBitStream {

public:
	CFBitStream();
	~CFBitStream();

	BOOL Init( u32 nNumBits, u32 nBytes=0, u8 *pMem=NULL, BOOL bZeroMem=TRUE );
	void Reset();
	u32 Set( u32 nBitIndex, u32 nValue, s32 nNumBitsToSet );
	u32 Read( u32 nBitIndex, s32 nNumBitsToRead );
	
	u32 GetNumBitsAvailable();
	u32 GetTotalBits();
	u32 GetNextBitIndex();
	u32 GetNumBytes();
	u32 GetNumBytesUsed();
	void MoveNextBitIndexToNextByte();

private:
	u8 *m_pAllocatedMem;
	u8 *m_pBytes;
	u32 m_nNumBytes;
	u32 m_nNumBits;
	u32 m_nNextBit;

protected:

};

#endif

