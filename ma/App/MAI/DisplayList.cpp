//////////////////////////////////////////////////////////////////////////////////////
// DisplayList.cpp - 
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
#include "DisplayList.h"
#include "fversion.h"
#include "fworld.h"
#include "fmath.h"

//====================
// private definitions

#define _VERSION		( FVERSION_CREATE_VERSION( FVERSION_APE_PLATFORM, 1, 0, 0 ) )

//////////////////
// .DL FILE FORMAT
// DLHeader_t
//	DLLeaf_t []

typedef struct {
	u32 nNumBytes;	// how many bytes are there in this file
	FVersionHeader_t Version;// what file version is this
	u32 nNumLeafs;	// how many DLLeaf_t follow this structure
	f32 fSizeOfLeaf;// how large is each leaf (world space units)
	CFVec2 MinXZ;	// where is the lower left XZ corner positioned
	u32 nTotalLeafNodes;// how many leaf nodes are in the world file( this counts empty ones too )
	u32 nLeafsAcross;// how many leaf nodes across are in the world file
} DLHeader_t;

typedef struct {
	CFVec2 XZ;		// where is this leaf located
	u32 nLeafIndex;	// what is this leaf's index
	u32 nNumVisibleLeafs;// how many DLElement_t follow this structure
} DLLeaf_t;

typedef struct {
	u32 nLeafIndex;	// index of visible leaf
} DLElement_t;

//==================
// private variables

static u8 *_pnBitStreamByte;
static s32 _nBitStreamRemainingBitCount;

//===================
// private prototypes

static u32 _ReadIntFromBitStream( u32 nTotalBitsToRead );



CDisplayList::CDisplayList() {
	m_nTotalLeafs = 0;
	m_pnEveryLeafDL = NULL;
	Reset();
}

CDisplayList::~CDisplayList() {
	Reset();
}

BOOL CDisplayList::ReadFromFile( cchar *pszFilename ) {
	u32 i, j;

	Reset();
	if( !pszFilename ) {
		return FALSE;
	}
	// Open the filestream for reading
	FILE *pFileStream = _tfopen( pszFilename, _T("rb") );
	if( !pFileStream ) {
		return FALSE;
	}
	// read the header in
	DLHeader_t DLHeader;
	u32 nItemsRead = fread( &DLHeader, sizeof( DLHeader_t ), 1, pFileStream );
	if( nItemsRead != 1 ) {
		fclose( pFileStream );
		return FALSE;
	}
	// move the file back to the beginning
	fseek( pFileStream, 0, SEEK_SET );
	// verify a valid .dl file
	if( DLHeader.Version.nSignature != FVERSION_FILE_SIGNATURE || DLHeader.nNumBytes == 0 ) {
		// invalid file
		fclose( pFileStream );
		return FALSE;
	}
	if( DLHeader.Version.nVersion != _VERSION ) {
		// invalid file version
		fclose( pFileStream );
		return FALSE;
	}
	// allocate memory for the entire file
	u8 *pFileMem = new u8[DLHeader.nNumBytes];
	if( !pFileMem ) {
		fclose( pFileStream );
		return FALSE;
	}
	// read the entire file into our allocated memory
	nItemsRead = fread( pFileMem, sizeof( u8 ), DLHeader.nNumBytes, pFileStream );
	fclose( pFileStream );// file is not longer needed, so close it
	if( nItemsRead != DLHeader.nNumBytes ) {
		delete[] (u8 *)pFileMem;
		return FALSE;
	}
	DLHeader_t *pHeader = (DLHeader_t *)pFileMem;
	DLLeaf_t *pDLLeaf = (DLLeaf_t *)&pHeader[1];
	DLElement_t *pElement = (DLElement_t *)&pDLLeaf[1];
	// copy from our header and setup for the leafs
	m_nTotalLeafs = pHeader->nTotalLeafNodes;
	m_nUsedLeafs = pHeader->nNumLeafs;
	m_aLeafs.SetSize( m_nTotalLeafs );
	m_fLeafSize = pHeader->fSizeOfLeaf;
	m_nLeafsAcross = m_nLeafsUp = pHeader->nLeafsAcross;
	m_MinXZ = pHeader->MinXZ;
	m_aDL.SetSize( m_nTotalLeafs );
	// reset our arrays
	for( i=0; i < m_nTotalLeafs; i++ ) {
		m_aLeafs[i] = NULL;
		m_aDL[i] = NULL;
	}
	// create a display list of every leaf
	m_pnEveryLeafDL = new u16[m_nUsedLeafs+1];
	if( !m_pnEveryLeafDL ) {
		Reset();
		return FALSE;
	}
	u16 *pnDisplayList = m_pnEveryLeafDL;
	// set up our leafs
	CLeaf *pLeaf;
	for( i=0; i < m_nUsedLeafs; i++ ) {
		pLeaf = new CLeaf;
		if( !pLeaf ) {
			Reset();
			return FALSE;
		}
		m_aLeafs[ pDLLeaf->nLeafIndex ] = pLeaf;
		pLeaf->SetIndex( pDLLeaf->nLeafIndex );
		pLeaf->ResetDL();
		pLeaf->SetLeafXZ( pDLLeaf->XZ );
		for( j=0; j < pDLLeaf->nNumVisibleLeafs; j++ ) {
			pLeaf->AddToDL( pElement->nLeafIndex );
			pElement++;	
		}
		// add to our global display list
		*pnDisplayList = pDLLeaf->nLeafIndex;
		pnDisplayList++;
		// advance our pointers
		pDLLeaf = (DLLeaf_t *)pElement;
		pElement = (DLElement_t *)&pDLLeaf[1];
	}	
	// terminate the global display list
	*pnDisplayList = 0xFFFF;

	delete[] (u8 *)pFileMem;

	return TRUE;
}

