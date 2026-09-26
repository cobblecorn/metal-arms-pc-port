//////////////////////////////////////////////////////////////////////////////////////
// KongToWorldFile.cpp - 
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
#include "stdafx.h"
#include "fang.h"

#if !FANG_USE_PORTAL

#include "KongToWorldFile.h"
#include "fbitstream.h"
#include "ErrorLog.h"
#include "utils.h"
#include "DisplayList.h"
#include "fclib.h"

#define _ERROR_HEADING				"KONG->WORLD FILE COMPILER "
#define _NO_INDEX					0xFFFF
#define _MAX_LEAF_OVERFLOW			0.9f	// tris can go +- this number * the leaf size, otherwise there will be an error
#define _BOUNDING_BOX_MULTIPLIER	( 128.0f )
#define _BOUNDING_BOX_FUDGE_FACTOR	0.01f	// this is added/subtracted from the max/min of each leaf to ensure that no errors
											// allow tris to be larger than their leaf's bounding box
#define _MIN_AUTO_LEAF_SIZE			20.0f
#define _MAX_AUTO_LEAF_SIZE			500.0f

CKongToWorldFile::CKongToWorldFile() {
	u32 i;

	m_bK2WConverted = FALSE;
	m_bPointersValid = FALSE;

	// fill in the cell count arrays
	m_anCellsPerSide[0] = 1;
	m_anNumCells[0] = 1;
	m_anTotalCellCount[0] = 1;
	m_aLevels[0].aNodes.RemoveAll();
	for( i=1; i < FWORLD_MAX_QUADTREE_LEVEL_COUNT; i++ ) {
		m_anCellsPerSide[i] = m_anCellsPerSide[i-1] * 2;
		m_anNumCells[i] = m_anCellsPerSide[i] * m_anCellsPerSide[i];
		m_anTotalCellCount[i] = m_anNumCells[i] + m_anTotalCellCount[i-1];
		m_aLevels[i].aNodes.RemoveAll();
	}
	m_fRootDimension = 0.0f;
	m_fLeafDimension = 0.0f;
	m_fOOLeafDim = 0.0f;
	m_fRootY = 0.0f;
	m_RootMin.Zero();
	m_RootMax.Zero();
	m_nQLevel = 0;
	m_pLeaves = NULL;
	m_nStatsDisplayListBytes = 0;
	m_pNodes = NULL;
	m_nStatsNonCrossingTris = 0;
	m_nStatsCrossingTris = 0;
	m_nStatsUsedLeafs = 0;
	m_nStatsUsedNodes = 0;
	m_nStatsBytesInFile = 0;
	m_pWorld = NULL;
	m_fStatsAvgTrisPerLeaf = 0.0f;
	m_nStatsNodeBytes = 0;
	m_pWorldNodes = NULL;
	m_pDisplayListMem = NULL;
	m_nNumMatTrackers = 0;
	m_nCurrentMatTracker = 0;
	m_pMatTrackers = NULL;
}

CKongToWorldFile::~CKongToWorldFile() {
	FreeData();
}

u32 CKongToWorldFile::GetSizeOfConvertedFile() {
	if( !m_bK2WConverted ) {
		return 0;
	}
	u32 nSize;
	nSize = sizeof( FData_WorldDataHeader_t );
	nSize += m_nStatsBytesInFile;// world data + fixups

	return nSize;
}

