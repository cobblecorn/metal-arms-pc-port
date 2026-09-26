//////////////////////////////////////////////////////////////////////////////////////
// WorldDef.h - contains the structures that make up the "World" format
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
// 01/25/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WORLD_DEF_H_
#define _WORLD_DEF_H_ 1

#include "fang.h"

typedef struct {
	u8 nCol;// x direction
	u8 nRow;// z direction
	u16 nParentIndex;		// Index to the parent node (0xFFFF=none) (from parent Level_t.aNodes[] array)
	CFVec3 Center;
	CFVec3 Min; // from center
	CFVec3 Max; // from center
	u32 nTris;// empty if == 0
	CPtrArray aTris;// *Kong Tris
	u32 nMats;
	CPtrArray aMats;// *MatTracker_t

	void *pUserData;// can be used by systems to attach data to a leaf, (NOT USED BY THE WORLD SYSTEM)
} Leaf_t;

typedef struct {
	u32 nMatID;// this is the ApeMat casted to a u32
	u32 nNumTris;
	u32 nTriStartIndex;// into a Leaf's aTris array
} MatTracker_t;

typedef struct {
	u16 nNodeCol;			// Node column number (0: minimum X)
	u16 nNodeRow;			// Node row number (0: minimum Z)

	CFVec3 Center;
	CFVec3 Min; // from center
	CFVec3 Max; // from center

	u16 nParentIndex;		// Index to the parent node (0xFFFF if this is the root node) (0xFFFF=none) (from parent Level_t.aNodes[] array)
	u16 nNumChildren;		// how many children does this node have
	u16 anChildIndex[4];	// Indices to the 4 children nodes (0xFFFF=none)

	u32 nNumLeafChildren;	// these are the leafs in the correct order that would be reached if you traveled down to the leaf level
	CPtrArray aLeafs;// *Leaf_t

	u32 nTris;// total tris contained in the leafs of this node
	
	u32 nNumUniqueMats;// how many unique materials are there
	CDWordArray aMatIDs;// list of nNumUniqueMats material IDs from the leaf's MatTracker.nMatID
} Node_t;

typedef struct {
	u32 nLevelNum;	// How deep are these nodes/leafs in the hierarchy (0=root)
	BOOL bLeafLevel;// TRUE only if this is the leaf level, FALSE otherwise
	u16 nNodeCount;// how many nodes pointers are contained in aNodes, some can be NULL
	u16 nUsedNodes;// how many nodes are non NULL
	CPtrArray aNodes;// contains Leaf_t * if bLeafLevel, otherwise contains Node_t *
} Level_t;

#endif