BOOL CDisplayList::WriteToFile( cchar *pszFilename ) {
	FILE *pFileStream;
	DLHeader_t Header;
	DLLeaf_t Leaf;
	DLElement_t Element;
	u32 nNumElements, i, j;
	CLeaf *pLeaf;
	
	if( !pszFilename ) {
		return FALSE;
	}
	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return FALSE;
	}

	// open our file for writing
	pFileStream = _tfopen( pszFilename, _T("wb") );
	if( !pFileStream ) {
		return FALSE;
	}
	// count the number of elements needed
	nNumElements = 0;
	for( i=0; i < m_nTotalLeafs; i++ ) {
		pLeaf = (CLeaf *)m_aLeafs[i];
		if( pLeaf ) {
			nNumElements += pLeaf->GetDLCount();
		}
	}
	// fill in our header
	Header.nNumBytes = sizeof( DLHeader_t ) +
					   sizeof( DLLeaf_t ) * m_nUsedLeafs + 
					   sizeof( DLElement_t ) * nNumElements;
	Header.Version.nSignature = FVERSION_FILE_SIGNATURE;
	Header.Version.nVersion = _VERSION;
	Header.nNumLeafs = m_nUsedLeafs;
	Header.fSizeOfLeaf = m_fLeafSize;
	Header.MinXZ = m_MinXZ;
	Header.nTotalLeafNodes = m_nTotalLeafs;
	Header.nLeafsAcross = m_nLeafsAcross;
	fwrite( &Header, sizeof( DLHeader_t ), 1, pFileStream );
	// fill in the leafs
	for( i=0; i < m_nTotalLeafs; i++ ) {
		pLeaf = (CLeaf *)m_aLeafs[i];
		if( pLeaf ) {
			pLeaf->GetLeafXZ( Leaf.XZ );
			Leaf.nLeafIndex = pLeaf->GetIndex();
			Leaf.nNumVisibleLeafs = pLeaf->GetDLCount();
			fwrite( &Leaf, sizeof( DLLeaf_t ), 1, pFileStream );
			// fill in the elements
			for( j=0; j < Leaf.nNumVisibleLeafs; j++ ) {
				Element.nLeafIndex = pLeaf->GetDLLeafIndex( j );
				fwrite( &Element, sizeof( DLElement_t ), 1, pFileStream );
			}
		}
	}

	fclose( pFileStream );

	return TRUE;
}

