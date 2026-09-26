//////////////////////////////////////////////////////////////////////////////////////
// fDataStreaming.cpp - Fang module used to stream data from storage systems
//
// Author: John Lafleur   
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
// 01/04/03 Lafleur		Created
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "fDataStreaming.h"
#include "famem.h"
#include "fres.h"
#include "fvid.h"
#include "fdraw.h"
#include "fworld.h"
#include "fviewport.h"
#include "frenderer.h"
#include "ftext.h"
#include "ftimer.h"
#include "fxfm.h"
#include "fperf.h"
#include "fvis.h"
#include "floop.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////
	
#define _VERIFY_LISTS				(FALSE & !(FANG_PRODUCTION_BUILD))
#define _DRAW_STATUS				(TRUE & !(FANG_PRODUCTION_BUILD))

#define _MAX_STATUS_VERTS 			150

#define _NEAR_CAPACITY_PERCENTAGE					75
#define _FRAMES_BEFORE_EXPIRING_AS_UNUSED			5400
#define _FRAMES_BEFORE_EXPIRING_WHEN_NEAR_CAPACITY	60


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

// Manager for streaming data from remote memory
CFStreamMgr FDS_StreamMgr;


//////////////////////////////////////////////////////////////////////////////////////
// Local variables:
//////////////////////////////////////////////////////////////////////////////////////

#if !FANG_PRODUCTION_BUILD
	static u32 _nLocalMemoryLastFrameUnavailable = 0;
#endif

#if _DRAW_STATUS
	static FDrawVtx_t _vTriVerts[_MAX_STATUS_VERTS];
#endif

static volatile u32 _nCurrentFileStreamingCount = 0;
static volatile u32 _nCurrentAMEMStreamingCount = 0;


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
// m_nManagedBytes must be set prior to initializing the local cache
//
BOOL CFBlockMgr::InitLocalCache( u32 nMinBlockSize, u32 nMaxAllocations )
{
	FASSERT( !m_bInitialized ); 
	
	FASSERT( m_nManagedBytes );
	if ( m_nManagedBytes == 0 )
	{
		return FALSE;
	}
	
#if !FANG_PRODUCTION_BUILD
	DEVPRINTF( "CFBlockMgr::InitLocalCache(): Configuring Streaming Cache to %d bytes.\n", m_nManagedBytes );
#endif
	
	FResFrame_t Frame = fres_GetFrame();
#if FANG_PLATFORM_XB
	m_pMemory = (u8 *)D3D_AllocContiguousMemory( m_nManagedBytes, 128 );
#else
	m_pMemory = (u8 *)fres_AlignedAlloc( m_nManagedBytes, 32 );
#endif
	if ( !m_pMemory )
	{
		return FALSE;
	}
	
#if _DRAW_STATUS
	fang_MemZero( _vTriVerts, sizeof( FDrawVtx_t ) * _MAX_STATUS_VERTS );
#endif

	m_nMinBlockSize = nMinBlockSize;
	
	m_nBlockCount = nMaxAllocations;
	m_paBlocks = (MemBlock_t *)fres_Alloc( sizeof( MemBlock_t ) * m_nBlockCount );
	if ( !m_paBlocks )
	{
		fres_ReleaseFrame( Frame );
		return FALSE;
	}

	ResetMemoryBlocks();

	m_bInitialized = TRUE;

	return TRUE;
}


//
//
//
void CFBlockMgr::ResetMemoryBlocks( void )
{
	// Start with a clear allocated list
	m_nAllocatedBlockCount = 0;
	m_pAllocatedBlocks = NULL;

	// Set up the first block as the first block in the free list
	m_nFreeBlockCount = 1;
	m_pMemoryBlocks = m_pFreeBlocks = &m_paBlocks[0];
	m_paBlocks[0].pStartAddress = m_pMemory;
	m_paBlocks[0].nMemoryInChain = m_nManagedBytes;
	m_paBlocks[0].pNextListBlock = NULL;
	m_paBlocks[0].pPriorListBlock = NULL;
	m_paBlocks[0].pPriorMemBlock = NULL;
	m_paBlocks[0].pNextMemBlock = NULL;
	
	// Link all remaining blocks into the unused block list
	u32 i;
	m_pUnusedBlockHeaders = NULL;
	m_nUnusedBlockHeaderCount = 0;
	for ( i = 1; i < m_nBlockCount; i++ )
	{
		m_paBlocks[i].pNextListBlock = m_pUnusedBlockHeaders;
		m_pUnusedBlockHeaders = &m_paBlocks[i];
		m_nUnusedBlockHeaderCount++;
	}

	m_nBytesInCache = 0;
}


//
//
//
void CFBlockMgr::UninitLocalCache( void )
{
	FASSERT( m_bInitialized );
	
#if FANG_PLATFORM_XB
	D3D_FreeContiguousMemory( m_pMemory );
#endif
	m_pMemory = NULL;
	m_nManagedBytes = 0;
	m_nMinBlockSize = 0;
	m_nBlockCount = 0;
	m_paBlocks = NULL;
	m_nFreeBlockCount = 0;
	m_nUnusedBlockHeaderCount = 0;
	m_nAllocatedBlockCount = 0;
	m_pFreeBlocks = NULL;
	m_pMemoryBlocks = NULL;
	m_pAllocatedBlocks = NULL;
	m_pUnusedBlockHeaders = NULL;
	
	m_nBytesInCache = 0;
	
	m_bInitialized = FALSE;
}


//
//
//
void CFBlockMgr::FlushCache( void )
{
	while ( m_pAllocatedBlocks )
	{
		FreeMemory( m_pAllocatedBlocks );
	}
}


//
//
//
FINLINE void CFBlockMgr::RemoveMemBlockFromList( MemBlock_t **ppListHead, MemBlock_t *pBlock )
{
	FASSERT( pBlock );
	
	if ( pBlock->pPriorListBlock != NULL )
	{
		pBlock->pPriorListBlock->pNextListBlock = pBlock->pNextListBlock;
	}
	else
	{
		FASSERT( *ppListHead == pBlock );
		(*ppListHead) = pBlock->pNextListBlock;
	}
	if ( pBlock->pNextListBlock )
	{
		pBlock->pNextListBlock->pPriorListBlock = pBlock->pPriorListBlock;
	}
}


