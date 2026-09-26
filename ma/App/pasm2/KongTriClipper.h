//////////////////////////////////////////////////////////////////////////////////////
// KongTriClipper.h - Classes used to clip Kong Tris to FVis data
//
// Author: John Lafleur
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/26/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _KONGTRICLIPPER_H_
#define _KONGTRICLIPPER_H_ 1

#include <mmsystem.h>
#include "fang.h"
#include "fvis.h"
#include "KongDef.h"
#include "KongToChimp.h"



#define KTC_MAX_CLIP_BUFFER_POINTS		256
#define KTC_MAX_CLIP_PLANES				512
#define KTC_MAX_CLIP_CELLS				512
#define KTC_MAX_CLIP_VOLUMES			256
#define KTC_MAX_CLIPPED_VERTS			512
#define KTC_MAX_CLIPPED_VERT_INDICES	KTC_MAX_CLIPPED_VERTS * 4


class CApeToKongFormat;


//
enum
{
	KTC_NO_ERROR = 0,
	KTC_GENERAL_ERROR,
	KTC_CALLBACK_ERROR,
	KTC_ERROR_TRIANGULATING_CLIPPED_POLY,
	KTC_EXCEEDED_CELL_CLIP_BUFFER,
	KTC_EXCEEDED_VERTEX_CLIP_BUFFER,
	KTC_EXCEEDED_PLANE_CLIP_BUFFER,
	KTC_UNABLE_TO_CREATE_CHIMPS,
	KTC_ERROR_ADDING_CHECK_TO_VOLUMES,
	KTC_ERROR_CLOSING_CHIMPS,
};


//
//
//
struct ClipperResults
{
	// Clipping Stats
	u32				m_nTrisSubmitted;
	u32				m_nTrisInNoCells;
	u32				m_nTrisInOneCell;
	u32				m_nTrisInOneVolume;
	u32				m_nPostclippedTrisInOneVolume;
	u32				m_nAllEdgesClippedToOneVolume;
	u32				m_nTrisClipped;
	u32				m_nTrisSavedByMerger;
	u32				m_nResultingZeroAreaTris;
	u32				m_nTotalVolumesOfClippedTris;
	u32				m_nTotalVolumesConsidered;
	u32				m_nTotalVolumesTriviallyDiscarded;

	// Non-clipping stats
	u32				m_nMostTrisPerVolume;
	u32				m_nVolIdxWithMostTris;
	u32				m_nTotalWorldTris;

	u32				m_nTicksClipping;
};


//
//
class CKTC_LinkNode
{
	public:
		CKTC_LinkNode	*m_pPrior;
		CKTC_LinkNode	*m_pNext;
};

typedef void CKTC_LinkListNodeRelease(  CKTC_LinkNode *pNode );

//
//
class CKTC_LinkList
{
	public:
		CKTC_LinkNode	*m_pHead;
		CKTC_LinkNode	*m_pTail;
		s32				m_nCount;

		CKTC_LinkList( void )
		{
			m_pHead = NULL;
			m_pTail = NULL;
			m_nCount = 0;
		}

		FINLINE CKTC_LinkNode* Add( CKTC_LinkNode *pFreeNode );
		FINLINE void Remove( CKTC_LinkNode *pNode, CKTC_LinkListNodeRelease *pNodeFreeFcn );
		FINLINE void Clear( CKTC_LinkListNodeRelease *pNodeFreeFcn );
		FINLINE void Verify( void );
};

//
//
class CKTC_ClipPlane : public CKTC_LinkNode
{
	public:
		u32				nPlaneIdx;

		FINLINE CKTC_ClipPlane* Next( void )
		{
			return (CKTC_ClipPlane *)m_pNext;
		}
};

//
//
class CKTC_ClipCell : public CKTC_LinkNode
{
	public:
		FVisCell_t		*m_pCell;
		u32				m_nFirstClippedVertIdx;
		u32				m_nClippedVertIdxCount;

		CKTC_LinkList	m_ClipPlaneList;

		FINLINE CKTC_ClipCell* Next( void )
		{
			return (CKTC_ClipCell *)m_pNext;
		}
};

//
//
class CKTC_ClipVolume : public CKTC_LinkNode
{
	public:
		FVisVolume_t	*m_pVolume;

		CKTC_LinkList	m_ClipCellList;

		FINLINE CKTC_ClipVolume* Next( void )
		{
			return (CKTC_ClipVolume *)m_pNext;
		}
};


//
// Simple tick timer class
class CTickTimer
{
	private:
		u32 m_nStartTicks;
		u32 *m_pAddTo;
	public:
		CTickTimer( u32 *pnAddTo )
		{
			m_nStartTicks = timeGetTime();
			m_pAddTo = pnAddTo;
		}
		~CTickTimer( void )
		{
			(*m_pAddTo) = (*m_pAddTo) + (timeGetTime() - m_nStartTicks);
		}
};