BOOL CDisplayList::InitFromWorld( const FWorld_t *pWorld ) {
	u32 i, j, nNumLeafs, nIndex, nDLCount;
	FWorldNode_t *pNode;
	CLeaf *pLeaf;
	CFVec2 XZ;
	CUIntArray anDL;
	u16 *pnDisplayList;

	Reset();

	if( !pWorld ) {
		return FALSE;
	}
	// count how many leafs there are
	nNumLeafs = 1;
	m_nLeafsAcross = 1;
	for( i=0; i < pWorld->nLeafLevelNum; i++ ) {
		nNumLeafs <<= 2;
		m_nLeafsAcross <<= 1;
	}
	// setup our leaf array for nNumLeafs slots
	m_nTotalLeafs = nNumLeafs;
	m_nUsedLeafs = 0;
	m_nLeafsUp = m_nLeafsAcross;
	m_MinXZ = pWorld->RootNodeCornerMin_WS;
	m_fLeafSize = pWorld->fRootDim_WS / m_nLeafsAcross;
	m_aLeafs.SetSize( m_nTotalLeafs );
	m_aDL.SetSize( m_nTotalLeafs );
	// reset our arrays
	for( i=0; i < m_nTotalLeafs; i++ ) {
		m_aLeafs[i] = NULL;
		m_aDL[i] = NULL;
	}
	// run through the nodes in the world 
	for( i=0; i < pWorld->nNodeCount; i++ ) {
		pNode = &pWorld->pNodeArray[i];
		if( pNode->nNodeLevel == pWorld->nLeafLevelNum ) {
			nIndex = ConvertLeafColRowToIndex( pNode->nNodeCol, pNode->nNodeRow );
			pLeaf = new CLeaf;
			if( !pLeaf ) {
				Reset();
				return FALSE;
			}
			m_aLeafs[nIndex] = pLeaf;
			pLeaf->SetIndex( nIndex );
			pLeaf->ResetDL();
			XZ.Set( m_MinXZ.x + (m_fLeafSize * pNode->nNodeCol), 
					m_MinXZ.y + (m_fLeafSize * pNode->nNodeRow) );
			pLeaf->SetLeafXZ( XZ );
			// create a display list from the bit stream
			CreateDisplayList( pWorld, pNode, nDLCount, anDL );
			for( j=0; j < nDLCount; j++ ) {
				pLeaf->AddToDL( anDL[j] );
			}
			m_nUsedLeafs++;
		}
	}
	// create a display list of every leaf
	m_pnEveryLeafDL = new u16[m_nUsedLeafs+1];
	if( !m_pnEveryLeafDL ) {
		Reset();
		return FALSE;
	}
	pnDisplayList = m_pnEveryLeafDL;
	for( i=0; i < pWorld->nNodeCount; i++ ) {
		pNode = &pWorld->pNodeArray[i];
		if( pNode->nNodeLevel == pWorld->nLeafLevelNum ) {
			*pnDisplayList = i;
			pnDisplayList++;
		}
	}
	*pnDisplayList = 0xFFFF;

	return TRUE;
}

CLeaf *CDisplayList::GetLeaf( const CFVec2 &rXZ ) {
	CFVec2 Delta;
	u32 nCol, nRow;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	
	Delta = rXZ - m_MinXZ;

	// compute the column number
	if( Delta.x < 0.0f ) {
		nCol = 0xFFFF;//an invalid col, but this is handled
	} else {
		nCol = 0;
		while( Delta.x >= m_fLeafSize ) { 
			Delta.x -= m_fLeafSize;
			nCol++;
		}
	}
	// compute the row number
	if( Delta.y < 0.0f ) {
		nRow = 0xFFFF;//an invalid row, but this is handled
	} else {
		nRow = 0;
		while( Delta.y >= m_fLeafSize ) { 
			Delta.y -= m_fLeafSize;
			nRow++;
		}
	}
	
	return GetLeaf( nCol, nRow );
}

CLeaf *CDisplayList::GetLeaf( u32 nAcross, u32 nUp ) {
	
	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	if( nAcross >= m_nLeafsAcross ) {
		// invalid column
		return NULL;
	}
	if( nUp >= m_nLeafsUp ) {
		// invalid row
		return NULL;
	}
	
	u32 nLeafIndex = ConvertLeafColRowToIndex( nAcross, nUp );
	return (CLeaf *)m_aLeafs[nLeafIndex];
}

CLeaf *CDisplayList::GetLeaf( u32 nIndex ) {
	
	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	if( nIndex >= m_nTotalLeafs ) {
		// invalid index
		return NULL;
	}
	return (CLeaf *)m_aLeafs[nIndex];
}

