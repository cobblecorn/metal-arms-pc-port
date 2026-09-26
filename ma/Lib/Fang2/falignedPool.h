//////////////////////////////////////////////////////////////////////////////////////
// falignedPool.h - General aligned pool system.
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
#ifndef _FALIGNED_POOL_H_
#define _FALIGNED_POOL_H_ 1

#include "fang.h"
#include "fmath.h"

#define FALIGNED_VEC3A_POOL_SIZE		256// the number of elements in a pool of CFVec3As (or CFQuatAs or any 16 byte structs)	

#define _LOG_ELEMENT_USAGE				TRUE// set to TRUE to log the number of elements used and print a message at shutdown time

/////////////////////////////////////////////////////////////////////////////
// general aligned pool of data (the alignment and size must be a power of 2)
FCLASS_NOALIGN_PREFIX class CFAlignedPool {
public:
	CFAlignedPool();
	~CFAlignedPool();

	BOOL CreatePool( u32 nNumElements, u32 nElementByteAlignRequirement, u32 nBytesPerElement );
	// the returned element will be zero'd out on to ensure no strange crash bugs
	void *GetElement();
    void ReturnElement( void *pElement );

	FINLINE u32 GetSizeOfEachElement()		{ FASSERT( m_paElements ); return m_nAlignedBytesPerElement; }
	FINLINE u32 GetNumberOfElements()		{ FASSERT( m_paElements ); return m_nNumElements; }

#if _LOG_ELEMENT_USAGE
	u32 m_nCurrentElementsUsed;// how many elements are used currently (used to optimize the pool size)
	u32 m_nMaxElementsUsedAtOneTime;// how many items were used at 1 time (used to optimize the pool size)
#endif

private:
	u32 m_nNumElements;
	void *m_paElements;// points to m_nNumElements aligned structs

	u32 m_nNumBitFields;
	u32 *m_panBitField;// points to m_nNumBitFields u32s, enough bits for m_nNumElements

	u32 m_nAlignedBytesPerElement;// the total size of an element including any padding, to keep each element properly aligned
	u32 m_nShiftAmount;// number of bits to shift, Right Shifting takes Bytes to Index, Left shifting takes Index to Bytes
	u32 m_nNextSearchIndex;// what element index should we begin our search for the next available element

	FINLINE u32 ElementIndexToBitMaskIndex( u32 nElementIndex )			{ return (nElementIndex >> 5); }
	FINLINE u32 ElementIndexToBitIndex( u32 nElementIndex )				{ return (nElementIndex & 0x1F); }
	FINLINE BOOL DoesMaskHaveAnyUnsetBits( u32 nMask )					{ return (nMask != 0xFFFFFFFF ); }
	FINLINE u32 ComputeElementIndex( u32 nBitMaskIndex, u32 nBitIndex ) { return ((nBitMaskIndex << 5) + nBitIndex); }
	FINLINE u32 ComputeByteOffset( u32 nElementIndex )					{ return (nElementIndex << m_nShiftAmount); }
	void Reset();

	FCLASS_STACKMEM_NOALIGN( CFAlignedPool );
} FCLASS_NOALIGN_SUFFIX;

// a pool of FALIGNED_VEC3A_POOL_SIZE elements
extern CFAlignedPool FAlignedPool_VecPool;

extern BOOL falignedpool_ModuleStartup( void );
extern void falignedpool_ModuleShutdown( void );

#endif