//
//
//
FINLINE void CFBlockMgr::AppendMemBlockToList( MemBlock_t **ppListHead, MemBlock_t *pBlock )
{
	FASSERT( pBlock );
	
	if ( *ppListHead != NULL )
	{
		(*ppListHead)->pPriorListBlock = pBlock;
		pBlock->pNextListBlock = *ppListHead;
	}
	else
	{
		pBlock->pNextListBlock = NULL;
	}
	pBlock->pPriorListBlock = NULL;
	(*ppListHead) = pBlock;
}


//
//
//
FINLINE void CFBlockMgr::InsertMemBlockInList( MemBlock_t **ppListHead, MemBlock_t *pBlock, MemBlock_t *pInsertBefore )
{
	FASSERT( pBlock );
	
	if ( pInsertBefore->pPriorListBlock != NULL )
	{
		pInsertBefore->pPriorListBlock->pNextListBlock = pBlock;
		pBlock->pPriorListBlock = pInsertBefore->pPriorListBlock;
	}
	else
	{
		FASSERT( *ppListHead == pInsertBefore );
		(*ppListHead) = pBlock;
		pBlock->pPriorListBlock = NULL;
	}
	pInsertBefore->pPriorListBlock = pBlock;
	pBlock->pNextListBlock = pInsertBefore;
}


//
//
//
void CFBlockMgr::VerifyAllLists( void )
{
#if _VERIFY_LISTS
	u32 nBlockCount;
	
	nBlockCount = 0;
	MemBlock_t *pLastTemp = NULL, *pTemp = m_pFreeBlocks;
	while ( pTemp )
	{
		nBlockCount++;
		FASSERT( pTemp->pPriorListBlock == pLastTemp );
		pLastTemp = pTemp;
		pTemp = pTemp->pNextListBlock;
	}
	FASSERT( nBlockCount == m_nFreeBlockCount );
	
	nBlockCount = 0;
	pLastTemp = NULL;
	pTemp = m_pAllocatedBlocks;
	while ( pTemp )
	{
		nBlockCount++;
		FASSERT( pTemp->pPriorListBlock == pLastTemp );
		pLastTemp = pTemp;
		pTemp = pTemp->pNextListBlock;
	}
	FASSERT( nBlockCount == m_nAllocatedBlockCount );
	
	nBlockCount = 0;
	pTemp = m_pUnusedBlockHeaders;
	while ( pTemp )
	{
		nBlockCount++;
		pTemp = pTemp->pNextListBlock;
	}
	FASSERT( nBlockCount == m_nUnusedBlockHeaderCount );
	
	nBlockCount = 0;
	pLastTemp = NULL;
	pTemp = m_pMemoryBlocks;
	while ( pTemp )
	{
		nBlockCount++;
		FASSERT( pTemp->pPriorMemBlock == pLastTemp );
		FASSERT( !pLastTemp || pTemp->pStartAddress == pLastTemp->pStartAddress + pLastTemp->nMemoryInChain );
		pLastTemp = pTemp;
		pTemp = pTemp->pNextMemBlock;
	}
	FASSERT( nBlockCount == m_nBlockCount - m_nUnusedBlockHeaderCount );
#endif // _VERIFY_LISTS
}

//
//
//
void CFBlockMgr::DrawStatus( u32 nTotalRemoteBytes/*=0*/, u32 nAllocatedRemoteBytes/*=0*/ )
{
#if _DRAW_STATUS
	u32 nCurrentVert = 0;

	if ( FPerf_nDisplayPerfType != FPERF_TYPE_LIGHTING )
	{
		return;
	}

	// Get current viewport
	FViewport_t *pPreviousVP = fviewport_GetActive();
	
	u32 nVPWidth = FViewport_pDefaultOrtho->nWidth;
	u32 nVPHeight = FViewport_pDefaultOrtho->nHeight;
	
	fviewport_SetActive( FViewport_pDefaultOrtho );
	CFXfm m_Xfm;
	m_Xfm.Identity();
	m_Xfm.InitStackWithView();
	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	
	f32 fX = nVPWidth * 0.1f;
	f32 fY = nVPHeight * 0.05f;
	f32 fWidth = nVPWidth * 0.8f;
	f32 fHeight = nVPHeight * 0.02f;

	f32 fLastX = fX, fLastY = fY, fAllocWidth;
	MemBlock_t *pLastTemp = NULL;
	MemBlock_t *pTemp = m_pMemoryBlocks;
	while ( pTemp )
	{
		if ( nCurrentVert + 6 > _MAX_STATUS_VERTS )
		{
			fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vTriVerts, nCurrentVert );
			nCurrentVert = 0;
		}
		
		CFColorRGBA ColorLeft, ColorRight;
		if ( pTemp->nFlags & BLOCK_FLAG_USED )
		{
			ColorLeft.Set( 1.f, 0.f, 0.f, 1.f );
			ColorRight.Set( 0.f, 0.f, 0.0f, 1.f );
		}
		else
		{
			ColorLeft.Set( 0.f, 1.f, 0.f, 1.f );
			ColorRight.Set( 0.f, 1.f, 0.f, 1.f );
		}
		
		fAllocWidth = fWidth * ( (f32)pTemp->nMemoryInChain / (f32)m_nManagedBytes );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fLastX, fLastY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA = ColorLeft;
		_vTriVerts[nCurrentVert].Pos_MS.Set( fLastX, fLastY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA = ColorLeft;
		_vTriVerts[nCurrentVert].Pos_MS.Set( fLastX + fAllocWidth, fLastY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA = ColorRight;
		
		_vTriVerts[nCurrentVert].Pos_MS.Set( fLastX, fLastY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA = ColorLeft;
		_vTriVerts[nCurrentVert].Pos_MS.Set( fLastX + fAllocWidth, fLastY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA = ColorRight;
		_vTriVerts[nCurrentVert].Pos_MS.Set( fLastX + fAllocWidth, fLastY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA = ColorRight;
		
		fLastX = fLastX + fAllocWidth;
		pLastTemp = pTemp;
		pTemp = pTemp->pNextMemBlock;
	}
	
	if ( nCurrentVert > 0 )
	{
		fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vTriVerts, nCurrentVert );
		nCurrentVert = 0;
	}

	if ( nTotalRemoteBytes != 0 )
	{
		// Draw the remote memory bar
		fY = nVPHeight * 0.08f;
		fAllocWidth = fWidth * ( (f32)nAllocatedRemoteBytes / (f32)nTotalRemoteBytes );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX, fY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 1.f, 0.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX, fY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 1.f, 0.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fAllocWidth, fY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 1.f, 0.f, 0.f, 1.f );

		_vTriVerts[nCurrentVert].Pos_MS.Set( fX, fY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 1.f, 0.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fAllocWidth, fY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 1.f, 0.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fAllocWidth, fY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 1.f, 0.f, 0.f, 1.f );
		
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fAllocWidth, fY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 0.f, 1.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fAllocWidth, fY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 0.f, 1.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fWidth, fY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 0.f, 1.f, 0.f, 1.f );

		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fAllocWidth, fY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 0.f, 1.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fWidth, fY, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 0.f, 1.f, 0.f, 1.f );
		_vTriVerts[nCurrentVert].Pos_MS.Set( fX + fWidth, fY + fHeight, 1.f );
		_vTriVerts[nCurrentVert++].ColorRGBA.Set( 0.f, 1.f, 0.f, 1.f );
		
		fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vTriVerts, nCurrentVert );
	}
	
	// Restore the prior renderer
	frenderer_Pop();
	fviewport_SetActive( pPreviousVP );

	char szText[128];
	u32 nCachePercentUsed = (u32)(((f32)m_nBytesInCache/(f32)m_nManagedBytes) * 100);
	sprintf( szText, "~AL~W1Cache: %d (%d)", m_nBytesInCache, nCachePercentUsed );
	if ( nTotalRemoteBytes != 0 )
	{
		ftext_DebugPrintf( 0.4f, 0.09f, szText );
	}
	else
	{
		ftext_DebugPrintf( 0.4f, 0.06f, szText );
	}
	