//
//
//
class CKongClipper
{
	public:
		// Vars
		ClipperResults	m_Results;

		// Pools
		static CKTC_ClipPlane	*m_pllClipPlanePool;
		static CKTC_ClipCell	*m_pllClipCellPool;
		static CKTC_ClipVolume	*m_pllClipVolumePool;
		static u32				m_nClipPlanePoolCount;
		static u32				m_nClipCellPoolCount;
		static u32				m_nClipVolumePoolCount;

		// Volume separation of tris
		u16				*m_anVolTriCounter;
		KongTri_t		*m_pVolTriLinkListHead;
		KongTri_t		*m_pVolTriLinkListTail;

		KongTri_t		*m_pNewUnprocessedTris;

		// Linklist of clipping data
		CKTC_LinkList	m_VolumeClipList;

		static CKongClipper *m_pThis;

		// Methods
		CKongClipper( void );
		~CKongClipper( void );

		u32 ClipAndSeparateKong( CApeToKongFormat *pApeToKong, KongMesh_t *pKongMesh, CKongToChimp *paChimps, u32 nVolumeCount, CCompileDlg *pDlg = NULL ) ;

	private:

		BOOL m_bContainment;

		FINLINE u32 ClipTri( KongTri_t *pTri, u32 nMatID );
		BOOL AddTriToVolume( KongTri_t *pTri, FVisVolume_t *pVolume, BOOL bClipped, u32 nMatID );
		BOOL FreeMemoryAllocations( void );
		u32  ClipAgainstClipVolumes( KongTri_t *pTri, FVisCell_t **apPointCells, CFSphere *pBoundingSphere, u32 nMatID );
		BOOL ClipToPlane( u16 *pnSource, u16 *pnVertCount, CFVec3A *pPlaneNormal, CFVec3A *pRefPoint );
		u32 AddPoly( u16 *pVertIndices, u16 nIndexCount, KongTri_t *pTri, u32 nMatID, CKTC_ClipVolume *pClipVolume );

		// Management of static pools of link nodes
		CKTC_ClipVolume* GetClipVolumeNode( void )
		{
			if ( !m_pllClipVolumePool ) return NULL;
			CKTC_ClipVolume *pReturn = m_pllClipVolumePool;
			m_pllClipVolumePool = (CKTC_ClipVolume *)m_pllClipVolumePool->m_pNext;
			m_nClipVolumePoolCount--;
			return pReturn;
		}

		CKTC_ClipCell* GetClipCellNode( void )
		{
			if ( !m_pllClipCellPool ) return NULL;
			CKTC_ClipCell *pReturn = m_pllClipCellPool;
			m_pllClipCellPool = (CKTC_ClipCell *)m_pllClipCellPool->m_pNext;
			m_nClipCellPoolCount--;
			return pReturn;
		}

		CKTC_ClipPlane* GetClipPlaneNode( void )
		{
			if ( !m_pllClipPlanePool ) return NULL;
			CKTC_ClipPlane *pReturn = m_pllClipPlanePool;
			m_pllClipPlanePool = (CKTC_ClipPlane *)m_pllClipPlanePool->m_pNext;
			m_nClipPlanePoolCount--;
			return pReturn;
		}

		static void ReleaseClipPlane( CKTC_LinkNode *pNode )
		{
			if ( !pNode ) return;
			pNode->m_pNext = m_pllClipPlanePool;
			m_pllClipPlanePool = (CKTC_ClipPlane *)pNode;
			m_nClipPlanePoolCount++;
		}

		static void ReleaseClipCell( CKTC_LinkNode *pNode )
		{
			if ( !pNode ) return;
			CKTC_ClipCell *pCellNode = (CKTC_ClipCell *)pNode;
			pCellNode->m_ClipPlaneList.Clear( ReleaseClipPlane );
			pCellNode->m_pNext = m_pllClipCellPool;
			m_pllClipCellPool = pCellNode;
			m_nClipCellPoolCount++;
		}

		static void ReleaseClipVolume( CKTC_LinkNode *pNode )
		{
			if ( !pNode ) return;
			CKTC_ClipVolume *pVolNode = (CKTC_ClipVolume *)pNode;
			pVolNode->m_ClipCellList.Clear( ReleaseClipCell );
			pVolNode->m_pNext = m_pllClipVolumePool;
			m_pllClipVolumePool = pVolNode;
			m_nClipVolumePoolCount++;
		}