CLeaf *CDisplayList::GetLeafWithLargestDLCount() {
	CLeaf *pBest, *pCurrent;
	u32 i, nLargestDLCount, nCurrentDLCount;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	pBest = NULL;
	for( i=0; i < m_nTotalLeafs; i++ ) {
		pCurrent = (CLeaf *)m_aLeafs[i];	
		if( pCurrent ) {
			nCurrentDLCount = pCurrent->GetDLCount();
			if( pBest ) {
				if( nLargestDLCount < nCurrentDLCount ) {
					pBest = pCurrent;
					nLargestDLCount = nCurrentDLCount;	
				}
			} else {
				pBest = pCurrent;
				nLargestDLCount = nCurrentDLCount;
			}
		}
	}
	return pBest;
}

CLeaf *CDisplayList::GetNearestValidLeaf( const CFVec2 &rXZ ) {
	CFVec2 Delta, Temp;
	u32 nCol, nRow, nClosest, i;
	f32 fDist2, fSmallestD2;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	CLeaf *pLeaf = GetLeaf( rXZ );
	if( pLeaf ) {
		return pLeaf;
	}
	// not inside the standard leaf space, find the nearest row,col
	Delta = rXZ - m_MinXZ;

	// compute the column number
	nCol = 0;
	while( Delta.x >= m_fLeafSize ) { 
		Delta.x -= m_fLeafSize;
		nCol++;
	}
	// compute the row number
	nRow = 0;
	while( Delta.y >= m_fLeafSize ) { 
		Delta.y -= m_fLeafSize;
		nRow++;
	}
	FMATH_CLAMP( nCol, 0, m_nLeafsAcross-1 );
	FMATH_CLAMP( nRow, 0, m_nLeafsUp-1 );
	
	pLeaf = GetLeaf( nCol, nRow );
	if( pLeaf ) {
		return pLeaf;
	}
	
	// the closest leaf is NULL, we need to find the closest leaf
	nClosest = 0;
	fSmallestD2 = -1.0f;
	for( i=0; i < m_nTotalLeafs; i++ ) {
		pLeaf = (CLeaf *)m_aLeafs[i];
		if( pLeaf ) {
			pLeaf->GetLeafXZ( Delta );
			Delta -= rXZ;
			fDist2 = Delta.Mag2();
			if( fSmallestD2 >= 0.0f ) {
				// see if we are smaller than fSmallestD2
				if( fDist2 < fSmallestD2 ) {
					fSmallestD2 = fDist2;
					nClosest = i;
				}
			} else {
				// this is the first valid D2
				fSmallestD2 = fDist2;
				nClosest = i;
			}
		}
	}
	return (CLeaf *)m_aLeafs[nClosest];
}

void CDisplayList::Reset() {
	u32 i;

	for( i=0; i < m_nTotalLeafs; i++ ) {
		if( m_aLeafs[i] ) {
			delete (CLeaf *)m_aLeafs[i];
			m_aLeafs[i] = NULL;
		}
		if( m_aDL[i] ) {
			delete[] (u16 *)m_aDL[i];
			m_aDL[i] = NULL;
		}
	}
	m_nTotalLeafs = 0;
	m_nUsedLeafs = 0;
	m_aLeafs.RemoveAll();
	m_fLeafSize = 0.0f;
	m_nLeafsAcross = 0;
	m_nLeafsUp = 0;
	m_MinXZ.Zero();
	m_aDL.RemoveAll();
	if( m_pnEveryLeafDL ) {
		delete[] (u16 *)m_pnEveryLeafDL;
		m_pnEveryLeafDL = NULL;
	}
}

