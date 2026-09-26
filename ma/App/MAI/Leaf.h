//////////////////////////////////////////////////////////////////////////////////////
// Leaf.h - 
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
// 03/26/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _LEAF_H_
#define _LEAF_H_ 1

#include "fang.h"
#include "fmath.h"

class CLeaf
{ 
public:
	CLeaf();
	~CLeaf();
	s32 GetDLLeafIndex( u32 nIndex );	// returns nIndex'th leaf index in the display list			
	u32 AddToDL( u32 nLeafIndex );		// adds nLeafIndex to this leaf's display list if it is not already apart of it		
	u32 RemoveFromDL( u32 nLeafIndex );	// removes nLeafIndex from this leaf's display list, if it apart of it
	void ToggleDL( u32 nLeafIndex );	// adds nLeafIndex if it is not currently in the DL, removes it if it is in the current DL

	// sets the leaf index of this leaf
	void SetIndex( u32 nLeafIndex )	{ m_nLeafIndex = nLeafIndex; }
	// returns the leaf index of this leaf
	u32 GetIndex()	{ return m_nLeafIndex; }

	// set the leaf pos of this leaf
	void SetLeafXZ( CFVec2 &rXZ )	{ m_XZPos = rXZ; }
	// fill rXZ with this leaf's XZ position in World Space
	void GetLeafXZ( CFVec2 &rXZ )	{ rXZ = m_XZPos; }

	// returns the number of elements in the display list
	u32 GetDLCount()	{ return m_nDLCount; }
	// removes all leaf's from this leaf's display list, including ourself
	void RemoveAllFromDL()	{ m_anDisplayList.RemoveAll(); m_nDLCount = 0; }
	// removes all but ourself from this leaf's display list
	void ResetDL()	{ m_anDisplayList.RemoveAll(); m_anDisplayList.Add( m_nLeafIndex ); m_nDLCount = 1; }

private:
	
protected:
	u32 m_nLeafIndex;	// what index is this leaf in the overall leaf list
	CFVec2 m_XZPos;		// what is the xz position of this leaf (it's lower left corner)	
	u32 m_nDLCount;		// how many items are currently in this leaf's display list
	CUIntArray m_anDisplayList;// m_nDLCount number of leaf indices that can be seen from this leaf (includes our index most probably)
};

#endif