BOOL CKongToWorldFile::WriteWorldFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {
	
	if( !m_bK2WConverted ) {
		return FALSE;
	}
	// open our file, if pFileStream == NULL
	BOOL bCloseFile = FALSE;
	if( !pFileStream ) {
		if( !pszFilename ) {
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if( !pFileStream ) {
			return FALSE;
		}
		bCloseFile = TRUE;	
	}

	///////////////////////////////
	// writeout a world data header
	FData_WorldDataHeader_t WHeader;
	fang_MemZero( &WHeader, sizeof( FData_WorldDataHeader_t ) );
	WHeader.nOffsetToFixupTable = m_nStatsBytesInFile;
	WHeader.nNumFixups = 0;
	
	fwrite( &WHeader, sizeof( FData_WorldDataHeader_t ), 1, pFileStream );

	//////////////////////////////////////////////////////
	// convert pointers to offsets from the the world head
	ConvertPointersToOffsets();

	/////////////////////////////
	// write out the entire world
	fwrite( m_pWorld, m_nStatsBytesInFile, 1, pFileStream );
	
	// close file, if need be
	if( bCloseFile ) {
		fclose( pFileStream );
	}
	return TRUE;
}

void CKongToWorldFile::FreeData() {
	m_bK2WConverted = FALSE;
	m_bPointersValid = FALSE;
	FreeAllocatedMemory();
	m_pWorldNodes = NULL;
	m_pDisplayListMem = NULL;
	m_fRootDimension = 0.0f;
	m_fLeafDimension = 0.0f;
	m_fOOLeafDim = 0.0f;
	m_fRootY = 0.0f;
	m_RootMin.Zero();
	m_RootMax.Zero();
	m_nQLevel = 0;
	m_nStatsDisplayListBytes = 0;
	m_nStatsNonCrossingTris = 0;
	m_nStatsCrossingTris = 0;
	m_nStatsUsedLeafs = 0;
	m_nStatsUsedNodes = 0;
	m_nStatsBytesInFile = 0;
	m_fStatsAvgTrisPerLeaf = 0.0f;
	m_nNumMatTrackers = 0;
	m_nCurrentMatTracker = 0;
	// call the base class's implemenation too
	CApeToKongFormat::FreeData();

	m_InitFile.FreeData();
}

// returns a number from 0-(FWORLD_MAX_QUADTREE_LEVEL_COUNT-1)
// finds best quad level while enforcing fMinQuadSize,
// this function will most likely adjust the root dimension, but
// may have to adjust fMinQuadSize if the deepest quad level still
// won't allow cells of fMinQuadSize.
u32 CKongToWorldFile::ComputeDeepestLevel( f32 &rfRootDimension, f32 &rfMinQuadSize ) {
	if( rfRootDimension <= rfMinQuadSize ) {
		// just 1 level will do it
		rfRootDimension = rfMinQuadSize;
		return 0;
	}
	// find the quad level that will allow our cell size to be rfMinQuadSize
	f32 fOOCellsPerSide;
	f32 fCellSize;
	for( u32 nLevel=1; nLevel < FWORLD_MAX_QUADTREE_LEVEL_COUNT; nLevel++ ) {
		fOOCellsPerSide = (f32)( 1.0f / m_anCellsPerSide[nLevel] );
		fCellSize = fOOCellsPerSide * rfRootDimension;
		if( fCellSize <= rfMinQuadSize ) {
			rfRootDimension = m_anCellsPerSide[nLevel] * rfMinQuadSize;
			return nLevel;
		}
	}
	// we will have to adjust our min quad size
	nLevel = (FWORLD_MAX_QUADTREE_LEVEL_COUNT-1);
	fOOCellsPerSide = (f32)( 1.0f / m_anCellsPerSide[nLevel] );
	rfMinQuadSize = rfRootDimension * fOOCellsPerSide;

	return nLevel;
}

void CKongToWorldFile::SetLeavesToDefaults() {

	f32 fHalfLeafDim = m_fLeafDimension * 0.5f;
	CFVec3 LeafMin( -fHalfLeafDim, 0.0f, -fHalfLeafDim );
	CFVec3 LeafMax( fHalfLeafDim, 0.0f, fHalfLeafDim );
	u32 nRow, nCol, nIndex;
	for( nRow=0; nRow < m_anCellsPerSide[m_nQLevel]; nRow++ ) {
		for( nCol=0; nCol < m_anCellsPerSide[m_nQLevel]; nCol++ ) {
			nIndex = (nRow * m_anCellsPerSide[m_nQLevel]) + nCol;
			ComputeNodeCenter( m_pLeaves[nIndex].Center, m_nQLevel, nRow, nCol );
			m_pLeaves[nIndex].Max = LeafMax;
			m_pLeaves[nIndex].Min = LeafMin;
			m_pLeaves[nIndex].nTris = 0;
			m_pLeaves[nIndex].aTris.RemoveAll();
			m_pLeaves[nIndex].nRow = (u8)nRow;
			m_pLeaves[nIndex].nCol = (u8)nCol;
			m_pLeaves[nIndex].nParentIndex = _NO_INDEX;// have not been assigned yet
			m_pLeaves[nIndex].nMats = 0;
			m_pLeaves[nIndex].aMats.RemoveAll();
			m_pLeaves[nIndex].pUserData = NULL;// not used by the world system
		}
	}
}

void CKongToWorldFile::ComputeNodeCenter( CFVec3 &rCenter, u32 nLevel, u32 nRow, u32 nCol ) {
	
	f32 fNodeDim = m_fRootDimension / m_anCellsPerSide[nLevel];
	f32 fHalfDim = fNodeDim * 0.5f;
	// init rCenter to the center of the 0,0th node at nLevel
	rCenter.Set( m_RootMin.x + fHalfDim, m_fRootY, m_RootMin.z + fHalfDim );
	// now move rCenter to nRow, nCol
	rCenter.x += (nCol * fNodeDim);
	rCenter.z += (nRow * fNodeDim);
}

void CKongToWorldFile::AssignKongTrisToLeaves() {
	u32 i, j, k, anQuad[3];
	CFVec3 *pP1, *pP2, *pP3;
	Leaf_t *pLeaf;
	KongSeg_t *pKSeg;
	KongMat_t *pKMat;
	KongTri_t *pKTri;
	u32 nIndex, nBestQuadIndex, nCol, nDeltaCol;
	f32 fLeastD2, fDist2;
		
	// reset our stats
	m_nStatsNonCrossingTris = 0;
	m_nStatsCrossingTris = 0;

	// walk the kong segments, materials, and then tris
	for( i=0; i < m_pKongMesh->nNumSegs; i++ ) {
		pKSeg = (KongSeg_t *)(*m_pKongMesh->apSegs)[i];
		for( j=0; j < pKSeg->nNumMats; j++ ) {
			pKMat = (KongMat_t *)(*pKSeg->apMats)[j];
			for( k=0; k < pKMat->nNumTris; k++ ) {
				pKTri = (KongTri_t *)(*pKMat->apTris)[k];
				
				pP1 = &pKTri->paApeVerts[0]->Pos;
				pP2 = &pKTri->paApeVerts[1]->Pos;
				pP3 = &pKTri->paApeVerts[2]->Pos;

				anQuad[0] = GetLeafIndexFromPos( pP1 );
				anQuad[1] = GetLeafIndexFromPos( pP2 );
				anQuad[2] = GetLeafIndexFromPos( pP3 );

				if( (anQuad[0] == anQuad[1]) && (anQuad[1] == anQuad[2]) ) {
					// all points are in the same quad, assign to this quad
					pLeaf = &m_pLeaves[ anQuad[0] ];
					pLeaf->nTris++;
					pLeaf->aTris.Add( pKTri );
					m_nStatsNonCrossingTris++;// update our stats
				} else {
					// this tri crosses quads, find the best one to put it in
					SortQuadIndices( anQuad[0], anQuad[1], anQuad[2] );
					
					// find out which quads need to be checked
					nDeltaCol = m_pLeaves[ anQuad[2] ].nCol - m_pLeaves[ anQuad[0] ].nCol;
					// search all quads looking for the best quad to put this tri in
					nIndex = anQuad[0];
					fLeastD2 = -1.0f;
					nBestQuadIndex = 0;
					nCol = 0;
					while( nIndex <= anQuad[2] ) {
						pLeaf = &m_pLeaves[nIndex];
						fDist2 = GetMaxXZDist2LeafToVert( &pLeaf->Center, pP1, pP2, pP3 );
						if( fDist2 < fLeastD2 ) {
							// pLeaf is a better choice than our previous one
							fLeastD2 = fDist2;
							nBestQuadIndex = nIndex;
						} else if( fLeastD2 < 0.0f ) {
							// we haven't set an initial best quad, do so now
							fLeastD2 = fDist2;
							nBestQuadIndex = nIndex;
						}
						// update our nIndex
						nIndex++;
						nCol++;
						if( nCol > nDeltaCol ) {
							nCol = 0;
							nIndex += m_anCellsPerSide[m_nQLevel];
							nIndex -= (nDeltaCol+1);
						}							
					}
					// we have found the best quad, put the tri into it
					pLeaf = &m_pLeaves[nBestQuadIndex];
					pLeaf->nTris++;
					pLeaf->aTris.Add( pKTri );
					m_nStatsCrossingTris++;// update our stats
				}
			}
		}
	}	
}

// map the xz portion of pPos into our leaf grid
u32 CKongToWorldFile::GetLeafIndexFromPos( CFVec3 *pPos ) {
	// transform pPos into grid space (with the root min as the origin)
	CFVec3 GridPos( *pPos );
	GridPos -= m_RootMin;
	// assign to Col
	u32 nCol = (u32)( GridPos.x * m_fOOLeafDim );
	FMATH_CLAMPMAX( nCol, m_anCellsPerSide[m_nQLevel]-1 );// assures that on the edge verts are handled
	// assign to Row
	u32 nRow = (u32)( GridPos.z * m_fOOLeafDim );
	FMATH_CLAMPMAX( nRow, m_anCellsPerSide[m_nQLevel]-1 );// assures that on the edge verts are handled
	// compute index
	u32 nIndex = (nRow * m_anCellsPerSide[m_nQLevel]) + nCol;
	FASSERT( nIndex < m_anNumCells[m_nQLevel] );
	
	return nIndex;
}

// sorts r1, r2, r3 from least to greatest
void CKongToWorldFile::SortQuadIndices( u32 &r1, u32 &r2, u32 &r3 ) {
	u32 nTemp;

	if( r2 < r1 ) {
		// swap 1 and 2
		nTemp = r1;
		r1 = r2;
		r2 = nTemp;
	}
	if( r3 < r2 ) {
		// swap 2 and 3
		nTemp = r2;
		r2 = r3;
		r3 = nTemp;
	}
	if( r2 < r1 ) {
		// swap 1 and 2
		nTemp = r1;
		r1 = r2;
		r2 = nTemp;
	}	
}

// returns the max distance^2 from pPos to pV1, pV2, or pV3.
f32 CKongToWorldFile::GetMaxXZDist2LeafToVert( const CFVec3 *pPos, const CFVec3 *pV1,
											   const CFVec3 *pV2, const CFVec3 *pV3 ) {
	CFVec3 Delta;
	f32 fMaxDist2, fDist2;

	// set the dist2 to the 1st vert as our max dist2
	Delta = *pPos - *pV1;
	fMaxDist2 = Delta.MagXZ2();
	// see if the dist2 to the 2nd vert is greater than MaxDist2
	Delta = *pPos - *pV2;
	fDist2 = Delta.MagXZ2();
	if( fDist2 > fMaxDist2 ) {
		fMaxDist2 = fDist2;
	}
	// see if the dist2 to the 3rd vert is greater than MaxDist2
	Delta = *pPos - *pV3;
	fDist2 = Delta.MagXZ2();
	if( fDist2 > fMaxDist2 ) {
		fMaxDist2 = fDist2;
	}
	return fMaxDist2;
}

void CKongToWorldFile::OptimizeLeafBoundingInfo() {
	u32 i, j;
	Leaf_t *pLeaf;
	CFVec3 Min, Max;

	// count the number of used leafs
	m_nStatsUsedLeafs = 0;

	for( i=0; i < m_anNumCells[m_nQLevel]; i++ ) {
		pLeaf = &m_pLeaves[i];
		if( pLeaf->nTris ) {
			m_nStatsUsedLeafs++;// update our stats
			// find this leaf's real bounding box
			for( j=0; j < pLeaf->nTris; j++ ) {
				FindMinAndMaxPosFromKongTris( Min, Max, (KongTri_t *)pLeaf->aTris[j], 1, (j!=0) ? TRUE : FALSE );
			}
			// add a fudge factor to the min/max
			Min.x -= _BOUNDING_BOX_FUDGE_FACTOR;
			Min.y -= _BOUNDING_BOX_FUDGE_FACTOR;
			Min.z -= _BOUNDING_BOX_FUDGE_FACTOR;
			Max.x += _BOUNDING_BOX_FUDGE_FACTOR;
			Max.y += _BOUNDING_BOX_FUDGE_FACTOR;
			Max.z += _BOUNDING_BOX_FUDGE_FACTOR;
			// put the min/max in terms of the leaf's center
			pLeaf->Min = Min - pLeaf->Center;
			pLeaf->Max = Max - pLeaf->Center;
		}
	}
}

void CKongToWorldFile::CreateLeafMaterialTrackingInfo() {
	u32 i, j;
	Leaf_t *pLeaf;
	KongTri_t *pKTri;
	ApeMaterial_t *pLastMat;
	MatTracker_t *pMatTracker;
		
	for( i=0; i < m_anNumCells[m_nQLevel]; i++ ) {
		pLeaf = &m_pLeaves[i];
		if( pLeaf->nTris ) {
			pLastMat = NULL;		
			for( j=0; j < pLeaf->nTris; j++ ) {
				pKTri = (KongTri_t *)pLeaf->aTris[j];
				if( pLastMat != pKTri->pApeMat ) {
					// the material changed, add a material tracker to this leaf
					pMatTracker = &m_pMatTrackers[m_nCurrentMatTracker++];
					FASSERT( m_nCurrentMatTracker <= m_nNumMatTrackers );
					pMatTracker->nMatID = (u32)pKTri->pApeMat;
					pMatTracker->nNumTris = 1;
					pMatTracker->nTriStartIndex = j;
					pLeaf->nMats++;
					pLeaf->aMats.Add( pMatTracker );
					// cache this material's pointer
					pLastMat = pKTri->pApeMat;
				} else {
					// same material as last time, just increase our tri count
					pMatTracker->nNumTris++;
				}
			}
		}
	}
#if 0
	// sort each leaf's Mat Tracker by the number of tris it contains
	MatTracker_t *pMatTracker2;
	u32 k;
	for( i=0; i < m_anNumCells[m_nQLevel]; i++ ) {
		pLeaf = &m_pLeaves[i];
		if( pLeaf->nTris && pLeaf->nMats > 1 ) {
			// sort
			for( j=0; j < pLeaf->nMats-1; j++ ) {
				pMatTracker = (MatTracker_t *)pLeaf->aMats[j];
				for( k=j+1; k < pLeaf->nMats; k++ ) {
					pMatTracker2 = (MatTracker_t *)pLeaf->aMats[k];	
					if( pMatTracker2->nNumTris > pMatTracker->nNumTris ) {
						// swap
						pLeaf->aMats[j] = pMatTracker2;
						pLeaf->aMats[k] = pMatTracker;
					}
				}
			}
		}
	}
#endif
}

void CKongToWorldFile::InitHeirarchy() {
	u32 nLevel, nCell;

	for( nLevel=0; nLevel <= m_nQLevel; nLevel++ ) {
		if( nLevel == m_nQLevel ) {
			// this is the leaf level
			m_aLevels[nLevel].bLeafLevel = TRUE;
			m_aLevels[nLevel].nLevelNum = nLevel;
			m_aLevels[nLevel].nNodeCount = (u16)m_anNumCells[nLevel];
			m_aLevels[nLevel].aNodes.SetSize( m_anNumCells[nLevel] );
			m_aLevels[nLevel].nUsedNodes = 0;
			for( nCell=0; nCell < m_anNumCells[nLevel]; nCell++ ) {
				if( m_pLeaves[nCell].nTris ) {
					m_aLevels[nLevel].aNodes[nCell] = &m_pLeaves[nCell];
					m_aLevels[nLevel].nUsedNodes++;
				} else {
					m_aLevels[nLevel].aNodes[nCell] = NULL;
				}
			}
		} else {
			// this is a parent node level
			m_aLevels[nLevel].bLeafLevel = FALSE;
			m_aLevels[nLevel].nLevelNum = nLevel;
			m_aLevels[nLevel].nNodeCount = (u16)m_anNumCells[nLevel];
			m_aLevels[nLevel].aNodes.SetSize( m_anNumCells[nLevel] );
			m_aLevels[nLevel].nUsedNodes = 0;
			for( nCell=0; nCell < m_anNumCells[nLevel]; nCell++ ) {
				m_aLevels[nLevel].aNodes[nCell] = NULL;
			}
		}
	}
}

void CKongToWorldFile::FillInFamilyTree() {
	u32 nLevel, nNode, nParentIndex, nParentLevel, nCurNodeIndex, nRow, nCol, nChildIndex, i, nChildLevel;
	Leaf_t *pLeaf;
	Node_t *pParent, *pChild;
	void *pPtr;
	CFVec3 MinWS, MaxWS;

	if( m_nQLevel == 0 ) {
		// no family tree, just the root
		return;
	}
	// FIRST PASS - START AT LEAFS AND CREATE PARENT NODES
	// walk the levels starting at the leaf level working toward the root
	nCurNodeIndex = 0;
	for( nLevel = m_nQLevel; nLevel > 0; nLevel-- ) {
		nParentLevel = nLevel - 1;
		nChildIndex = 0;
		// walk all nodes at nLevel
		for( nNode=0; nNode < m_aLevels[nLevel].nNodeCount; nNode++ ) {
			pPtr = m_aLevels[nLevel].aNodes[nNode];
			if( pPtr ) {
				if( m_aLevels[nLevel].bLeafLevel ) {
					// cast pPtr to a Leaf_t *
					pLeaf = (Leaf_t *)pPtr;
					// determine what our parent's row col would be
					nRow = (pLeaf->nRow >> 1);
					nCol = (pLeaf->nCol >> 1);
					// compute our world space min/max, just in case we have to add a parent
					MinWS = pLeaf->Center + pLeaf->Min;
					MaxWS = pLeaf->Center + pLeaf->Max;
				} else {
					// cast pPtr to a Node_t *
					pChild = (Node_t *)pPtr;
					// determine what our parent's row col would be
					nRow = (pChild->nNodeRow >> 1);
					nCol = (pChild->nNodeCol >> 1);
					// compute our world space min/max, just in case we have to add a parent
					MinWS = pChild->Min;
					MaxWS = pChild->Max;
				}
				nParentIndex = (nRow * m_anCellsPerSide[nParentLevel]) + nCol;
				// grab our parent
				pParent = (Node_t *)m_aLevels[nParentLevel].aNodes[nParentIndex];
				if( !pParent ) {
					// no parent has been added at this location, add one
					pParent = &m_pNodes[nCurNodeIndex++];
					m_aLevels[nParentLevel].aNodes[nParentIndex] = pParent;
					// fill in the parent's struct
					pParent->nNodeCol = (u16)nCol;
					pParent->nNodeRow = (u16)nRow;

					ComputeNodeCenter( pParent->Center, nParentLevel, nRow, nCol );
					pParent->Max = MaxWS;// keep in world coordinates for now, we will convert them in terms of center later
					pParent->Min = MinWS;// keep in world coordinates for now, we will convert them in terms of center later
					pParent->nParentIndex = _NO_INDEX;// for now, we will fix this on a 2nd pass
					pParent->nNumChildren = 1;
					pParent->anChildIndex[0] = (u16)nChildIndex;
					pParent->anChildIndex[1] = _NO_INDEX;
					pParent->anChildIndex[2] = _NO_INDEX;
					pParent->anChildIndex[3] = _NO_INDEX;

					pParent->nNumLeafChildren = 0;// we'll fix these later
					pParent->aLeafs.RemoveAll();// we'll fix these later
					pParent->nTris = 0;// we'll fix these later
					
					m_aLevels[nParentLevel].nUsedNodes++;
				} else {
					// the parent has already been added, just add the child
					pParent->anChildIndex[ pParent->nNumChildren ] = (u16)nChildIndex;
					pParent->nNumChildren++;
					// update our min, max while we are here
					pParent->Min.x = FMATH_MIN( MinWS.x, pParent->Min.x );
					pParent->Min.y = FMATH_MIN( MinWS.y, pParent->Min.y );
					pParent->Min.z = FMATH_MIN( MinWS.z, pParent->Min.z );
					
					pParent->Max.x = FMATH_MAX( MaxWS.x, pParent->Max.x );
					pParent->Max.y = FMATH_MAX( MaxWS.y, pParent->Max.y );
					pParent->Max.z = FMATH_MAX( MaxWS.z, pParent->Max.z );
				}
				nChildIndex++;
			}
		}
		// compress the nodes at nLevel so that their are no gaps. 
		// this makes the parent's child indices correct indices into the array
		for( nNode=0; nNode < m_aLevels[nLevel].nUsedNodes; nNode++ ) {
			if( !m_aLevels[nLevel].aNodes[nNode] ) {
				// this slot is empty and it shouldn't be, find the next used slot and swap with it
				for( i=nNode+1; i < m_aLevels[nLevel].nNodeCount; i++ ) {
					pPtr = m_aLevels[nLevel].aNodes[i];
					if( pPtr ) {
						// we found a used slot, swap the nNode'th and i'th slot
						m_aLevels[nLevel].aNodes[nNode] = pPtr;
						m_aLevels[nLevel].aNodes[i] = NULL;
						break;
					}
				}
			}
		}
	}
	// SECOND PASS = START AT ROOT LEVEL AND ASSIGN PARENT 
	nParentIndex = 0;
	m_nStatsUsedNodes = 0;
	for( nLevel = 0; nLevel < m_nQLevel; nLevel++ ) {
		nChildLevel = nLevel+1;
		m_nStatsUsedNodes += m_aLevels[nLevel].nUsedNodes;
		// walk all of the used nodes at nLevel
		for( nNode=0; nNode < m_aLevels[nLevel].nUsedNodes; nNode++ ) {
			pParent = (Node_t *)m_aLevels[nLevel].aNodes[nNode];
			// walk this node's children, setting their parent index
			for( nChildIndex=0; nChildIndex < pParent->nNumChildren; nChildIndex++ ) {
				if( m_aLevels[nChildLevel].bLeafLevel ) {
					pLeaf = (Leaf_t *)m_aLevels[nChildLevel].aNodes[ pParent->anChildIndex[nChildIndex] ];
					pLeaf->nParentIndex = (u16)nNode;//nParentIndex;
				} else {
					pChild = (Node_t *)m_aLevels[nChildLevel].aNodes[ pParent->anChildIndex[nChildIndex] ];
					pChild->nParentIndex = (u16)nNode;//nParentIndex;
				}
				// fix our child indices so that they reference one big array
			//	pParent->anChildIndex[nChildIndex] += m_nStatsUsedNodes;
			}
			// advance our parent index
			nParentIndex++;
			// since we are here, convert pParent's min/max back into terms of its center
			pParent->Max -= pParent->Center;
			pParent->Min -= pParent->Center;
		}
	}
	m_nStatsUsedNodes += m_nStatsUsedLeafs;// finally add our leafs to our used node count
}

void CKongToWorldFile::FillFWorld( FWorld_t &rWorld ) {
	ApeMesh_t *pApeMesh = GetMeshInfo();

	rWorld.nMeshID = utils_ComputeUniqueNumFromString( pApeMesh->szMeshName );
	rWorld.fTerrainBoxBaseY_WS = m_fRootY;
	rWorld.fNodeBoxMinY_WS = m_RootMin.y;
	rWorld.fNodeBoxMaxY_WS = m_RootMax.y;
	rWorld.fRootDim_WS = m_fRootDimension;
	rWorld.RootNodeCornerMin_WS.Set( m_RootMin.x, m_RootMin.z );
	rWorld.pRootNode = m_pWorldNodes;
	rWorld.nLevelCount = (u8)( m_nQLevel + 1 );
	rWorld.nLeafLevelNum = (u8)m_nQLevel;
	rWorld.nNodeCount = m_nStatsUsedNodes;
	rWorld.pNodeArray = m_pWorldNodes;
	rWorld.fInflationDelta = 0.0f;
	// fog
	if( pApeMesh->nNumFogs ) {
		ApeFog_t *pApeFog = GetFogInfo( 0 );
		rWorld.fFogStartZ = pApeFog->fStartDist;
		rWorld.fFogEndZ = pApeFog->fEndDist;
		rWorld.FogMotif.ColorRGB = pApeFog->Color;
		rWorld.FogMotif.fAlpha = 1.0f;
		rWorld.FogMotif.nMotifIndex = pApeFog->nMotifID;
	} else {
		rWorld.fFogStartZ = 0.0f;
		rWorld.fFogEndZ = 0.0f;
		rWorld.FogMotif.ColorRGB.Zero();
		rWorld.FogMotif.fAlpha = 0.0f;
		rWorld.FogMotif.nMotifIndex = 0;
	}
}

void CKongToWorldFile::FillFNode( FWorldNode_t &rNode, Level_t &rLevel, u32 nNodeNum, u32 nChildAdd, u32 nParentAdd ) {
	
	rNode.pNodeInfo = NULL;
	rNode.nCrossesPlanesMask = 0;
	rNode.nNodeLevel = (u8)rLevel.nLevelNum;
	rNode.hDrawNode = (void *)-1;
	//rNode.anTotalTrackerCount[] already = 0

	if( rLevel.bLeafLevel ) {
		Leaf_t *pLeaf = (Leaf_t *)rLevel.aNodes[nNodeNum];
		rNode.nNodeCol = (u8)pLeaf->nCol;
		rNode.nNodeRow = (u8)pLeaf->nRow;
		rNode.nParentNodeIndex = (u16)( pLeaf->nParentIndex + nParentAdd );
		FillFNodeBox( rNode.TerrainBox, 
					  pLeaf->Min, pLeaf->Max,
					  m_fRootDimension / m_anCellsPerSide[rLevel.nLevelNum] );
		rNode.pnDisplayListBitStream = m_pDisplayListMem;
		rNode.pLeafInfo = NULL;
		rNode.nTriCount = pLeaf->nTris;
		rNode.nMtlCount = pLeaf->nMats;
	} else {
		Node_t *pNode = (Node_t *)rLevel.aNodes[nNodeNum];
		rNode.nNodeCol = (u8)pNode->nNodeCol;
		rNode.nNodeRow = (u8)pNode->nNodeRow;
		rNode.nParentNodeIndex = (u16)( pNode->nParentIndex + nParentAdd );
		FillFNodeBox( rNode.TerrainBox, 
					  pNode->Min, pNode->Max,
					  m_fRootDimension / m_anCellsPerSide[rLevel.nLevelNum] );
		rNode.anChildNodeIndex[0] = pNode->anChildIndex[0];
		if( rNode.anChildNodeIndex[0] != _NO_INDEX ) {
			rNode.anChildNodeIndex[0] += (u16)nChildAdd;
		}
		rNode.anChildNodeIndex[1] = pNode->anChildIndex[1];
		if( rNode.anChildNodeIndex[1] != _NO_INDEX ) {
			rNode.anChildNodeIndex[1] += (u16)nChildAdd;
		}
		rNode.anChildNodeIndex[2] = pNode->anChildIndex[2];
		if( rNode.anChildNodeIndex[2] != _NO_INDEX ) {
			rNode.anChildNodeIndex[2] += (u16)nChildAdd;
		}
		rNode.anChildNodeIndex[3] = pNode->anChildIndex[3];
		if( rNode.anChildNodeIndex[3] != _NO_INDEX ) {
			rNode.anChildNodeIndex[3] += (u16)nChildAdd;
		}
		rNode.nTriCount = pNode->nTris;
		rNode.nMtlCount = pNode->nNumUniqueMats;
	}
}

void CKongToWorldFile::FillFNodeBox( FWorldNodeBox_t &rNodeBox, CFVec3 Min, CFVec3 Max, f32 fNodeDim ) {
	f32 f128OverNodeDim = _BOUNDING_BOX_MULTIPLIER / fNodeDim;
	f32 fValue;

	fValue = Min.x * f128OverNodeDim;
	FMATH_CLAMP( fValue, -127.0f, 127.0f )
	rNodeBox.nMinX = (s8)(fValue-1.0f);

	FASSERT( ((f32)rNodeBox.nMinX*(1.0f/128.0f)*fNodeDim) < Min.x );

	fValue = Min.z * f128OverNodeDim;
	FMATH_CLAMP( fValue, -127.0f, 127.0f )
	rNodeBox.nMinZ = (s8)(fValue-1.0f);

	FASSERT( ((f32)rNodeBox.nMinZ*(1.0f/128.0f)*fNodeDim) < Min.z );

	fValue = Max.x * f128OverNodeDim;
	FMATH_CLAMP( fValue, -128.0f, 126.0f )
	rNodeBox.nMaxX = (s8)(fValue+1.0f);

	FASSERT( ((f32)rNodeBox.nMaxX*(1.0f/128.0f)*fNodeDim) > Max.x );

	fValue = Max.z * f128OverNodeDim;
	FMATH_CLAMP( fValue, -128.0f, 126.0f )
	rNodeBox.nMaxZ = (s8)(fValue+1.0f);

	FASSERT( ((f32)rNodeBox.nMaxZ*(1.0f/128.0f)*fNodeDim) > Max.z );

	rNodeBox.nMinY = (s16)(Min.y-1.0f);
	rNodeBox.nMaxY = (s16)(Max.y+1.0f);	
}

BOOL CKongToWorldFile::IsTriTooBigForBoundingBox( KongTri_t &rKTri, CFVec3 &rWSMin, CFVec3 &rWSMax ) {
	u32 i;

	for( i=0; i < 3; i++ ) {
		// test the mins
		if( rKTri.paApeVerts[i]->Pos.x < rWSMin.x ) {
			return TRUE;
		}
		if( rKTri.paApeVerts[i]->Pos.z < rWSMin.z ) {
			return TRUE;
		}
		// test the maxs
		if( rKTri.paApeVerts[i]->Pos.x > rWSMax.x ) {
			return TRUE;
		}
		if( rKTri.paApeVerts[i]->Pos.z > rWSMax.z ) {
			return TRUE;
		}
	}
	return FALSE;
}

// returns TRUE if all tris in all leafs are within +- leaf dimension
// returns FALSE if any tris is too big for the current leaf dimension
// will log the center point of each invalid tri in the error log
BOOL CKongToWorldFile::AreAllLeafsValid( BOOL bLogErrors ) {
	Leaf_t *pLeaf;
	u32 i, j, nLineNum = 0;
	KongTri_t *pKTri;
	CFVec3 WSMin, WSMax;
	CString s;
	
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	m_fStatsAvgTrisPerLeaf = 0.0f;
	f32 fLimit = m_fLeafDimension * _MAX_LEAF_OVERFLOW;
	for( i=0; i < m_aLevels[m_nQLevel].nUsedNodes; i++ ) {
		pLeaf = (Leaf_t *)m_aLevels[m_nQLevel].aNodes[i];
		m_fStatsAvgTrisPerLeaf += pLeaf->nTris;
		if( pLeaf->Min.x < -fLimit ||
			pLeaf->Min.z < -fLimit ||
			pLeaf->Max.x > fLimit ||
			pLeaf->Max.z > fLimit ) {
			// this box is too big, tell the artist to scale it down
			if( bLogErrors ) {
				// since we are logging errors, find all tris that are problematic and report it to the error log
				if( !nLineNum ) {
					if( bLogErrors ) {
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
						s.Format( "Leaf size = %f", m_fLeafDimension );
						rErrorLog.WriteErrorLine( s );
					}
				}
				// compute this leaf's max/min acceptable bounding box so that we can find out what tris are too big
				WSMin = pLeaf->Center;
				WSMin.x -= fLimit;
				WSMin.z -= fLimit;
				WSMax = pLeaf->Center;
				WSMax.x += fLimit;
				WSMax.z += fLimit;
				for( j=0; j < pLeaf->nTris; j++ ) {
					pKTri = (KongTri_t *)pLeaf->aTris[j];
					if( IsTriTooBigForBoundingBox( *pKTri, WSMin, WSMax ) ) {
						s.Format( "%d) Tri is too large for leaf, center of tri (3DS Max Space) = %f %f %f", 
							++nLineNum, 
							pKTri->BoundSphere.m_Pos.x,
							pKTri->BoundSphere.m_Pos.z,
							pKTri->BoundSphere.m_Pos.y );
						rErrorLog.WriteErrorLine( s );
					}
				}
			} else {
				// since we are not logging errors and we found at least 1 leaf that didn't work, just return FALSE
				return FALSE;
			}
		}
#if 0
		// make sure that every tri is inside it's bounding box
		WSMin = pLeaf->Center;
		WSMin += pLeaf->Min;
		WSMax = pLeaf->Center;
		WSMax += pLeaf->Max;
		u32 nFoo = 0;
		for( j=0; j < pLeaf->nTris; j++ ) {
			pKTri = (KongTri_t *)pLeaf->aTris[j];
			if( IsTriTooBigForBoundingBox( *pKTri, WSMin, WSMax ) ) {
				nFoo++;
			}
		}	
#endif
	}
	m_fStatsAvgTrisPerLeaf /= (f32)m_aLevels[m_nQLevel].nUsedNodes;
	if( nLineNum ) {
		// we failed so return FALSE
		return FALSE;			
	}
	return TRUE;
}

void CKongToWorldFile::SetNodeLeafArrays() {
	s32 i;
	u32 j, k, s, nMats, nIndex, nChildLevel;
	Leaf_t *pLeaf;
	Node_t *pNode, *pChildNode;
	MatTracker_t *pMatTracker;

	// start at the 2nd to the lowest level and work up to level 0
	for( i=(m_nQLevel-1); i >= 0; i-- ) {
		nChildLevel = i+1;
		for( j=0; j < m_aLevels[i].nUsedNodes; j++ ) {
			pNode = (Node_t *)m_aLevels[i].aNodes[j];
			pNode->nNumLeafChildren = 0;
			pNode->aLeafs.RemoveAll();
			pNode->nTris = 0;
			pNode->nNumUniqueMats = 0;
			pNode->aMatIDs.RemoveAll();
			for( k=0; k < pNode->nNumChildren; k++ ) {
				if( nChildLevel == m_nQLevel ) {
					// copy leaf pointers into our leaf array
					pLeaf = (Leaf_t *)m_aLevels[nChildLevel].aNodes[ pNode->anChildIndex[k] ];
					pNode->aLeafs.Add( pLeaf );
					pNode->nNumLeafChildren++;
					pNode->nTris += pLeaf->nTris;
					// create a list of unique mat IDs, walk the leaf's mats
					for( nMats=0; nMats < pLeaf->nMats; nMats++ ) {
						pMatTracker = (MatTracker_t *)pLeaf->aMats[nMats];
						if( k != 0 ) {
							// walk the node's unique list and make sure that pMatTracker->nMatID isn't already in the list
							for( nIndex=0; nIndex < pNode->nNumUniqueMats; nIndex++ ) {
								if( pMatTracker->nMatID == pNode->aMatIDs[nIndex] ) {
									// already in the list
									break;
								}
							}
						} else {
							// we don't have to do anything for the first leaf
							nIndex = pNode->nNumUniqueMats;
						}
						if( nIndex == pNode->nNumUniqueMats ) {
							pNode->nNumUniqueMats++;
							pNode->aMatIDs.Add( pMatTracker->nMatID );
						}
					}
				} else {
					// copy our children's leaf pointer into our leaf array
					pChildNode = (Node_t *)m_aLevels[nChildLevel].aNodes[ pNode->anChildIndex[k] ];
					for( s=0; s < pChildNode->nNumLeafChildren; s++ ) {
						pLeaf = (Leaf_t *)pChildNode->aLeafs[s];
						pNode->aLeafs.Add( pLeaf );
						pNode->nNumLeafChildren++;
						pNode->nTris += pLeaf->nTris;
					}

					// create a list of unique mat IDs, walk the child's mats
					for( nMats=0; nMats < pChildNode->nNumUniqueMats; nMats++ ) {
						if( k != 0 ) {
							// walk this node's unique list and make sure that 
							// pChildNode->aMatIDs[nMats] isn't already in the list
							for( nIndex=0; nIndex < pNode->nNumUniqueMats; nIndex++ ) {
								if( pChildNode->aMatIDs[nMats] == pNode->aMatIDs[nIndex] ) {
									// already in the list
									break;
								}
							}
						} else {
							// we don't have to do anything for the first node (already compressed)
							nIndex = pNode->nNumUniqueMats;
						}
						if( nIndex == pNode->nNumUniqueMats ) {
							pNode->nNumUniqueMats++;
							pNode->aMatIDs.Add( pChildNode->aMatIDs[nMats] );
						}
					}
				}
			}
		}
	}
}

BOOL CKongToWorldFile::IsMaterialADuplicate( Node_t *pNode, u32 nLeafIndex, u32 nMatIndex ) {
	u32 nLeaf, nMatID;
	Leaf_t *pLeaf;
	MatTracker_t *pMatTrak;

	if( nLeafIndex >= pNode->nNumLeafChildren ) {
		// invalid leaf index
		return FALSE;
	}
	pLeaf = (Leaf_t *)pNode->aLeafs[nLeafIndex];
	if( nMatIndex >= pLeaf->nMats ) {
		// invalid mat index
		return FALSE;
	}
	pMatTrak = (MatTracker_t *)pLeaf->aMats[nMatIndex];
	nMatID = pMatTrak->nMatID;
	for( nLeaf=0; nLeaf < nLeafIndex; nLeaf++ ) {
		pLeaf = (Leaf_t *)pNode->aLeafs[nLeaf];
		for( nMatIndex=0; nMatIndex < pLeaf->nMats; nMatIndex++ ) {
			pMatTrak = (MatTracker_t *)pLeaf->aMats[nMatIndex];
			if( pMatTrak->nMatID == nMatID ) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

s32 CKongToWorldFile::ConvertLeafPtrToLeafArrayIndex( Leaf_t *pLeaf ) {
	
	if( !pLeaf ) {
		return -1;
	}
	u32 i;
	void *pPtr;
	for( i=0; i < m_aLevels[m_nQLevel].nUsedNodes; i++ ) {
		pPtr = m_aLevels[m_nQLevel].aNodes[i];
		if( (u32)pLeaf == (u32)pPtr ) {
			return i;
		}
	}
	return -1;
}

void CKongToWorldFile::ConvertPointersToOffsets() {
	if( !m_bK2WConverted ) {
		return;
	}
	if( !m_bPointersValid ) {
		return;
	}
	m_bPointersValid = FALSE;

	u32 nStartOffset = (u32)m_pWorld;
	m_pWorld->pRootNode = (FWorldNode_t *)UTILS_CONVERT_PTR_TO_OFFSET( m_pWorld->pRootNode, nStartOffset );
	u32 i;

	// convert the node pointer
	for( i=0; i < m_pWorld->nNodeCount; i++ ) {
		if( m_pWorld->pNodeArray[i].nNodeLevel == m_nQLevel ) {
			m_pWorld->pNodeArray[i].pnDisplayListBitStream = (u8 *)UTILS_CONVERT_PTR_TO_OFFSET( m_pWorld->pNodeArray[i].pnDisplayListBitStream, nStartOffset );
		}
	}	
	m_pWorld->pNodeArray = (FWorldNode_t *)UTILS_CONVERT_PTR_TO_OFFSET( m_pWorld->pNodeArray, nStartOffset );	
}

void CKongToWorldFile::ConvertOffsetsToPointers() {
	if( !m_bK2WConverted ) {
		return;
	}
	if( m_bPointersValid ) {
		return;
	}
	m_bPointersValid = TRUE;

	u32 nStartOffset = (u32)m_pWorld;
	m_pWorld->pRootNode = (FWorldNode_t *)UTILS_CONVERT_OFFSET_TO_PTR( m_pWorld->pRootNode, nStartOffset );
	m_pWorld->pNodeArray = (FWorldNode_t *)UTILS_CONVERT_OFFSET_TO_PTR( m_pWorld->pNodeArray, nStartOffset );
		
	u32 i;
	// convert the node offsets
	for( i=0; i < m_pWorld->nNodeCount; i++ ) {
		if( m_pWorld->pNodeArray[i].nNodeLevel == m_nQLevel ) {
			m_pWorld->pNodeArray[i].pnDisplayListBitStream = (u8 *)UTILS_CONVERT_OFFSET_TO_PTR( m_pWorld->pNodeArray[i].pnDisplayListBitStream, nStartOffset );
		}
	}	
}

void CKongToWorldFile::BuildEdgeDataPerMaterial() {
	if( !m_bK2WConverted ) {
		return;
	}
	u32 nLeafIndex, nMatIndex;
	Leaf_t *pLeaf;
	MatTracker_t *pMatTracker;

	ResetEdgeInfo();

	// for every leaf
	for( nLeafIndex=0; nLeafIndex < m_aLevels[m_nQLevel].nUsedNodes; nLeafIndex++ ) {
		pLeaf = (Leaf_t *)m_aLevels[ m_nQLevel ].aNodes[ nLeafIndex ];
		// for every mat
		for( nMatIndex=0; nMatIndex < pLeaf->nMats; nMatIndex++ ) {
			pMatTracker = (MatTracker_t *)pLeaf->aMats[ nMatIndex ];
			ComputeEdgeInfo( pLeaf->aTris, pMatTracker->nNumTris, pMatTracker->nTriStartIndex );
		}
	}
}

// runs through a leaf's tri list and resets the bTriAdded and fDist2 vars to 0.0f
void CKongToWorldFile::ResetLeafTriListAddedAndDist2Vars( Leaf_t *pLeaf ) {
	u32 i;
	KongTri_t *pKTri;

	for( i=0; i < pLeaf->nTris; i++ ) {
		pKTri = (KongTri_t *)pLeaf->aTris[i];
		pKTri->bTriAdded = FALSE;
		pKTri->fDist2 = 0.0f;
	}
}

// it is assumed that pNode has an integer display list terminated with 0xFFFF
// it is also assumed that the integer display list is sorted from lowest to highest
BOOL CKongToWorldFile::CreateDisplayList( FWorldNode_t *pNode, CFBitStream &rDisplayList ) {
	const u16 *pnIntDL;
	FWorldNode_t *pWorldNode;
	s32 nLastLevelNum = -1;
	u32 i, nDLIndex, nNodesAtCurrentLevel, nDLSize, nCountBits, nMaxItemsChunk;
	
	if( !pNode || pNode->nNodeLevel != m_pWorld->nLeafLevelNum ) {
		return FALSE;
	}
	pnIntDL = (u16 *)pNode->pnDisplayListBitStream;
	// count how many DL entries there are
	nDLSize = 0;
	while( pnIntDL[nDLSize] != 0xFFFF ) {
		nDLSize++;
	}
	if( nDLSize == 0 ) {
		return FALSE;
	}
	// find out how many DL entries there are at the pnIntDL[nDLSize-1]'s level
	FASSERT( pnIntDL[ nDLSize-1 ] < m_pWorld->nNodeCount );
	pWorldNode = &m_pWorld->pNodeArray[ pnIntDL[ nDLSize-1 ] ];
	nLastLevelNum = pWorldNode->nNodeLevel;
	nNodesAtCurrentLevel = 0;
	for( nDLIndex=0; nDLIndex < nDLSize; nDLIndex++ ) {
		pWorldNode = &m_pWorld->pNodeArray[ pnIntDL[nDLIndex] ];
		if( pWorldNode->nNodeLevel == nLastLevelNum ) {
			nNodesAtCurrentLevel++;
		}
	}
	// find out how many bits are needed in the count field
	if( nNodesAtCurrentLevel >= 64 ) {
		nCountBits = 7;
		nMaxItemsChunk = 127;
	} else if( nNodesAtCurrentLevel >= 32 ) {
		nCountBits = 6;
		nMaxItemsChunk = 63;
	} else if( nNodesAtCurrentLevel >= 16 ) {
		nCountBits = 5;
		nMaxItemsChunk = 31;
	} else if( nNodesAtCurrentLevel >= 8 ) {
		nCountBits = 4;
		nMaxItemsChunk = 15;
	} else if( nNodesAtCurrentLevel >= 4 ) {
		nCountBits = 3;
		nMaxItemsChunk = 7;
	} else if( nNodesAtCurrentLevel >= 2 ) {
		nCountBits = 2;
		nMaxItemsChunk = 3;
	} else {
		nCountBits = 1;
		nMaxItemsChunk = 1;
	}
	// write out our count field
	rDisplayList.Set( rDisplayList.GetNextBitIndex(), nCountBits, FWORLD_DLBITSTREAM_COUNT_FIELD_BITS );
		
	nDLIndex = 0;
	while( pnIntDL[nDLIndex] != 0xFFFF ) {
		pWorldNode = &m_pWorld->pNodeArray[ pnIntDL[nDLIndex] ];	
		nLastLevelNum = pWorldNode->nNodeLevel;
		// count how many nodes we have at nLastLevelNum level
		nNodesAtCurrentLevel = 0;	
		for( i=nDLIndex; i < nDLSize; i++ ) {
			FASSERT( pnIntDL[i] < m_pWorld->nNodeCount );
			if( m_pWorld->pNodeArray[ pnIntDL[i] ].nNodeLevel == nLastLevelNum ) {
				nNodesAtCurrentLevel++;
			} else {
				break;// end of the current chunk
			}
		}
		FMATH_CLAMPMAX( nNodesAtCurrentLevel, nMaxItemsChunk );
		// write out our level num
		rDisplayList.Set( rDisplayList.GetNextBitIndex(), nLastLevelNum, FWORLD_DLBITSTREAM_LEVEL_FIELD_BITS );
		// write out our count
		rDisplayList.Set( rDisplayList.GetNextBitIndex(), nNodesAtCurrentLevel, nCountBits );
		// write out each element (only needed if we are higher than level 0)
		if( nLastLevelNum > 0 ) {
			for( i=0; i < nNodesAtCurrentLevel; i++ ) {
				pWorldNode = &m_pWorld->pNodeArray[ pnIntDL[nDLIndex+i] ];
				// col first...
				rDisplayList.Set( rDisplayList.GetNextBitIndex(), pWorldNode->nNodeCol, nLastLevelNum );
				// row next...
				rDisplayList.Set( rDisplayList.GetNextBitIndex(), pWorldNode->nNodeRow, nLastLevelNum );					
			}
		}
		// advance our counter
		nDLIndex += nNodesAtCurrentLevel;
	}
	// write out a terminator	
	rDisplayList.Set( rDisplayList.GetNextBitIndex(), FWORLD_DLBITSTREAM_LEVEL_TERM_VALUE, FWORLD_DLBITSTREAM_LEVEL_FIELD_BITS );

	return TRUE;
}

void CKongToWorldFile::FreeAllocatedMemory() {
	if( m_pLeaves ) {
		delete[] (Leaf_t *)m_pLeaves;
		m_pLeaves = NULL;
	}
	if( m_pNodes ) {
		delete[] (Node_t *)m_pNodes;
		m_pNodes = NULL;
	}
	if( m_pWorld ) {
		fang_Free( m_pWorld );
		m_pWorld = NULL;
	}
	if( m_pMatTrackers ) {
		delete[] (MatTracker_t *)m_pMatTrackers;
		m_pMatTrackers = NULL;
	}
}

BOOL CKongToWorldFile::CreateQuadTree( BOOL bLogErrors, f32 fMinQuadSize, BOOL bComputeMinMaxPos ) {
	
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// find the min & max of this world
	if( bComputeMinMaxPos ) {
		FindMinAndMaxPosFromKongTris( m_RootMin, m_RootMax, m_pKongTri, m_nNumKongTris );
	}
	// make sure that the delta y is at least as high as it is wide
	f32 fDeltaY = m_RootMax.y - m_RootMin.y;
	if( fDeltaY < fMinQuadSize ) {
		f32 fAddY = (fMinQuadSize - fDeltaY) * 0.5f;
		m_RootMax.y += fAddY;
		m_RootMin.y -= fAddY;
	}
	m_fRootY = (m_RootMax.y + m_RootMin.y) * 0.5f;
	// the largest dimension becomes our root quad tree dimension
	m_fRootDimension = FMATH_MAX( (m_RootMax.x - m_RootMin.x), (m_RootMax.z - m_RootMin.z) );
	// determine the deepest level to make our quad tree
	FMATH_CLAMPMIN( fMinQuadSize, 1.0f );
	m_fLeafDimension = fMinQuadSize;
	m_nQLevel = ComputeDeepestLevel( m_fRootDimension, m_fLeafDimension );
	if( m_nQLevel < 1 ) {
		if( bLogErrors ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Make the leaf size smaller!  There must be at least 2 levels." );
		}
		return FALSE;
	}
	m_fOOLeafDim = 1.0f / m_fLeafDimension;
	// allocate memory to store all the leafs
	m_pLeaves = new Leaf_t[ m_anNumCells[m_nQLevel] ];
	if( !m_pLeaves ) {
		if( bLogErrors ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Could not allocate enough memory for the needed leafs" );
		}
		return FALSE;
	}
	// allocate memory for the material trackers
	m_nNumMatTrackers = m_anNumCells[m_nQLevel] * m_nNumKongMats;// worst case every material in every leaf node
	m_nCurrentMatTracker = 0;
	m_pMatTrackers = new MatTracker_t[m_nNumMatTrackers];
	if( !m_pMatTrackers ) {
		if( bLogErrors ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Could not allocate enough memory for the material trackers" );
		}
		return FALSE;
	}
	// set the leaves to the default size
	SetLeavesToDefaults();
	// run through the tris adding them to 1 and only 1 leaf
	AssignKongTrisToLeaves();
	// run through the leafs and optimize the bounding boxes
	OptimizeLeafBoundingInfo();
	// run through the leafs and create the material tracking info
	CreateLeafMaterialTrackingInfo();

	// now we need to setup our hierarchy, first allocate memory
	if( m_nQLevel > 0 ) {
		m_pNodes = new Node_t[ m_anTotalCellCount[m_nQLevel-1] ];
		if( !m_pNodes ) {
			if( bLogErrors ) {
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
				rErrorLog.WriteErrorLine( "Could not allocate enough memory for the needed nodes" );
			}
			return FALSE;
		}
	} else {
		m_pNodes = NULL;
	}
	// init our hierarchy
	InitHeirarchy();
	FillInFamilyTree();
	// verify that no tri in any leaf is not too big for it's max bounding box
	if( !AreAllLeafsValid( bLogErrors ) ) {
		return FALSE;
	}
	// set the node leaf arrays
	SetNodeLeafArrays();
	return TRUE;
}

#endif