void CDisplayList::CreateDisplayList( const FWorld_t *pWorld, const FWorldNode_t *pNode,
									  u32 &rnDLCount, CUIntArray &ranDL ) {
	u32 i, j, nCountFieldBits, nNodeFieldBits, nLevel, nNodeCount, nNodeColRow, nNodeIndex, nAcross;
	u32 nRow, nCol;
	
	// reset our DL
	rnDLCount = 0;
	ranDL.RemoveAll();
	
	// decompress the display list
	_nBitStreamRemainingBitCount = 8;
	_pnBitStreamByte = pNode->pnDisplayListBitStream;
	
	nCountFieldBits = _ReadIntFromBitStream( FWORLD_DLBITSTREAM_COUNT_FIELD_BITS );

	while( (nLevel = _ReadIntFromBitStream( FWORLD_DLBITSTREAM_LEVEL_FIELD_BITS )) != FWORLD_DLBITSTREAM_LEVEL_TERM_VALUE ) {

		nAcross = 1;
		for( i=0; i < nLevel; i++ ) {
			nAcross <<= 1;
		}
		nNodeFieldBits = nLevel << 1;
		nNodeCount = _ReadIntFromBitStream( nCountFieldBits );

		for( i=0; i < nNodeCount; i++ ) {
			nNodeColRow = _ReadIntFromBitStream( nNodeFieldBits );
			nCol = nNodeColRow >> nLevel;
			nRow = ( nNodeColRow & ( (1<<nLevel) - 1) );
			
			if( nLevel == pWorld->nLeafLevelNum ) {
				// this is a leaf index, add to our DL
				nNodeIndex = nCol + (nRow * nAcross);
				rnDLCount++;
				ranDL.Add( nNodeIndex );
			} else {
				// this is not a leaf
				for( j=0; j < pWorld->nNodeCount; j++ ) {
					if( pWorld->pNodeArray[j].nNodeLevel == nLevel &&
						pWorld->pNodeArray[j].nNodeCol == nCol && 
						pWorld->pNodeArray[j].nNodeRow == nRow ) {
						// we need to find all of this nodes leaf childern
						RecurseDownToLeaf( pWorld, &pWorld->pNodeArray[j], rnDLCount, ranDL );
						break;
					}
				}
				
			}
		}				
	}
}

void CDisplayList::RecurseDownToLeaf( const FWorld_t *pWorld, const FWorldNode_t *pNode, 
									  u32 &rnDLCount, CUIntArray &ranDL ) {
	u32 i, nNodeIndex;

	if( pNode->nNodeLevel == pWorld->nLeafLevelNum ) {
		// this is a leaf, add it to our DL and exit
		nNodeIndex = ConvertLeafColRowToIndex( pNode->nNodeCol, pNode->nNodeRow );
		rnDLCount++;
		ranDL.Add( nNodeIndex );	
		return;
	}
	// not a leaf, recurse through our childern
	for( i=0; i < 4; i++ ) {
		if( pNode->anChildNodeIndex[i] != 0xFFFF ) {
			RecurseDownToLeaf( pWorld, &pWorld->pNodeArray[ pNode->anChildNodeIndex[i] ], rnDLCount, ranDL );
		}
	}
}

