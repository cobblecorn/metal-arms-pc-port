//////////////////////////////////////////////////////////////////////////////////////
// DisplayList.h - 
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
#ifndef _DISPLAY_LIST_H_
#define _DISPLAY_LIST_H_ 1

#include "fang.h"
#include "fworld.h"
#include "Leaf.h"

class CDisplayList
{
public:
	CDisplayList();
	~CDisplayList();

	BOOL ReadFromFile( cchar *pszFilename );
	BOOL WriteToFile( cchar *pszFilename );
	BOOL InitFromWorld( const FWorld_t *pWorld );

	CLeaf *GetLeaf( const CFVec2 &rXZ );
	CLeaf *GetLeaf( u32 nAcross, u32 nUp );
	CLeaf *GetLeaf( u32 nIndex );
	CLeaf *GetNearestValidLeaf( const CFVec2 &rXZ );
	CLeaf *GetLeafWithLargestDLCount();
	const u16 *GetIntDisplayList( const FWorld_t *pWorld, u32 nAcross, u32 nUp, BOOL bOnlyContainLeafNodes=TRUE );
	void ConvertLeafIndexToColRow( u32 nLeafIndex, u32 &rnCol, u32 &rnRow );
	u32 ConvertLeafColRowToIndex( u32 nCol, u32 nRow );
	const u16 *GetDLWithEveryLeaf();
	void ToggleNodeInDisplayList( const FWorldNode_t *pDLNode, const FWorldNode_t *pNodeToToggleFromDL );
	void RemoveNodeFromDisplayList( const FWorldNode_t *pDLNode, const FWorldNode_t *pNodeToRemove );
	void AddNodeToDisplayList( const FWorldNode_t *pDLNode, const FWorldNode_t *pNodeToAdd );
	void Reset();
	void ResetAllLeafsDL( u32 nRadius );
	void MergeDisplayList( CDisplayList &rSrcDL );
	f32 GetLeafSize();

private:
	void CreateDisplayList( const FWorld_t *pWorld, const FWorldNode_t *pNode, u32 &rnDLCount, CUIntArray &ranDL );
	void RecurseDownToLeaf( const FWorld_t *pWorld, const FWorldNode_t *pNode, u32 &rnDLCount, CUIntArray &ranDL );
	BOOL AreAllChildrenInDL( const u16 *pnIntDL, const FWorldNode_t *pNode, CUIntArray &ranChildDLIndices );

protected:
	u32 m_nTotalLeafs;	// total CLeafs are contained in m_aLeafs ( equals m_nLeafsAcross * m_nLeafsUp )
	u32 m_nUsedLeafs;	// how many leafs are actually used (the non NULL leafs)
	CPtrArray m_aLeafs;	// m_nTotalLeafs of CLeaf pointers, some may be NULL for unused leafs
	f32 m_fLeafSize;	// how large is each leaf
	u32 m_nLeafsAcross;	// how many leafs are across each row (+x from lower left leaf)
	u32 m_nLeafsUp;		// how many leafs are up each column (+z from lower left leaf)
	CFVec2 m_MinXZ;		// the lower left cornor of all leafs
	CPtrArray m_aDL;	// m_nTotalLeafs of u16 pointers, some may be NULL for unused leafs
	u16 *m_pnEveryLeafDL;// a display list with every leaf in it, used to draw the entire world
};

#endif