#endif // _DRAW_STATUS
}


//
//
//
MemBlock_t* CFBlockMgr::AllocMemory( u32 nBytes )
{
	if ( !m_bInitialized )
	{
		// This manager is not initialized
		return NULL;
	}
	
	if ( nBytes == 0 )
	{
		return NULL;
	}
	
	if ( m_pFreeBlocks == NULL )
	{
		// There are currently no free blocks
		return NULL;
	}
	
	u32 nBytesNeeded = FMATH_BYTE_ALIGN_UP( nBytes, m_nMinBlockSize );
	
	// Search through the free blocks
	MemBlock_t *pTest = m_pFreeBlocks;
	while ( pTest )
	{
		if ( pTest->nMemoryInChain >= nBytesNeeded )
		{
			if ( pTest->nMemoryInChain == nBytesNeeded || m_nUnusedBlockHeaderCount == 0 )
			{
				// Allocate the whole chain
				
				// Remove this block from the free link list
				RemoveMemBlockFromList( &m_pFreeBlocks, pTest );
				
				m_nFreeBlockCount--;
			}
			else
			{
				// Allocate part of this chain
				
				// Fixup the remainder of the chain in the free list
				MemBlock_t *pNewBlock = m_pUnusedBlockHeaders;
				m_pUnusedBlockHeaders = m_pUnusedBlockHeaders->pNextListBlock;
				m_nUnusedBlockHeaderCount--;
				
				pNewBlock->nFlags = BLOCK_FLAG_NONE;
				pNewBlock->pStartAddress = pTest->pStartAddress + nBytesNeeded;
				pNewBlock->nMemoryInChain = pTest->nMemoryInChain - nBytesNeeded;
				pTest->nMemoryInChain = nBytesNeeded;
				
				// Place this new block in the old block's position in the free list
				pNewBlock->pPriorListBlock = pTest->pPriorListBlock;
				pNewBlock->pNextListBlock = pTest->pNextListBlock;
				if ( pTest->pPriorListBlock )
				{
					pTest->pPriorListBlock->pNextListBlock = pNewBlock;
				}
				else
				{
					FASSERT( m_pFreeBlocks == pTest );
					m_pFreeBlocks = pNewBlock;
				}
				if ( pTest->pNextListBlock )
				{
					pTest->pNextListBlock->pPriorListBlock = pNewBlock;
				}
				
				// Segment the test block in the memory block list
				pNewBlock->pNextMemBlock = pTest->pNextMemBlock;
				if ( pTest->pNextMemBlock )
				{
					pTest->pNextMemBlock->pPriorMemBlock = pNewBlock;
				}
				pNewBlock->pPriorMemBlock = pTest;
				pTest->pNextMemBlock = pNewBlock;
			}
				
			// Place the block at the head of the allocated list
			AppendMemBlockToList( &m_pAllocatedBlocks, pTest );
			m_nAllocatedBlockCount++;
			
			m_nBytesInCache += pTest->nMemoryInChain;
			
			pTest->nFlags |= BLOCK_FLAG_USED;
			
#if _VERIFY_LISTS
			VerifyAllLists();
#endif
			
			return pTest;
		}
		
		// Advance to the next free chain
		pTest = pTest->pNextListBlock;
	}
	
	// Could not find a block of acceptable size
	return NULL;	
}