// creates an integer display list
const u16 *CDisplayList::GetIntDisplayList( const FWorld_t *pWorld, u32 nAcross, u32 nUp, BOOL bOnlyContainLeafNodes/*=TRUE*/ ) {
	u32 i, j, k, nIndex, nDLCount, nRow, nCol, nDLSize;
	u16 *pnDL;
	s32 nLeafIndex;
	FWorldNode_t *pNode, *pParentNode;
	CUIntArray anChildDLIndices;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	if( !pWorld ) {
		return NULL;
	}
	CLeaf *pLeaf = GetLeaf( nAcross, nUp );
	if( !pLeaf ) {
		return NULL;
	}
	nIndex = pLeaf->GetIndex();
	if( m_aDL[nIndex] ) {
		delete[] (u16 *)m_aDL[nIndex];
		m_aDL[nIndex] = NULL;
	}
	nDLCount = pLeaf->GetDLCount();
	if( !nDLCount ) {
		return NULL;
	}
	// allocate memory for our display list (remember the terminator!)
	pnDL = new u16[nDLCount + 1];
	if( !pnDL ) {
		return NULL;
	}
	m_aDL[nIndex] = pnDL;
	// fill in our display list
	for( i=0, nDLSize=0; i < nDLCount; i++ ) {
		nLeafIndex = pLeaf->GetDLLeafIndex( i );
		if( nLeafIndex >= 0 ) {
			ConvertLeafIndexToColRow( nLeafIndex, nCol, nRow );
			// convert leaf index into world node index
			for( j=0; j < pWorld->nNodeCount; j++ ) {
				if( pWorld->pNodeArray[j].nNodeLevel == pWorld->nLeafLevelNum &&
					pWorld->pNodeArray[j].nNodeCol == nCol && 
					pWorld->pNodeArray[j].nNodeRow == nRow ) {
					*pnDL = j;
					nDLSize++;
					pnDL++;
					break;
				}
			}
		}
	}
	*pnDL = 0xFFFF;
	// compress our display list 
	if( !bOnlyContainLeafNodes ) {
		pnDL = (u16 *)m_aDL[nIndex];
		
		CUIntArray anTempDL;
		u32 nNumChildren, nSwap;
		BOOL bDeleteChildren, bKeepGoing = TRUE;
		while( bKeepGoing ) {
			bKeepGoing = FALSE;
			for( i=0; i < nDLSize; i++ ) {
				pNode = &pWorld->pNodeArray[ pnDL[i] ];
				if( pNode->nParentNodeIndex != 0xFFFF ) {
					// grab the parent node
					pParentNode = &pWorld->pNodeArray[ pNode->nParentNodeIndex ];
					// see if all of our parent's childern are in our display list
					bDeleteChildren = AreAllChildrenInDL( pnDL, pParentNode, anChildDLIndices );
					if( bDeleteChildren ) {
						// since we are removing elements, we can keep compressing
						bKeepGoing = TRUE;
						nNumChildren = anChildDLIndices.GetSize();

						// create a temp DL
						anTempDL.RemoveAll();
						for( j=0; j < nDLSize; j++ ) {
							anTempDL.Add( pnDL[j] );
						}
						// sort anChildDLIndices from greatest to smallest so that 
						// we can remove elements from the back toward the front
						if( nNumChildren >= 2 ) {
							for( j=0; j < nNumChildren-1; j++ ) {
								for( k=j+1; k < nNumChildren; k++ ) {
									if( anChildDLIndices[k] > anChildDLIndices[j] ) {
										// swap elements
										nSwap = anChildDLIndices[j];
										anChildDLIndices[j] = anChildDLIndices[k];
										anChildDLIndices[k] = nSwap;
									}
								}
							}
						}
						// remove nNumChildren elements from our display list
						for( j=0; j < nNumChildren; j++ ) {
							FASSERT( anChildDLIndices[j] < (u32)anTempDL.GetSize() );
							anTempDL.RemoveAt( anChildDLIndices[j] );
						}
						// add the parent index 
						anTempDL.Add( pNode->nParentNodeIndex );
						
						nDLSize = anTempDL.GetSize();
						
						// copy the elements from our temp DL back into our display list memory
						for( k=0; k < nDLSize; k++ ) {
							pnDL[k] = anTempDL[k];
						}
						// terminate the new list
						pnDL[nDLSize] = 0xFFFF;
					}
				}
			}
		}
		// sort the final display list from smallest to largest
		if( nDLSize >= 2 ) {
			for( i=0; i < nDLSize-1; i++ ) {
				for( j=i+1; j < nDLSize; j++ ) {
					if( pnDL[j] < pnDL[i] ) {
						// swap elements
						nSwap = pnDL[j];
						pnDL[j] = pnDL[i];
						pnDL[i] = nSwap;
					}
				}
			}
		}
	}	

	return (u16 *)m_aDL[nIndex];
}

const u16 *CDisplayList::GetDLWithEveryLeaf() {
	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return NULL;
	}
	return m_pnEveryLeafDL;
}

void CDisplayList::ConvertLeafIndexToColRow( u32 nLeafIndex, u32 &rnCol, u32 &rnRow ) {
	rnCol = nLeafIndex % m_nLeafsAcross;
	rnRow = nLeafIndex / m_nLeafsUp;	
}

u32 CDisplayList::ConvertLeafColRowToIndex( u32 nCol, u32 nRow ) {
	return (nCol + nRow * m_nLeafsUp);
}

void CDisplayList::ToggleNodeInDisplayList( const FWorldNode_t *pDLNode, const FWorldNode_t *pNodeToToggleFromDL ) {
	u32 nSrcIndex, nIndex;
	CLeaf *pLeaf;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return;
	}
	if( !pDLNode || !pNodeToToggleFromDL ) {
		return;
	}
	nSrcIndex = ConvertLeafColRowToIndex( pDLNode->nNodeCol, pDLNode->nNodeRow );
	pLeaf = GetLeaf( nSrcIndex );
	if( pLeaf ) {
		nIndex = ConvertLeafColRowToIndex( pNodeToToggleFromDL->nNodeCol, pNodeToToggleFromDL->nNodeRow );	
		pLeaf->ToggleDL( nIndex );
	}	
}

