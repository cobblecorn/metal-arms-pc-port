//////////////////////////////////////////////////////////////////////////////////////
// Leaf.cpp - 
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
// 03/27/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "Leaf.h"



CLeaf::CLeaf() {
	m_nLeafIndex = 0;
	m_XZPos.Zero();
	m_nDLCount = 0;
	m_anDisplayList.RemoveAll();
}

CLeaf::~CLeaf() {
}

// returns nIndex'th leaf index in the display list
// returns -1 if nIndex is invalid
s32 CLeaf::GetDLLeafIndex( u32 nIndex ) {
	
	if( nIndex >= m_nDLCount ) {
		return -1;
	}
	return m_anDisplayList[nIndex];
}

// adds nLeafIndex to this leaf's display list if it is not already apart of it
// returns the index into the display list where nLeafIndex now resides
u32 CLeaf::AddToDL( u32 nLeafIndex ) {
	u32 i;

	// search for nLeafIndex
	for( i=0; i < m_nDLCount; i++ ) {
		if( m_anDisplayList[i] == nLeafIndex ) {
			return i;
		}
	}
	// we didn't find nLeafIndex, add it
	m_nDLCount++;
	return m_anDisplayList.Add( nLeafIndex );
}

// removes nLeafIndex from this leaf's display list, if it apart of it
// returns the number of elements left in the display list after removing nLeafIndex.
u32 CLeaf::RemoveFromDL( u32 nLeafIndex ) {
	u32 i;

	// search for nLeafIndex
	for( i=0; i < m_nDLCount; i++ ) {
		if( m_anDisplayList[i] == nLeafIndex ) {
			m_anDisplayList.RemoveAt( i, 1 );
			m_nDLCount--;
			return m_nDLCount;
		}
	}
	// we didn't find nLeafIndex, nothing to remove
	return m_nDLCount;	
}

void CLeaf::ToggleDL( u32 nLeafIndex ) {
	u32 nOldDLCount;
	
	nOldDLCount = GetDLCount();
	if( nOldDLCount == RemoveFromDL( nLeafIndex ) ) {
		// item was not in the display list, add it
		AddToDL( nLeafIndex );
	}
}
