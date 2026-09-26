//////////////////////////////////////////////////////////////////////////////////////
// KongToWorldFile.h - 
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
// 01/15/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _KONG_TO_WORLD_FILE_H_
#define _KONG_TO_WORLD_FILE_H_ 1

#include "fang.h"

#if !FANG_USE_PORTAL

#include "fworld.h"
#include "ApeToKongFormat.h"
#include "WorldDef.h"
#include "fbitstream.h"
#include "KongToWorldInitFile.h"

class CKongToWorldFile : public CApeToKongFormat
{
public:
	CKongToWorldFile();
	~CKongToWorldFile();

	u32 GetSizeOfConvertedFile();
	BOOL WriteWorldFile( cchar *pszFilename, FILE *pFileStream=NULL );
	
	BOOL m_bK2WConverted;		// TRUE indicates the this class contains successfully converted kong->world data
	BOOL m_bPointersValid;		// TRUE indicates that you can use the pointers contained in m_pWorld
								// FALSE means that you need to call, 

	u32 m_nStatsNonCrossingTris;// how many tris did not cross quad boundaries
	u32 m_nStatsCrossingTris;	// how many tris did cross quad boundaries
	u32 m_nStatsUsedLeafs;		// how many leafs contain tris after assigning all tris
	u32 m_nStatsUsedNodes;		// how many nodes are used (nodes + leafs)
	f32 m_fStatsAvgTrisPerLeaf;	// what is the average # of tris per used leaf node

	u32 m_nStatsDisplayListBytes;// how many bytes are in the display list
	u32 m_nStatsBytesInFile;	// how many bytes will be written to disk (just the world part)
	u32 m_nStatsNodeBytes;		// how many bytes are used by node data

	FWorld_t *m_pWorld;			// the memory image to be written out to disk
								// when written to disk and 8 byte header will be added
								// and pointers will be converted to offsets

	CKongToWorldInitFile m_InitFile;

	u32 m_nQLevel;
	Level_t m_aLevels[FWORLD_MAX_QUADTREE_LEVEL_COUNT];// a header for each level (the first m_nQLevel+1 are used)
	f32 m_fRootDimension;
	f32 m_fLeafDimension;
	BOOL IsMaterialADuplicate( Node_t *pNode, u32 nLeafIndex, u32 nMatIndex );// returns TRUE if the material at nLeafIndex, nMatIndex appears earlier in the leaf chain
	s32 ConvertLeafPtrToLeafArrayIndex( Leaf_t *pLeaf );// takes a leaf pointer and figures out what index into the level node array it represents
	void ConvertPointersToOffsets();
	void ConvertOffsetsToPointers();
	void BuildEdgeDataPerMaterial();
	void ResetLeafTriListAddedAndDist2Vars( Leaf_t *pLeaf );

protected:
	virtual void FreeData();
	
private:
	u32 m_anCellsPerSide[FWORLD_MAX_QUADTREE_LEVEL_COUNT];// this array contains the number of cell per side at a given quad level
	u32 m_anNumCells[FWORLD_MAX_QUADTREE_LEVEL_COUNT];// this array contains the number of cells at a given quad level (m_anCellsPerSide^2)
	u32 m_anTotalCellCount[FWORLD_MAX_QUADTREE_LEVEL_COUNT];// this array contains the total number of cells at a given quad level
	
	f32 m_fOOLeafDim;
	f32 m_fRootY;
	CFVec3 m_RootMin;
	CFVec3 m_RootMax;
	Leaf_t *m_pLeaves;			// there are m_anNumCells[m_nQLevel] of these
	Node_t *m_pNodes;			// total node pool (there are m_anTotalCellCount[m_nQLevel-1] of these)
	u32 m_nNumMatTrackers;
	u32 m_nCurrentMatTracker;
	MatTracker_t *m_pMatTrackers;// material trakers (size = m_nNumMatTrackers elements)

	FWorldNode_t *m_pWorldNodes;// world nodes (size = m_nStatsNodeBytes)
	u8 *m_pDisplayListMem;		// display list memory (size = m_nStatsDisplayListBytes)
	
	u32 ComputeDeepestLevel( f32 &rfRootDimension, f32 &rfMinQuadSize );
	void SetLeavesToDefaults();
	void ComputeNodeCenter( CFVec3 &rCenter, u32 nLevel, u32 nRow, u32 nCol );
	void AssignKongTrisToLeaves();
	u32 GetLeafIndexFromPos( CFVec3 *pPos );
	void SortQuadIndices( u32 &r1, u32 &r2, u32 &r3 );
	f32 GetMaxXZDist2LeafToVert( const CFVec3 *pPos, const CFVec3 *pV1, const CFVec3 *pV2, const CFVec3 *pV3 );
	void OptimizeLeafBoundingInfo();
	void CreateLeafMaterialTrackingInfo();
	void InitHeirarchy();
	void FillInFamilyTree();
	void FillFWorld( FWorld_t &rWorld );
	void FillFNode( FWorldNode_t &rNode, Level_t &rLevel, u32 nNodeNum, u32 nChildAdd, u32 nParentAdd );
	void FillFNodeBox( FWorldNodeBox_t &rNodeBox, CFVec3 Min, CFVec3 Max, f32 fNodeDim );
	BOOL IsTriTooBigForBoundingBox( KongTri_t &rKTri, CFVec3 &rWSMin, CFVec3 &rWSMax );
	BOOL AreAllLeafsValid( BOOL bLogErrors );
	void SetNodeLeafArrays();
	BOOL CreateDisplayList( FWorldNode_t *pNode, CFBitStream &rDisplayList );
	void FreeAllocatedMemory();
	BOOL CreateQuadTree( BOOL bLogErrors, f32 fMinQuadSize, BOOL bComputeMinMaxPos );
};

#endif

#endif