void CDisplayList::RemoveNodeFromDisplayList( const FWorldNode_t *pDLNode, const FWorldNode_t *pNodeToRemove ) {
	u32 nSrcIndex, nIndex;
	CLeaf *pLeaf;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return;
	}
	if( !pDLNode || !pNodeToRemove ) {
		return;
	}
	nSrcIndex = ConvertLeafColRowToIndex( pDLNode->nNodeCol, pDLNode->nNodeRow );
	pLeaf = GetLeaf( nSrcIndex );
	if( pLeaf ) {
		nIndex = ConvertLeafColRowToIndex( pNodeToRemove->nNodeCol, pNodeToRemove->nNodeRow );	
		pLeaf->RemoveFromDL( nIndex );
	}
}

void CDisplayList::AddNodeToDisplayList( const FWorldNode_t *pDLNode, const FWorldNode_t *pNodeToAdd ) {
	u32 nSrcIndex, nIndex;
	CLeaf *pLeaf;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return;
	}
	if( !pDLNode || !pNodeToAdd ) {
		return;
	}
	nSrcIndex = ConvertLeafColRowToIndex( pDLNode->nNodeCol, pDLNode->nNodeRow );
	pLeaf = GetLeaf( nSrcIndex );
	if( pLeaf ) {
		nIndex = ConvertLeafColRowToIndex( pNodeToAdd->nNodeCol, pNodeToAdd->nNodeRow );	
		pLeaf->AddToDL( nIndex );
	}
}