		static BOOL VolumeVsSphereCallback( FVisVolume_t *pVolume, BOOL bUsingVisData )
		{
			FASSERT( pVolume );

			if ( pVolume->nVolumeID == 0 )
			{
				// This is the "Slop Bucket" so we don't want to consider it (the
				// slop bucket only gets polys that aren't anywhere)
				return TRUE;
			}

			// Append this volume to the clipping list
			CKTC_ClipVolume *pClipVol = (CKTC_ClipVolume *)m_pThis->m_VolumeClipList.Add( m_pThis->GetClipVolumeNode() );
			if ( !pClipVol )
			{
				FASSERT_NOW;
				return FALSE;
			}

			pClipVol->m_pVolume = pVolume;
			pClipVol->m_ClipCellList.Clear( ReleaseClipCell );

			return TRUE;
		}
};


//////////////////////////////////////////////////////////////////
//
//	CKTC_LinkList Implementation
//
//////////////////////////////////////////////////////////////////

//
//
FINLINE CKTC_LinkNode* CKTC_LinkList::Add( CKTC_LinkNode *pNode )
{
	if ( !pNode )
	{
		return NULL;
	}

	m_nCount++;

	if ( !m_pHead )
	{
		m_pHead = pNode;
		m_pTail = pNode;
		pNode->m_pNext = NULL;
		pNode->m_pPrior = NULL;
#if FANG_DEBUG_BUILD
			Verify();
#endif
		return pNode;
	}

	m_pTail->m_pNext = pNode;
	pNode->m_pPrior = m_pTail;
	m_pTail = pNode;
	m_pTail->m_pNext = NULL;
#if FANG_DEBUG_BUILD
			Verify();
#endif
	return pNode;
}

//
//
FINLINE void CKTC_LinkList::Remove( CKTC_LinkNode *pNode, CKTC_LinkListNodeRelease *pNodeFreeFcn )
{
	FASSERT( pNodeFreeFcn );

	if ( !pNode )
	{
		return;
	}

	CKTC_LinkNode *pTemp = m_pHead;
	while ( pTemp )
	{
		if ( pTemp == pNode )
		{
			if ( m_pHead == pTemp )
			{
				m_pHead = m_pHead->m_pNext;
				if ( m_pHead )
				{
					m_pHead->m_pPrior = NULL;
				}
			}

			if ( m_pTail == pTemp )
			{
				m_pTail = m_pTail->m_pPrior;
				if ( m_pTail )
				{
					m_pTail->m_pNext = NULL;
				}
			}

			if ( pTemp->m_pNext )
			{
				pTemp->m_pNext->m_pPrior = pTemp->m_pPrior;
			}
			if ( pTemp->m_pPrior )
			{
				pTemp->m_pPrior->m_pNext = pTemp->m_pNext;
			}
			pTemp->m_pNext = NULL;
			pTemp->m_pPrior = NULL;
			if ( pNodeFreeFcn )
			{
				pNodeFreeFcn( pTemp );
			}
			m_nCount--;
#if FANG_DEBUG_BUILD
			Verify();
#endif
			return;
		}

		pTemp = pTemp->m_pNext;
	}
}

//
//
FINLINE void CKTC_LinkList::Clear( CKTC_LinkListNodeRelease *pNodeFreeFcn )
{
	FASSERT( m_nCount < 1000 );

	CKTC_LinkNode *pTemp = m_pHead;
	CKTC_LinkNode *pNext;
	while ( pTemp )
	{
		pNext = pTemp->m_pNext;

		// Clear the pointers
		pTemp->m_pPrior = NULL;
		pTemp->m_pNext = NULL;

		if ( pNodeFreeFcn )
		{
			pNodeFreeFcn( pTemp );
		}

		m_nCount--;

		pTemp = pNext;
	}

	FASSERT( m_nCount == 0 );

	m_nCount = 0;
	m_pHead = NULL;
	m_pTail = NULL;
}

//
//
FINLINE void CKTC_LinkList::Verify( void )
{
	s32 nNodeCounter = 0;
	if ( m_pHead )
	{
		CKTC_LinkNode *pNode1 = m_pHead;
		CKTC_LinkNode *pNode2 = m_pHead->m_pNext;
		nNodeCounter++;

		while ( pNode2 )
		{
			// Check for looping
			FASSERT( pNode2 != pNode1 );

			pNode1 = pNode1->m_pNext;
			pNode2 = pNode2->m_pNext;
			nNodeCounter++;

			// Check for looping
			FASSERT( pNode2 != pNode1 );

			if ( pNode2 )
			{
				pNode2 = pNode2->m_pNext;

				nNodeCounter++;
			}
		}
	}
	FASSERT( nNodeCounter == m_nCount );
}
#endif // _KONGTRICLIPPER_H_