//
//
//
void CFBlockMgr::FreeMemory( MemBlock_t *pRemove )
{
	FASSERT( pRemove );

	// Remove the head block from the allocated list
	RemoveMemBlockFromList( &m_pAllocatedBlocks, pRemove );
	pRemove->nFlags &= ~BLOCK_FLAG_USED;
	
	// Update the counts	
	m_nFreeBlockCount++;
	m_nAllocatedBlockCount--;
	m_nBytesInCache -= pRemove->nMemoryInChain;
	
	// Place the chain back into the free list.
	
	// Is the next mem block an unused block?	
	if ( pRemove->pNextMemBlock && !(pRemove->pNextMemBlock->nFlags & BLOCK_FLAG_USED) )
	{
		// Absorb the next block into the pRemove block
		MemBlock_t *pNextBlock = pRemove->pNextMemBlock;
		RemoveMemBlockFromList( &m_pFreeBlocks, pNextBlock );
		pRemove->nMemoryInChain += pNextBlock->nMemoryInChain;
		if ( pNextBlock->pNextMemBlock )
		{
			pNextBlock->pNextMemBlock->pPriorMemBlock = pRemove;
		}
		pRemove->pNextMemBlock = pNextBlock->pNextMemBlock;
		
		// Place the next block header in the Unused list
		pNextBlock->pNextListBlock = m_pUnusedBlockHeaders;
		m_pUnusedBlockHeaders = pNextBlock;
		m_nUnusedBlockHeaderCount++;
		m_nFreeBlockCount--;
	}
		
	// Is the prior mem block an unused block?
	if ( pRemove->pPriorMemBlock && !(pRemove->pPriorMemBlock->nFlags & BLOCK_FLAG_USED) )
	{
		// Let the prior block absorb this block
		MemBlock_t *pPriorBlock = pRemove->pPriorMemBlock;
		pPriorBlock->nMemoryInChain += pRemove->nMemoryInChain;
		pPriorBlock->pNextMemBlock = pRemove->pNextMemBlock;
		if ( pRemove->pNextMemBlock )
		{
			pRemove->pNextMemBlock->pPriorMemBlock = pPriorBlock;
		}
		
		// Place the remove block header in the Unused list
		pRemove->pNextListBlock = m_pUnusedBlockHeaders;
		m_pUnusedBlockHeaders = pRemove;
		m_nUnusedBlockHeaderCount++;
		m_nFreeBlockCount--;
		
		// Now we need to re-sort the prior block based on its new size
		if ( !pPriorBlock->pNextListBlock || pPriorBlock->nMemoryInChain < pPriorBlock->pNextListBlock->nMemoryInChain )
		{
			// pPriorBlock is already in a good position
			#if _VERIFY_LISTS
				VerifyAllLists();
			#endif
			return;
		}
		
		// We need to push pPriorBlock down the list
		MemBlock_t *pLastTest = NULL, *pTest = pPriorBlock->pNextListBlock;
		while ( pTest )
		{
			if ( pTest->nMemoryInChain >= pPriorBlock->nMemoryInChain )
			{
				// We need to put pRemove before this block
				RemoveMemBlockFromList( &m_pFreeBlocks, pPriorBlock );
				InsertMemBlockInList( &m_pFreeBlocks, pPriorBlock, pTest );
				#if _VERIFY_LISTS
					VerifyAllLists();
				#endif
				return;				
			}
			
			pLastTest = pTest;
			pTest = pTest->pNextListBlock;
		}
		
		// We need to put pRemove at the end of the list
		RemoveMemBlockFromList( &m_pFreeBlocks, pPriorBlock );
		pLastTest->pNextListBlock = pPriorBlock;
		pPriorBlock->pPriorListBlock = pLastTest;
		pPriorBlock->pNextListBlock = NULL;
		#if _VERIFY_LISTS
			VerifyAllLists();
		#endif
		return;
	}
	
	// If there are currently no free blocks, then pRemove should start them
	if ( !m_pFreeBlocks )
	{
		m_pFreeBlocks = pRemove;
		pRemove->pPriorListBlock = NULL;
		pRemove->pNextListBlock = NULL;
		#if _VERIFY_LISTS
			VerifyAllLists();
		#endif
		return;
	}
	
	// We need to put pRemove in the free list based on its size
	MemBlock_t *pLastTest = NULL, *pTest = m_pFreeBlocks;
	while ( pTest )
	{
		if ( pTest->nMemoryInChain >= pRemove->nMemoryInChain )
		{
			// We need to put pRemove before this block
			InsertMemBlockInList( &m_pFreeBlocks, pRemove, pTest );
			#if _VERIFY_LISTS
				VerifyAllLists();
			#endif
			return;				
		}
		
		pLastTest = pTest;
		pTest = pTest->pNextListBlock;
	}
	
	// We need to put pRemove at the end of the list
	pLastTest->pNextListBlock = pRemove;
	pRemove->pPriorListBlock = pLastTest;
	pRemove->pNextListBlock = NULL;
	#if _VERIFY_LISTS
		VerifyAllLists();
	#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CFStreamMgr class:
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL CFStreamMgr::ModuleStartup( void )
{
	return TRUE;
}


//
//
//
void CFStreamMgr::ModuleShutdown( void )
{
	if ( FDS_StreamMgr.m_bMgrInitialized )
	{
		FDS_StreamMgr.Uninit();
	}
}


//
//
//
BOOL CFStreamMgr::Init( u32 nCacheSizeInBytes )
{
	FASSERT( !m_bMgrInitialized );

#if !(FANG_PLATFORM_XB | FANG_PLATFORM_GC)

	return TRUE;

#else

	#if FANG_PLATFORM_XB
		u32 nMinLocalBlockSize = 256;
		u32 nTotalARAMDataBytes = 0;
		u32 nMaxAllocations = 1024;
	#elif FANG_PLATFORM_GC
		u32 nMinLocalBlockSize = 256;
		u32 nTotalARAMDataBytes = ((u32)(3.75 * (1024 * 1024)));
		u32 nMaxAllocations = 2048;
		
		// Since the data is a bit more compressed on GC, we're going to allocate
		// a slightly smaller cache for GC
//		nCacheSizeInBytes = nCacheSizeInBytes * 0.9f;
//		nCacheSizeInBytes = nCacheSizeInBytes * 0.5f;
	#endif
	
	if ( nCacheSizeInBytes < ((u32)FDS_MINIMUM_LOCAL_CACHE_SIZE) )
	{
		DEVPRINTF( "CFStreamMgr::Init() - WARNING: Specified data streaming cache of %d bytes is\n", nCacheSizeInBytes );
		DEVPRINTF( "below the minimum local cache size of %d bytes.  Minimum cache size will be used.\n", FDS_MINIMUM_LOCAL_CACHE_SIZE );
		nCacheSizeInBytes = ((u32)FDS_MINIMUM_LOCAL_CACHE_SIZE);		
	}
	
	m_nManagedBytes = nCacheSizeInBytes;

	// Initialize our local memory
	if ( !InitLocalCache( nMinLocalBlockSize, nMaxAllocations ) )
	{
		return FALSE;
	}

	// Allocate an auxiliary memory accessor	
	m_pAccessor = NULL;
	if ( nTotalARAMDataBytes )
	{
		m_pAccessor = fnew CFAMemAccessor;
		if ( !m_pAccessor )
		{
			UninitLocalCache();
			return FALSE;
		}	

		// Initialize the accessor
		m_nAMEMBytesAllocated = 0;
		m_nAMEMBytesTotal = FMATH_BYTE_ALIGN_UP( nTotalARAMDataBytes, 32 );
		if ( !m_pAccessor->Init( m_nAMEMBytesTotal, FAMEM_TYPE_FAST ) )
		{
			UninitLocalCache();
			fdelete( m_pAccessor );
			return FALSE;
		}
	}
	
	m_nMaxHandles = nMaxAllocations;
	m_nUsedHandleCount = 0;
	m_paAllocationHandles = fnew CFAllocHandle[m_nMaxHandles];
	if ( !m_paAllocationHandles )
	{
		UninitLocalCache();
		fdelete( m_pAccessor );
		return FALSE;
	}
	
	m_nLocalListCount = 0;
	m_pLocalListHead = m_pLocalListTail = NULL;
	m_nAMEMBytesDeniedAlloc = 0;
	m_nPostPauseFlushDelay = 0;
	
	m_bCacheLocked = FALSE;
	
	m_bMgrInitialized = TRUE;
	
	return TRUE;
#endif // !(FANG_PLATFORM_XB | FANG_PLATFORM_GC)
}


//
//
//
void CFStreamMgr::Uninit( void )
{
	if ( !m_bMgrInitialized )
	{
		return;
	}
	
	// Make sure all pending reads are done
	BlockOnStreaming();

	// Free our local memory
	UninitLocalCache();

	// Delete the auxiliary memory accessor	
	if ( m_pAccessor )
	{
		m_pAccessor->Free();
		fdelete( m_pAccessor );
		m_pAccessor = NULL;
	}	

	// Reset our vars
	m_nMaxHandles = 0;
	m_nUsedHandleCount = 0;
	m_nAMEMBytesAllocated = 0;
	m_nAMEMBytesTotal = 0;
	m_paAllocationHandles = NULL;
	m_nLocalListCount = 0;
	m_pLocalListHead = m_pLocalListTail = NULL;
	
	m_bMgrInitialized = FALSE;
}


//
//
//
void CFStreamMgr::Update( void )
{
	u32 nCachePercentUsed = (u32)(((f32)m_nBytesInCache/(f32)m_nManagedBytes) * 100);

	if ( m_bCacheLocked )
	{
		m_nAMEMBytesDeniedAlloc = 0;
		return;
	}


	FASSERT( m_nAllocatedBlockCount == m_nLocalListCount );

#if _VERIFY_LISTS
	if ( m_pLocalListHead )
	{
		u32 nCount = 0;
		CFAllocHandle *pHandle = m_pLocalListHead;
		while ( pHandle  )
		{
			nCount++;
			pHandle = pHandle->pNext;
		}

		FASSERT( nCount == m_nLocalListCount );
	}
	else
	{
		FASSERT( m_nLocalListCount == 0 );
	}
#endif

	if ( m_nAMEMBytesDeniedAlloc )
	{
		s32 nBytesDenied = m_nAMEMBytesDeniedAlloc;
		CFAllocHandle *pPriorHandle = NULL, *pNext, *pHandle = m_pLocalListHead;
		while ( pHandle && nBytesDenied > 0 )
		{
			pNext = pHandle->pNext;
			if ( !(pHandle->nFlags & CFAllocHandle::FLAG_STREAMING_IN)
				&& fmath_Abs((s32)FVid_nFrameCounter - (s32)pHandle->nLastAccessFrame) > 10 )
			{
				nBytesDenied -= pHandle->nAllocationSize;
				FreeMemory( pHandle->pLocalMemory );
				m_nLocalListCount--;
				
				pHandle->nFlags &= ~CFAllocHandle::FLAG_IN_CACHE;
				pHandle->pLocalMemory = NULL;
				if ( m_pLocalListHead == pHandle )
				{
					m_pLocalListHead = pHandle->pNext;
				}
				else if ( m_pLocalListTail == pHandle )
				{
					m_pLocalListTail = pPriorHandle;
					pPriorHandle->pNext = NULL;
				}
				else
				{
					pPriorHandle->pNext = pHandle->pNext;
				}
				pHandle->pNext = NULL;
			}
			else
			{
				pPriorHandle = pHandle;
			}
			pHandle = pNext;			
		}
	}
	else if ( m_nPostPauseFlushDelay == 0 )
	{
		if ( nCachePercentUsed > _NEAR_CAPACITY_PERCENTAGE )
		{
			// Go through the whole cache looking for old data
			CFAllocHandle *pPriorHandle = NULL, *pNext, *pHandle = m_pLocalListHead;
			while ( pHandle )
			{
				pNext = pHandle->pNext;
				if ( !(pHandle->nFlags & CFAllocHandle::FLAG_STREAMING_IN)
					&& fmath_Abs((s32)FVid_nFrameCounter - (s32)pHandle->nLastAccessFrame) > _FRAMES_BEFORE_EXPIRING_WHEN_NEAR_CAPACITY )
				{
					FreeMemory( pHandle->pLocalMemory );
					m_nLocalListCount--;
					
					pHandle->nFlags &= ~CFAllocHandle::FLAG_IN_CACHE;
					pHandle->pLocalMemory = NULL;
					if ( m_pLocalListHead == pHandle )
					{
						m_pLocalListHead = pHandle->pNext;
					}
					else if ( m_pLocalListTail == pHandle )
					{
						m_pLocalListTail = pPriorHandle;
						pPriorHandle->pNext = NULL;
					}
					else
					{
						pPriorHandle->pNext = pHandle->pNext;
					}
					pHandle->pNext = NULL;
				}
				else
				{
					pPriorHandle = pHandle;
				}
				pHandle = pNext;			
			}
		}
		else if ( m_nLocalListCount > 10 )
		{
			// Look at the first five handles
			u32 nAge, nHandleCounter = 0;
			CFAllocHandle *pPriorHandle = NULL, *pNext, *pHandle = m_pLocalListHead;
			while ( pHandle && nHandleCounter < 10 )
			{
				nAge = fmath_Abs((s32)FVid_nFrameCounter - (s32)pHandle->nLastAccessFrame);
				
				pNext = pHandle->pNext;
				
				if ( !(pHandle->nFlags & CFAllocHandle::FLAG_STREAMING_IN)
					&& nAge > _FRAMES_BEFORE_EXPIRING_AS_UNUSED )
				{
					FreeMemory( pHandle->pLocalMemory );
					m_nLocalListCount--;
					
					pHandle->nFlags &= ~CFAllocHandle::FLAG_IN_CACHE;
					pHandle->pLocalMemory = NULL;
					if ( m_pLocalListHead == pHandle )
					{
						m_pLocalListHead = pHandle->pNext;
					}
					else if ( m_pLocalListTail == pHandle )
					{
						m_pLocalListTail = pPriorHandle;
						pPriorHandle->pNext = NULL;
					}
					else
					{
						pPriorHandle->pNext = pHandle->pNext;
					}
					
					pHandle->pNext = NULL;
				}
				else if ( nAge == 0 )
				{
					// Put this handle at the end of the list
					// since it seems to be getting used a lot
					if ( m_pLocalListHead == pHandle )
					{
						m_pLocalListHead = pHandle->pNext;
					}
					else
					{
						pPriorHandle->pNext = pHandle->pNext;
					}
					if ( m_pLocalListTail )
					{
						m_pLocalListTail->pNext = pHandle;
					}
					m_pLocalListTail = pHandle;
					pHandle->pNext = NULL;
				}
				else
				{
					pPriorHandle = pHandle;
				}
				
				nHandleCounter++;
				pHandle = pNext;			
			}
		}
	}

	FASSERT( m_nAllocatedBlockCount == m_nLocalListCount );

#if _VERIFY_LISTS
	if ( m_pLocalListHead )
	{
		u32 nCount = 0;
		CFAllocHandle *pHandle = m_pLocalListHead;
		while ( pHandle  )
		{
			nCount++;
			pHandle = pHandle->pNext;
		}

		FASSERT( nCount == m_nLocalListCount );
	}
	else
	{
		FASSERT( m_nLocalListCount == 0 );
	}
#endif

	// If the game loop is paused, we want to delay any flushing
	if ( FLoop_bGamePaused )
	{
		m_nPostPauseFlushDelay = 60;
	}
	else if ( m_nPostPauseFlushDelay )
	{
		m_nPostPauseFlushDelay--;
	}

	m_nAMEMBytesDeniedAlloc = 0;
}


//
//
//
void CFStreamMgr::FlushCache( void )
{
	CFAllocHandle *pHandle = m_pLocalListHead;
	while ( pHandle )
	{
		FreeMemory( pHandle->pLocalMemory );
		m_nLocalListCount--;
		
		pHandle->nFlags &= ~CFAllocHandle::FLAG_IN_CACHE;
		pHandle->pLocalMemory = NULL;
		pHandle = pHandle->pNext;			
	}
	
	m_pLocalListHead = NULL;
	
	CFBlockMgr::FlushCache();
}


//
//
//
BOOL CFStreamMgr::LockCache( void )
{
	if ( !m_bMgrInitialized )
	{
		DEVPRINTF( "CFStreamMgr::LockCache() - Request to Lock Cache denied because manager not initialized.\n" );
		return FALSE;
	}

	FlushCache();

#if FANG_PLATFORM_XB
	D3D_FreeContiguousMemory( m_pMemory );
	m_pMemory = (u8 *)fang_Malloc( m_nManagedBytes, 4 );
	ResetMemoryBlocks();
#endif

	m_bCacheLocked = TRUE;
	return TRUE;
}


//
//
//
BOOL CFStreamMgr::UnlockCache( void )
{
	if ( !m_bMgrInitialized )
	{
		return FALSE;
	}
	
	if ( !m_bCacheLocked )
	{
		return TRUE;
	}

	FlushCache();

#if FANG_PLATFORM_XB
	fang_Free( m_pMemory );
	m_pMemory = (u8 *)D3D_AllocContiguousMemory( m_nManagedBytes, 128 );
	ResetMemoryBlocks();
#endif

	m_bCacheLocked = FALSE;
	
	// Tell FVis to block on the next render until the streaming
	// data is in the cache so that we don't end up missing a bunch of crap.
	fvis_BlockOnStreamingData();
	
	return TRUE;
}


//
//
//
void* CFStreamMgr::AllocFromCache( cu32 nBytes )
{
	if ( !m_bMgrInitialized || !m_bCacheLocked )
	{
		DEVPRINTF( "CFStreamMgr::AllocFromCache() - Allocation requested from unlocked cache.\n" );
		return NULL;
	}
	
	void *pMemory = AllocMemory( nBytes )->pStartAddress;
	
#if !FANG_PRODUCTION_BIULD
	if ( !pMemory )
	{
		DEVPRINTF( "CFStreamMgr::AllocFromCache() - Failed to allocate %d bytes from the locked cache.\n" );
	}
#endif
	return pMemory;
}


//
//
//
BOOL CFStreamMgr::StoreRawData( const void *pDataPointer, u32 nDataSize, FDS_Remote_Location_e nLocation )
{
	FASSERT( m_bMgrInitialized );

	if ( nLocation == FDS_LOCATION_MASTERFILE )
	{
		// Cannot store data in the masterfile
		FASSERT_NOW;
		return FALSE;
	}

	u32 nBytesToAlloc = FMATH_BYTE_ALIGN_UP( nDataSize, 32 );
	if ( m_nAMEMBytesAllocated + nBytesToAlloc > m_nAMEMBytesTotal )
	{
		return FALSE;
	}
	
	if ( m_pAccessor->Write( m_nAMEMBytesAllocated, pDataPointer, nBytesToAlloc ) == FAMEM_ERROR_NONE )
	{
		m_nAMEMBytesAllocated += nBytesToAlloc;
		return TRUE;
	}
	
	DEVPRINTF( "CFStreamMgr::StoreRawData() - Failed to Store Raw Data.\n" );
	
	return FALSE;
}


//
//
//
CFAllocHandle* CFStreamMgr::GetDataHandle( u32 nRemoteDataOffset, u32 nDataSize, FDS_Remote_Location_e nLocation )
{
	FASSERT( m_bMgrInitialized );

	if ( m_nUsedHandleCount == m_nMaxHandles )
	{
		DEVPRINTF( "CFStreamMgr::GetDataHandle() - Exceeded max streaming data handles.\n" );
		FASSERT_NOW;
		return NULL;
	}

	// Get a handle
	CFAllocHandle *pHandle = &m_paAllocationHandles[m_nUsedHandleCount];

	// Store the data based on location
	switch ( nLocation )
	{
		case FDS_LOCATION_ARAM:
		{
			if ( nRemoteDataOffset + nDataSize > m_nAMEMBytesAllocated )
			{
				DEVPRINTF( "CFStreamMgr::GetDataHandle() - Request exceeded streaming data bytes.\n" );
				return NULL;
			}

			// Fill out location specific handle data
			pHandle->nARAMAddress = nRemoteDataOffset;
			pHandle->nAllocationSize = nDataSize;

			break;
		}

		default:
			FASSERT_NOW;
			break;
	}

	// Fill in common data
	pHandle->nLocation = nLocation;
	pHandle->pLocalMemory = NULL;
	pHandle->nLastAccessFrame = 0;
	pHandle->nFlags = 0;
	
	m_nUsedHandleCount++;
	return pHandle;
}


//
//
//
CFAllocHandle* CFStreamMgr::StreamFromMasterfile( cchar *pszFilename, u32 nOffsetFromStart, s32 nBytesToStream/*=-1*/ )
{
	FASSERT( m_bMgrInitialized );

	if ( m_nUsedHandleCount == m_nMaxHandles )
	{
		DEVPRINTF( "CFStreamMgr::StreamFromMasterfile() - Exceeded max streaming data handles.\n" );
		FASSERT_NOW;
		return NULL;
	}

	// Get a handle and fill in common data
	CFAllocHandle *pHandle = &m_paAllocationHandles[m_nUsedHandleCount];

	// Attempt to open the file
	pHandle->pMFFileEntry = ffile_MF_OpenStream( pszFilename, &pHandle->nAllocationSize );
	if ( pHandle->pMFFileEntry == NULL )
	{
		DEVPRINTF( "CFStreamMgr::StreamFromMasterfile() - Unable to open file as streaming data.\n" );
		return NULL;
	}

	// fill out the remainder of the handle data
	pHandle->nStreamStartOffset = nOffsetFromStart;
	if ( nBytesToStream == -1 )
	{
		#if FANG_PLATFORM_XB
			pHandle->nAllocationSize = FMATH_BYTE_ALIGN_UP( pHandle->nAllocationSize - nOffsetFromStart, 2048 );
		#elif FANG_PLATFORM_GC
			pHandle->nAllocationSize = pHandle->nAllocationSize - nOffsetFromStart;
		#endif
	}
	else
	{
		FASSERT( nBytesToStream + nOffsetFromStart <= pHandle->nAllocationSize );
		#if FANG_PLATFORM_XB
			pHandle->nAllocationSize = FMATH_BYTE_ALIGN_UP( nBytesToStream, 2048 );
		#elif FANG_PLATFORM_GC
			pHandle->nAllocationSize = nBytesToStream;
		#endif
	}
	pHandle->nLocation = FDS_LOCATION_MASTERFILE;
	pHandle->pLocalMemory = NULL;
	pHandle->nLastAccessFrame = 0;
	pHandle->nFlags = 0;
	
	m_nUsedHandleCount++;
	return pHandle;
}


//
//
//
CFAllocHandle* CFStreamMgr::StoreData( const void *pDataPointer, u32 nDataSize, FDS_Remote_Location_e nLocation )
{
	FASSERT( m_bMgrInitialized );
	
	if ( nLocation == FDS_LOCATION_MASTERFILE )
	{
		// Cannot store data in the masterfile
		DEVPRINTF( "CFStreamMgr::StoreData() - Exceeded max streaming data handles.\n" );
		FASSERT_NOW;
		return FALSE;
	}

	if ( m_nUsedHandleCount == m_nMaxHandles )
	{
		// We've exceeded the maximum allowable handles
		return NULL;
	}
	
	// Get the next availavle handle
	CFAllocHandle *pHandle = &m_paAllocationHandles[m_nUsedHandleCount];

	// Store the data based on location
	switch ( nLocation )
	{
		case FDS_LOCATION_ARAM:
		{
			u32 nBytesToAlloc = FMATH_BYTE_ALIGN_UP( nDataSize, 32 );
			if ( m_nAMEMBytesAllocated + nBytesToAlloc > m_nAMEMBytesTotal )
			{
				DEVPRINTF( "CFStreamMgr::StoreData() - Request exceeded streaming data bytes.\n" );
				return NULL;
			}

			// Fill out the handle data
			pHandle->nLocation = nLocation;
			pHandle->nARAMAddress = m_nAMEMBytesAllocated;
			pHandle->pLocalMemory = NULL;
			pHandle->nLastAccessFrame = 0;
			pHandle->nAllocationSize = nBytesToAlloc;
			pHandle->nFlags = 0;
			
			if ( m_pAccessor->Write( m_nAMEMBytesAllocated, pDataPointer, nBytesToAlloc ) == FAMEM_ERROR_NONE )
			{
				m_nUsedHandleCount++;
				m_nAMEMBytesAllocated += nBytesToAlloc;
				return pHandle;
			}
			
			DEVPRINTF( "CFStreamMgr::StoreData() - Failed to Store Data.\n" );

			break;
		}

		default:
			FASSERT_NOW;
			break;
	}
	
	return NULL;
}


//
//  Asyncronous read callback for the AMEM system
//
void CFStreamMgr::AMemCallback( FAmem_Operation_e nOp, CFAMemAccessor *pAMemAccessor, void *pUser, FAmem_Error_e nError )
{
	// Clear the streaming flag
	((CFAllocHandle *)pUser)->nFlags &= ~CFAllocHandle::FLAG_STREAMING_IN;

	_nCurrentAMEMStreamingCount--;
	FASSERT( _nCurrentAMEMStreamingCount >=0 );

	// If we successfully read the data in, flag it as in the cache
	if ( nError == FAMEM_ERROR_NONE && nOp == FAMEM_OP_READ )
	{
		((CFAllocHandle *)pUser)->nFlags |= CFAllocHandle::FLAG_IN_CACHE;
	}
	else
	{
		// Otherwise, free the local memory
		DEVPRINTF( "CFStreamMgr::AMemCallback() - Auxilliary memory read failed.\n" );
		FDS_StreamMgr.FreeMemory( ((CFAllocHandle *)pUser)->pLocalMemory );
		((CFAllocHandle *)pUser)->pLocalMemory = NULL;
	}
}


//
//  Asyncronous read callback for the file system
//
void CFStreamMgr::FFileCallback( s32 nErrorCode, FFileHandle hFile, void *pDestBuffer, u32 uReadAmt, void *pUser )
{
	// Clear the streaming flag
	((CFAllocHandle *)pUser)->nFlags &= ~CFAllocHandle::FLAG_STREAMING_IN;

	_nCurrentFileStreamingCount--;
	FASSERT( _nCurrentFileStreamingCount >=0 );

	// If we successfully read the data in, flag it as in the cache
	if ( nErrorCode != FFILE_ERROR_RET )
	{
		((CFAllocHandle *)pUser)->nFlags |= CFAllocHandle::FLAG_IN_CACHE;
//		DEVPRINTF( "CFStreamMgr::FFileCallback() - Async file read of %x successful.\n", pUser );
	}
	else
	{
		// Otherwise, free the local memory
		DEVPRINTF( "CFStreamMgr::FFileCallback() - Async file read failed.\n" );
		FDS_StreamMgr.FreeMemory( ((CFAllocHandle *)pUser)->pLocalMemory );
		((CFAllocHandle *)pUser)->pLocalMemory = NULL;
	}
}


//
//
//
void CFStreamMgr::StartStreaming( CFAllocHandle *pHandle )
{
	FASSERT( pHandle );
	
	if ( m_bCacheLocked )
	{
		// Cannot load into a locked cache
		return;
	}
	
#if _VERIFY_LISTS
	if ( m_pLocalListHead )
	{
		u32 nCount = 0;
		CFAllocHandle *pHandle = m_pLocalListHead;
		while ( pHandle  )
		{
			nCount++;
			pHandle = pHandle->pNext;
		}

		FASSERT( nCount == m_nLocalListCount );
	}
	else
	{
		FASSERT( m_nLocalListCount == 0 );
	}
#endif

	// Data is not local.  We need to request it from auxiliary memory
	pHandle->pLocalMemory = AllocMemory( pHandle->nAllocationSize );
	if ( !pHandle->pLocalMemory )
	{
		#if !FANG_PRODUCTION_BUILD
			if ( FVid_nFrameCounter >= _nLocalMemoryLastFrameUnavailable )
			{
				DEVPRINTF( "CFStreamMgr::StartStreaming() - No Local Memory for Remote Data.\n" );
				_nLocalMemoryLastFrameUnavailable = FVid_nFrameCounter + 60;
			}
		#endif
	
		m_nAMEMBytesDeniedAlloc += pHandle->nAllocationSize;
		return;
	}
	
	pHandle->nFlags |= CFAllocHandle::FLAG_STREAMING_IN;

	switch ( pHandle->nLocation )
	{
		case FDS_LOCATION_ARAM:
		{
			_nCurrentAMEMStreamingCount++;
			FAmem_Error_e nReadResult = m_pAccessor->Read( pHandle->nARAMAddress, pHandle->pLocalMemory->pStartAddress, pHandle->nAllocationSize, CFStreamMgr::AMemCallback, pHandle );
			if ( nReadResult != FAMEM_ERROR_NONE )
			{
				_nCurrentAMEMStreamingCount--;
				DEVPRINTF( "CFStreamMgr::StartStreaming() - Failed to read remote auxilliary memory data into local memory.\n" );
				FreeMemory( pHandle->pLocalMemory );
				pHandle->pLocalMemory = NULL;
				pHandle->nFlags &= ~CFAllocHandle::FLAG_STREAMING_IN;
				return;
			}
			break;
		}

		case FDS_LOCATION_MASTERFILE:
		{
			_nCurrentFileStreamingCount++;
//			DEVPRINTF( "CFStreamMgr::StartStreaming() - Attempting to read %x.\n", pHandle );
			if ( ffile_MF_ReadStream( pHandle->pMFFileEntry, pHandle->nStreamStartOffset, pHandle->nAllocationSize, pHandle->pLocalMemory->pStartAddress, FFileCallback, pHandle ) == FFILE_ERROR_RET )
			{
				_nCurrentFileStreamingCount--;
				DEVPRINTF( "CFStreamMgr::StartStreaming() - Failed to read remote file data into local memory.\n" );
				FreeMemory( pHandle->pLocalMemory );
				pHandle->pLocalMemory = NULL;
				pHandle->nFlags &= ~CFAllocHandle::FLAG_STREAMING_IN;
				return;
			}
			
			break;
		}

		default:
			FASSERT_NOW;
			break;
	}

	// Put this handle in the local list
	if ( !m_pLocalListHead )
	{
		m_pLocalListHead = pHandle;
	}
	else
	{
		m_pLocalListTail->pNext = pHandle;
	}
	m_pLocalListTail = pHandle;
	pHandle->pNext = NULL;
	m_nLocalListCount++;

#if _VERIFY_LISTS
	if ( m_pLocalListHead )
	{
		u32 nCount = 0;
		CFAllocHandle *pHandle = m_pLocalListHead;
		while ( pHandle  )
		{
			nCount++;
			pHandle = pHandle->pNext;
		}

		FASSERT( nCount == m_nLocalListCount );
	}
	else
	{
		FASSERT( m_nLocalListCount == 0 );
	}
#endif

}


//
//
//
void CFStreamMgr::BlockOnStreaming( void )
{
	CFTimer BlockTimer;
	BlockTimer.Reset();

	DEVPRINTF( "CFStreamMgr::BlockOnStreaming() - Waiting for data to stream in...\n" );

	BOOL bStillStreaming = TRUE;
	while ( bStillStreaming && BlockTimer.SampleSeconds( FALSE ) < 1.5f )
	{
		bStillStreaming = FALSE;
		
		// Go through the whole cache looking pending reads
		CFAllocHandle *pHandle = FDS_StreamMgr.m_pLocalListHead;
		while ( pHandle )
		{
			if ( pHandle->nFlags & CFAllocHandle::FLAG_STREAMING_IN )
			{
				bStillStreaming = TRUE;
				break;
			}
			pHandle = pHandle->pNext;			
		}
	}

	DEVPRINTF( "CFStreamMgr::BlockOnStreaming() - Waited for %f seconds.\n", BlockTimer.SampleSeconds( FALSE ) );

#if !FANG_PRODUCTION_BUILD
	if ( bStillStreaming )
	{
		DEVPRINTF( "CFStreamMgr::BlockOnStreaming() - WARNING! Exceeded Block time. Advancing through block with reads pending.\n" );
	}
#endif
}