void CDisplayList::MergeDisplayList( CDisplayList &rSrcDL ) {
	u32 i, j, nMinCol, nMinRow, nMaxCol, nMaxRow, s, t;
	CLeaf *pLeafDst, *pLeafSrc, *pTempLeaf, *pLeafToAdd;
	CFVec2 XZ;
	s32 nSrcIndex;
	f32 fAmountToAlmostGetToNextLeaf;
	
	if( m_nTotalLeafs == 0 || rSrcDL.m_nTotalLeafs == 0 ) {
		// one or both of the Display Lists are not inited yet...
		return;
	}
	// run through all of our destination's leafs
	for( i=0; i < m_nTotalLeafs; i++ ) {
		// grab our destination leaf
		pLeafDst = (CLeaf *)m_aLeafs[i];
		if( pLeafDst ) {
			// get our dest leaf's xz point
			pLeafDst->GetLeafXZ( XZ );
			// find the closest leaf in rSrcDL to XZ
			pLeafSrc = rSrcDL.GetNearestValidLeaf( XZ );
			if( pLeafSrc ) {
				// reset our destination leaf's DL
				pLeafDst->ResetDL();
				// copy pLeafSrc's DL into pLeafDst's
				for( j=0; j < pLeafSrc->GetDLCount(); j++ ) {
					nSrcIndex = pLeafSrc->GetDLLeafIndex( j );
					if( nSrcIndex >= 0 ) {
						// grab the leaf from rSrcDL at nSrcIndex
						pTempLeaf = (CLeaf *)rSrcDL.m_aLeafs[nSrcIndex];
						if( pTempLeaf ) {
							// find the closest leaf to pTempLeaf's min [0] point in the dest DL 
							nMinCol = nMinRow = 0xFFFF;
							pTempLeaf->GetLeafXZ( XZ );
							pLeafToAdd = GetNearestValidLeaf( XZ );
							if( pLeafToAdd ) {
								// add pLeafToAdd to pLeafDst's DL (MIN)
								pLeafDst->AddToDL( pLeafToAdd->GetIndex() );
								ConvertLeafIndexToColRow( pLeafToAdd->GetIndex(), nMinCol, nMinRow );
							}
							fAmountToAlmostGetToNextLeaf = rSrcDL.GetLeafSize() * 0.999f;
							XZ.x += fAmountToAlmostGetToNextLeaf;
							XZ.y += fAmountToAlmostGetToNextLeaf;
							pLeafToAdd = GetNearestValidLeaf( XZ );
							if( pLeafToAdd ) {
								// add pLeafToAdd to pLeafDst's DL (MAX)
								pLeafDst->AddToDL( pLeafToAdd->GetIndex() );
								ConvertLeafIndexToColRow( pLeafToAdd->GetIndex(), nMaxCol, nMaxRow );
							}

							// add everything inbetween nMinCol and nMaxCol
							if( nMinCol != 0xFFFF &&
								nMinRow != 0xFFFF &&
								nMaxCol != 0xFFFF &&
								nMinRow != 0xFFFF ) {
								// all row & col fields are valid
								for( s=nMinRow; s <= nMaxRow; s++ ) {
									for( t=nMinCol; t <= nMaxCol; t++ ) {
										pLeafToAdd = GetLeaf( t, s );
										if( pLeafToAdd ) {
											pLeafDst->AddToDL( pLeafToAdd->GetIndex() );
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

f32 CDisplayList::GetLeafSize() {
	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return 0.0f;
	}
	return m_fLeafSize;
}

void CDisplayList::ResetAllLeafsDL( u32 nRadius ) {
	u32 i, j;
	CLeaf *pLeaf, *pLeaf2;
	f32 fRadius2 = (f32)(nRadius * nRadius);
	CFVec2 XZ, Pos;
	f32 fR2;

	if( m_nTotalLeafs == 0 ) {
		// not inited yet...
		return;
	}

	for( i=0; i < m_nTotalLeafs; i++ ) {
		pLeaf = (CLeaf *)m_aLeafs[i];
		if( pLeaf ) {
			pLeaf->ResetDL();
			pLeaf->GetLeafXZ( XZ );
			// find all leafs that are within nRadius of pLeaf
			for( j=0; j < m_nTotalLeafs; j++ ) {
				if( j != i ) {
					pLeaf2 = (CLeaf *)m_aLeafs[j];
					if( pLeaf2 ) {
						pLeaf2->GetLeafXZ( Pos );
						Pos -= XZ;
						fR2 = Pos.Mag2();
						if( fR2 <= fRadius2 ) {
							// pLeaf2 is within the radius, add it to pLeaf's display list
							FASSERT( j == pLeaf2->GetIndex() );
							pLeaf->AddToDL( j );
						}
					}
				}
			}
		}
	}
}

BOOL CDisplayList::AreAllChildrenInDL( const u16 *pnIntDL, const FWorldNode_t *pNode, CUIntArray &ranChildDLIndices ) {
	u32 i, j, nNumChildren;
	BOOL bFound;

	nNumChildren = 0;
	for( i=0; i < 4; i++ ) {
		if( pNode->anChildNodeIndex[i] != 0xFFFF ) {
			nNumChildren++;
		} else {
			break;
		}
	}
	if( !nNumChildren ) {
		return FALSE;
	}
	// look through the display list, seeing if all children are present
	ranChildDLIndices.RemoveAll();
	for( i=0; i < nNumChildren; i++ ) {
		bFound = FALSE;
		j=0; 
		while( pnIntDL[j] != 0xFFFF ) {
			if( pnIntDL[j] == pNode->anChildNodeIndex[i] ) {
				bFound = TRUE;
				ranChildDLIndices.Add( j );
				break;
			}
			j++;
		}
		if( !bFound ) {
			return FALSE;
		}
	}
	return TRUE;
}

static u32 _ReadIntFromBitStream( u32 nTotalBitsToRead ) {
	u32 nValue, nMask, nShift;
	s32 nBitsToRead, nNewBitNumber;

	FASSERT( nTotalBitsToRead <= 32 );

	nValue = 0;

	while( nTotalBitsToRead ) {
		nNewBitNumber = _nBitStreamRemainingBitCount - nTotalBitsToRead;

		nBitsToRead = (nNewBitNumber >= 0) ? nTotalBitsToRead : _nBitStreamRemainingBitCount;

		nShift = _nBitStreamRemainingBitCount - nBitsToRead;
		nMask = ((1<<nBitsToRead)-1) << nShift;

		nValue <<= nBitsToRead;
		nValue |= ((*_pnBitStreamByte & nMask) >> nShift);

		if( nNewBitNumber <= 0 ) {
			_nBitStreamRemainingBitCount = 8;
			_pnBitStreamByte++;
		} else {
			_nBitStreamRemainingBitCount -= nBitsToRead;
			FASSERT( _nBitStreamRemainingBitCount > 0 );
		}

		nTotalBitsToRead -= nBitsToRead;
		FASSERT( nTotalBitsToRead >= 0 );
	}

	return nValue;
}