//////////////////////////////////////////////////////////////////////////////////////
// MLHash.h - Fang2MeshLib hash table classes
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
// 09/27/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLHASH_H_
#define __MLHASH_H_

#include "fang.h"
#include "ml.h"
#include "dx\fdx8vb.h"
#include "gc\fGCDisplayList.h"


#define ML_PC_MAX_VERT_NODES				65536
#define MLHASH_MAX_GC_ATTRIBUTE_INDICES		32768


//
//	Node for linklists of verts
//
class CHashNode
{
	public:
		void		*m_pData;
		u32			m_nIndex;
		CHashNode	*m_pNext;

		CHashNode	*m_pHashNext;  // Next node in the hash table
};


//
//	Structure for collecting vert nodes
//
class CHashTable
{
	public:
		u16			m_nHashTableSize;
		u16			m_nMaxNodeCount;
		u32			m_nActiveNodeCount;
		CHashNode	*m_pFirstActiveNode;
		CHashNode	*m_pLastActiveNode;

		CHashNode	**m_apHashTableHead;
		CHashNode	**m_apHashTableTail;

		CHashNode	*m_paHashNodes;

		CHashTable( void )
		{
			m_nActiveNodeCount = 0;
			m_nHashTableSize = 0;
			m_pFirstActiveNode = NULL;
			m_pLastActiveNode = NULL;
			m_paHashNodes = NULL;
			m_apHashTableHead = NULL;
			m_apHashTableTail = NULL;
		}

		~CHashTable( void )
		{
			if ( m_apHashTableHead )
			{
				free( m_apHashTableHead );
			}
			if ( m_apHashTableTail )
			{
				free( m_apHashTableTail );
			}
			if ( m_paHashNodes )
			{
				free( m_paHashNodes );
			}
		}

		FINLINE BOOL Init( u16 nHashTableSize, u16 nMaxNodeCount )
		{
			FASSERT( nMaxNodeCount < 65534 );

			FASSERT( !m_apHashTableHead & !m_apHashTableTail && !m_paHashNodes );

			m_apHashTableHead = (CHashNode **)malloc( sizeof( CHashNode* ) * nHashTableSize);
			if ( !m_apHashTableHead )
			{
				return FALSE;
			}

			m_apHashTableTail = (CHashNode **)malloc( sizeof( CHashNode* ) * nHashTableSize);
			if ( !m_apHashTableTail )
			{
				return FALSE;
			}

			m_paHashNodes = (CHashNode *)malloc( sizeof( CHashNode ) * nMaxNodeCount );
			if ( !m_paHashNodes )
			{
				return FALSE;
			}

			m_nHashTableSize = nHashTableSize;
			m_nMaxNodeCount = nMaxNodeCount;

			Clear();

			return TRUE;
		}

		FINLINE void Clear( void )
		{
			m_nActiveNodeCount = 0;
			m_pFirstActiveNode = NULL;
			m_pLastActiveNode = NULL;

			u32 i;
			for ( i = 0; i < m_nHashTableSize; i++ )
			{
				m_apHashTableHead[i] = NULL;
				m_apHashTableTail[i] = NULL;
			}
		}

		FINLINE u16 AddData( void *pNewData, u16 nHashKey )
		{
			FASSERT( pNewData && nHashKey >=0 && nHashKey < m_nHashTableSize );

			if ( m_nActiveNodeCount == m_nMaxNodeCount )
			{
				FASSERT_NOW;
				return 0xffff;
			}
			if ( m_nActiveNodeCount == 65534 )
			{
				FASSERT_NOW;
				return 0xffff;
			}
			CHashNode *pNewNode = &m_paHashNodes[m_nActiveNodeCount];

			pNewNode->m_pData = pNewData;
			pNewNode->m_pNext = NULL;
			pNewNode->m_nIndex = m_nActiveNodeCount;
			m_nActiveNodeCount++;

			pNewNode->m_pHashNext = NULL;

			if ( !m_apHashTableHead[nHashKey] )
			{
				m_apHashTableHead[nHashKey] = pNewNode;
				m_apHashTableTail[nHashKey] = pNewNode;
			}
			else
			{
				m_apHashTableTail[nHashKey]->m_pHashNext = pNewNode;
				m_apHashTableTail[nHashKey] = pNewNode;
			}

			if ( !m_pFirstActiveNode )
			{
				m_pFirstActiveNode = pNewNode;
				m_pLastActiveNode = pNewNode;
				return (u16)(m_nActiveNodeCount - 1);
			}

			m_pLastActiveNode->m_pNext = pNewNode;
			m_pLastActiveNode = pNewNode;

			return (u16)(m_nActiveNodeCount - 1);
		}

};


#define MLHASH_MAX_PC_TABLES		24
//
//
//
class CPCVertHash
{
	public:
		CHashTable	*m_apHashTables[MLHASH_MAX_PC_TABLES];
		KongVert_t	*m_pSampleHashVert[MLHASH_MAX_PC_TABLES];
		u16			m_nHashTableCount;
		u16			m_nHashTableSize;

		u32			m_nDuplicateVerts;
		u32			m_nVertsProcessed;

	public:
		CPCVertHash( void )
		{
			u32 i;
			for ( i = 0; i < MLHASH_MAX_PC_TABLES; i++ )
			{
                m_apHashTables[i] = NULL;
				m_pSampleHashVert[i] = NULL;
			}

			m_nHashTableCount = 0;
			m_nDuplicateVerts = 0;
			m_nVertsProcessed = 0;
		}

		~CPCVertHash( void )
		{
/*
			DEVPRINTF( "\nCPCVertHash:: Percent Duplicate -  " );
			if ( m_nVertsProcessed )
			{
				DEVPRINTF( "Verts (%d): %4.2f  ", m_nVertsProcessed, ((f32)m_nDuplicateVerts / (f32)m_nVertsProcessed) * 100.f );
			}
			DEVPRINTF( "\n" );
*/
			Clear();
		}

		FINLINE BOOL Init( void )
		{
/*
			// Default values for the PC hash table
			m_nHashTableCount = FDX8VB_TYPE_COUNT;
			m_nHashTableSize = 1024;

			m_paHashTables = new CHashTable[m_nHashTableCount];
			if ( !m_paHashTables )
			{
				return FALSE;
			}

			int i;
			for ( i = 0; i < m_nHashTableCount; i++ )
			{
				if ( !m_paHashTables[i].Init( m_nHashTableSize, 65533 ) )
				{
					return FALSE;
				}
			}
*/
			m_nHashTableSize = 1024;

			Clear();

			return TRUE;
		}

		FINLINE void Clear( void )
		{
			u32 i;
			for ( i = 0; i < m_nHashTableCount; i++ )
			{
				FASSERT( m_apHashTables[i] );
				delete m_apHashTables[i];
				m_apHashTables[i] = NULL;
				m_pSampleHashVert[i] = NULL;
			}
			FASSERT( !m_apHashTables[i] );

			m_nHashTableCount = 0;
		}

		FINLINE s16 GetTableIdx( KongVert_t *pSampleVert )
		{
			FASSERT( pSampleVert );

			s16 nTableIndex;
			for ( nTableIndex = 0; nTableIndex < m_nHashTableCount; nTableIndex++ )
			{
				if (   (pSampleVert->nNumWeights == 0) == (m_pSampleHashVert[nTableIndex]->nNumWeights == 0)
					&& pSampleVert->nBaseUVCount         == m_pSampleHashVert[nTableIndex]->nBaseUVCount
					&& pSampleVert->nLightMapUVCount     == m_pSampleHashVert[nTableIndex]->nLightMapUVCount )
				{
					return nTableIndex;
				}
			}

			return -1;
		}

		FINLINE u16 AddVert( KongVert_t *pNewVert, u16 &nVBIndex )
		{
			FASSERT( pNewVert );

			u16 nHashKey = GetHashKey( pNewVert );

			m_nVertsProcessed++;

			// See if we can put this vert in an existing table
			u32 nTableIndex;
			for ( nTableIndex = 0; nTableIndex < m_nHashTableCount; nTableIndex++ )
			{
				if (   (pNewVert->nNumWeights == 0) == (m_pSampleHashVert[nTableIndex]->nNumWeights == 0)
					&& pNewVert->nBaseUVCount         == m_pSampleHashVert[nTableIndex]->nBaseUVCount
					&& pNewVert->nLightMapUVCount     == m_pSampleHashVert[nTableIndex]->nLightMapUVCount )
				{
					break;
				}
			}

			if ( nTableIndex == m_nHashTableCount )
			{
				// Didn't find an existing hash table that will support this vertex, so create another
				if ( m_nHashTableCount == MLHASH_MAX_PC_TABLES )
				{
					FASSERT_NOW;
					return 0xffff;
				}

				m_apHashTables[m_nHashTableCount] = new CHashTable;
				if ( !m_apHashTables[m_nHashTableCount] )
				{
					return 0xffff;
				}

				if ( !m_apHashTables[m_nHashTableCount]->Init( m_nHashTableSize, 65533 ) )
				{
					delete m_apHashTables[m_nHashTableCount];
					return 0xffff;
				}
				nTableIndex = m_nHashTableCount;
				m_nHashTableCount++;

				nVBIndex = nTableIndex;
				m_pSampleHashVert[nTableIndex] = pNewVert;
				return m_apHashTables[nTableIndex]->AddData( pNewVert, nHashKey );
			}

			nVBIndex = nTableIndex;

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewVert, nTableIndex, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicateVerts++;
				return (u16)nIndex;
			}

			return m_apHashTables[nTableIndex]->AddData( pNewVert, nHashKey );
		}

	protected:
		FINLINE u16 GetHashKey( KongVert_t *pVert )
		{
			FASSERT( pVert );

			u16 nOMHashTableSize = m_nHashTableSize - 1;

			f32 fX = (f32)fmath_Abs( pVert->Pos.x - (s32)pVert->Pos.x );
			u16 nHashKey = (u16)(fX * nOMHashTableSize);
			FASSERT( nHashKey < m_nHashTableSize && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const KongVert_t *pVert, u16 nTableIndex, u16 nHashKey )
		{
			FASSERT( pVert && nTableIndex >= 0 && nTableIndex < m_nHashTableCount );
			FASSERT( nHashKey >= 0 && nHashKey < m_apHashTables[nTableIndex]->m_nHashTableSize );

			u32 i;
			CHashNode *pNode = m_apHashTables[nTableIndex]->m_apHashTableHead[nHashKey];
			KongVert_t *pTestVert;
			while ( pNode )
			{
				pTestVert = (KongVert_t *)pNode->m_pData;
				if (	pTestVert->Pos == pVert->Pos
					&&	pTestVert->Norm == pVert->Norm
					&&	pTestVert->Color == pVert->Color )
				{
					for ( i = 0; i < (u16)(pTestVert->nBaseUVCount + pTestVert->nLightMapUVCount); i++ )
					{
						if ( pTestVert->aUV[i] != pVert->aUV[i] )
						{
							break;
						}
					}
					if ( i == pTestVert->nBaseUVCount + pTestVert->nLightMapUVCount )
					{
						return pNode->m_nIndex;
					}
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}
};


//
//
//
struct MLH_GCColor_t
{
	u32			nPosIdx;
	u16			nNormIdx;
	u16			nVBIdx;
	FGCColor_t	Color;
};


//
//
//
class CGCVertHash
{
	public:
		u16			m_nHashTableCount;
		CHashTable	*m_paHashTables;
		u16			*m_pnHashTableSize;

		u32			m_nDuplicatePosF32;
		u32			m_nProcessedPosF32;

		u32			m_nDuplicatePosS16;
		u32			m_nProcessedPosS16;

		u32			m_nDuplicatePosS8;
		u32			m_nProcessedPosS8;

		u32			m_nDuplicateNBT8;
		u32			m_nProcessedNBT8;

		u32			m_nDuplicateColor;
		u32			m_nProcessedColor;

		u32			m_nDuplicateST16;
		u32			m_nProcessedST16;

		u32			m_nPosF32Count;
		FGCPosF32_t	m_PosF32[MLHASH_MAX_GC_ATTRIBUTE_INDICES];

		u32			m_nPosS16Count;
		FGCPosS16_t	m_PosS16[MLHASH_MAX_GC_ATTRIBUTE_INDICES];

		u32			m_nPosS8Count;
		FGCPosS8_t	m_PosS8[MLHASH_MAX_GC_ATTRIBUTE_INDICES];

		u32			m_nNBT8Count;
		FGCNBT8_s	m_NBT8[MLHASH_MAX_GC_ATTRIBUTE_INDICES];

		u32			m_nColorCount;
		MLH_GCColor_t	m_Color[MLHASH_MAX_GC_ATTRIBUTE_INDICES];

		u32			m_nST16Count;
		FGCST16_t	m_ST16[MLHASH_MAX_GC_ATTRIBUTE_INDICES];

	public:
		CGCVertHash( void )
		{
			m_paHashTables = NULL;
			m_nHashTableCount = 0;

			m_nDuplicatePosF32 = 0;
			m_nProcessedPosF32 = 0;
			m_nDuplicatePosS16 = 0;
			m_nProcessedPosS16 = 0;
			m_nDuplicatePosS8 = 0;
			m_nProcessedPosS8 = 0;
			m_nDuplicateNBT8 = 0;
			m_nProcessedNBT8 = 0;
			m_nDuplicateColor = 0;
			m_nProcessedColor = 0;
			m_nDuplicateST16 = 0;
			m_nProcessedST16 = 0;

			m_nPosF32Count = 0;
			m_nPosS16Count = 0;
			m_nPosS8Count = 0;
			m_nNBT8Count = 0;
			m_nColorCount = 0;
			m_nST16Count = 0;
		}

		~CGCVertHash( void )
		{
/*
			DEVPRINTF( "\nCGCVertHash:: Percent Duplicate -  " );
			if ( m_nProcessedPosF32 )
			{
				DEVPRINTF( "Pos32 (%d): %4.2f  ", m_nProcessedPosF32, ((f32)m_nDuplicatePosF32 / (f32)m_nProcessedPosF32) * 100.f );
			}
			if ( m_nProcessedPosS16 )
			{
				DEVPRINTF( "Pos16 (%d): %4.2f  ", m_nProcessedPosS16, ((f32)m_nDuplicatePosS16 / (f32)m_nProcessedPosS16) * 100.f );
			}
			if ( m_nProcessedPosS8 )
			{
				DEVPRINTF( "Pos8 (%d): %4.2f  ", m_nProcessedPosS8, ((f32)m_nDuplicatePosS8 / (f32)m_nProcessedPosS8) * 100.f );
			}
			if ( m_nProcessedNBT8 )
			{
				DEVPRINTF( "NBT8 (%d): %4.2f  ", m_nProcessedNBT8, ((f32)m_nDuplicateNBT8 / (f32)m_nProcessedNBT8) * 100.f );
			}
			if ( m_nProcessedColor )
			{
				DEVPRINTF( "Color (%d): %4.2f  ", m_nProcessedColor, ((f32)m_nDuplicateColor / (f32)m_nProcessedColor) * 100.f );
			}
			if ( m_nProcessedST16 )
			{
				DEVPRINTF( "ST (%d): %4.2f  ", m_nProcessedST16, ((f32)m_nDuplicateST16 / (f32)m_nProcessedST16) * 100.f );
			}
			DEVPRINTF( "\n" );
*/
			if ( m_paHashTables )
			{
				delete[] m_paHashTables;
			}

			if ( m_pnHashTableSize )
			{
				free( m_pnHashTableSize );
			}
		}

		FINLINE BOOL Init( void )
		{
			// Default values for the PC hash table
			m_nHashTableCount = 6;

			m_paHashTables = new CHashTable[m_nHashTableCount];
			if ( !m_paHashTables )
			{
				return FALSE;
			}

			m_pnHashTableSize = (u16 *)malloc( sizeof(u16) * m_nHashTableCount );
			if ( !m_pnHashTableSize )
			{
				return FALSE;
			}

			// Table for float positions
			if ( !m_paHashTables[0].Init( 1024, MLHASH_MAX_GC_ATTRIBUTE_INDICES ) )
			{
				return FALSE;
			}
			m_pnHashTableSize[0] = 1024;

			// Table for 16 bit positions
			if ( !m_paHashTables[1].Init( 1024, MLHASH_MAX_GC_ATTRIBUTE_INDICES ) )
			{
				return FALSE;
			}
			m_pnHashTableSize[1] = 1024;

			// Table for 8 bit positions
			if ( !m_paHashTables[2].Init( 256, MLHASH_MAX_GC_ATTRIBUTE_INDICES ) )
			{
				return FALSE;
			}
			m_pnHashTableSize[2] = 256;

			// Table for Color
			if ( !m_paHashTables[3].Init( 256, MLHASH_MAX_GC_ATTRIBUTE_INDICES ) )
			{
				return FALSE;
			}
			m_pnHashTableSize[3] = 256;

			// Table for 16 bit UVs
			if ( !m_paHashTables[4].Init( 1024, MLHASH_MAX_GC_ATTRIBUTE_INDICES ) )
			{
				return FALSE;
			}
			m_pnHashTableSize[4] = 1024;

			// Table for 8 bit NBTs
			if ( !m_paHashTables[5].Init( 256, MLHASH_MAX_GC_ATTRIBUTE_INDICES ) )
			{
				return FALSE;
			}
			m_pnHashTableSize[5] = 256;

			return TRUE;
		}

		FINLINE void Clear( void )
		{
			u32 i;
			for ( i = 0; i < m_nHashTableCount; i++ )
			{
				m_paHashTables[i].Clear();
			}
			m_nPosF32Count = 0;
			m_nPosS16Count = 0;
			m_nPosS8Count = 0;
			m_nColorCount = 0;
			m_nST16Count = 0;
			m_nNBT8Count = 0;
		}

		FINLINE u16 AddVert( FGCPosF32_t *pNewVert )
		{
			FASSERT( pNewVert );

			u16 nHashKey = GetHashKey( pNewVert );

			m_nProcessedPosF32++;

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewVert, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicatePosF32++;
				return (u16)nIndex;
			}

			fang_MemCopy( &m_PosF32[ m_nPosF32Count ], pNewVert, sizeof( FGCPosF32_t ) );
			m_nPosF32Count++;

			return m_paHashTables[0].AddData( &m_PosF32[ m_nPosF32Count - 1 ], nHashKey );
		}

		FINLINE u16 AddVert( FGCPosS16_t *pNewVert )
		{
			FASSERT( pNewVert );

			u16 nHashKey = GetHashKey( pNewVert );

			m_nProcessedPosS16++;

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewVert, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicatePosS16++;
				return (u16)nIndex;
			}

			fang_MemCopy( &m_PosS16[ m_nPosS16Count ], pNewVert, sizeof( FGCPosS16_t ) );
			m_nPosS16Count++;

			return m_paHashTables[1].AddData( &m_PosS16[ m_nPosS16Count - 1 ], nHashKey );
		}

		FINLINE u16 AddVert( FGCPosS8_t *pNewVert )
		{
			FASSERT( pNewVert );

			u16 nHashKey = GetHashKey( pNewVert );

			m_nProcessedPosS8++;

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewVert, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicatePosS8++;
				return (u16)nIndex;
			}

			fang_MemCopy( &m_PosS8[ m_nPosS8Count ], pNewVert, sizeof( FGCPosS8_t ) );
			m_nPosS8Count++;

			return m_paHashTables[2].AddData( &m_PosS8[ m_nPosS8Count - 1 ], nHashKey );
		}

		FINLINE u16 AddColor( FGCColor_t *pNewColor )
		{
			FASSERT( pNewColor );

			u16 nHashKey;

			nHashKey = GetColorHashKey( pNewColor );

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewColor, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicateColor++;
				return (u16)nIndex;
			}

			m_nProcessedColor++;

//			fang_MemCopy( &m_Color[ m_nColorCount ].vPos, pPos, sizeof( CFVec3 ) );
//			fang_MemCopy( &m_Color[ m_nColorCount ].vNormal, pNorm, sizeof( CFVec3 ) );
			fang_MemCopy( &m_Color[ m_nColorCount ].Color, pNewColor, sizeof( FGCColor_t ) );
			m_nColorCount++;

			return m_paHashTables[3].AddData( &m_Color[ m_nColorCount - 1 ], nHashKey );
		}

		FINLINE u16 AddColor( u16 nVBIndex, u16 nPosIdx, u16 nNormIdx, FGCColor_t *pNewColor )
		{
			FASSERT( pNewColor );

			u16 nHashKey;

			nHashKey = GetColorHashKey( nNormIdx );

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( nVBIndex, nPosIdx, nNormIdx, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicateColor++;
				return (u16)nIndex;
			}

			m_nProcessedColor++;

			m_Color[ m_nColorCount ].nVBIdx = nVBIndex;
			m_Color[ m_nColorCount ].nPosIdx = nPosIdx;
			m_Color[ m_nColorCount ].nNormIdx = nNormIdx;
			fang_MemCopy( &m_Color[ m_nColorCount ].Color, pNewColor, sizeof( FGCColor_t ) );
			m_nColorCount++;

			return m_paHashTables[3].AddData( &m_Color[ m_nColorCount - 1 ], nHashKey );
		}

		FINLINE u16 AddST( FGCST16_t *pNewST )
		{
			FASSERT( pNewST );

			u16 nHashKey = GetHashKey( pNewST );

			m_nProcessedST16++;

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewST, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicateST16++;
				return (u16)nIndex;
			}

			fang_MemCopy( &m_ST16[ m_nST16Count ], pNewST, sizeof( FGCST16_t ) );
			m_nST16Count++;

			return m_paHashTables[4].AddData( &m_ST16[ m_nST16Count - 1 ], nHashKey );
		}

		FINLINE u16 AddNBT8( FGCNBT8_t *pNewNBT )
		{
			FASSERT( pNewNBT );

			u16 nHashKey = GetHashKey( pNewNBT );

			m_nProcessedNBT8++;

			// See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewNBT, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicateNBT8++;
				return (u16)nIndex;
			}

			fang_MemCopy( &m_NBT8[ m_nNBT8Count ], pNewNBT, sizeof( FGCNBT8_t ) );
			m_nNBT8Count++;

			return m_paHashTables[5].AddData( &m_NBT8[ m_nNBT8Count - 1 ], nHashKey );
		}

	protected:
		// 32 bit position hash
		FINLINE u16 GetHashKey( FGCPosF32_t *pVert )
		{
			FASSERT( pVert );

			u16 nOMHashTableSize = m_pnHashTableSize[0] - 1;

			f32 fX = (f32)fmath_Abs( pVert->x - (s32)pVert->x );
			u16 nHashKey = (u16)(fX * nOMHashTableSize);

			FASSERT( nHashKey < m_pnHashTableSize[0] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const FGCPosF32_t *pVert, u16 nHashKey )
		{
			FASSERT( pVert );
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[0].m_nHashTableSize );

			CHashNode *pNode = m_paHashTables[0].m_apHashTableHead[nHashKey];
			FGCPosF32_t *pTestVert;
			while ( pNode )
			{
				pTestVert = (FGCPosF32_t *)pNode->m_pData;
				if (	pTestVert->x == pVert->x
					&&	pTestVert->y == pVert->y
					&&	pTestVert->z == pVert->z )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}

		// 16 bit position hash
		FINLINE u16 GetHashKey( FGCPosS16_t *pVert )
		{
			FASSERT( pVert );

			u16 nHashKey = (u16)(pVert->x & 0x3ff);

			FASSERT( nHashKey < m_pnHashTableSize[1] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const FGCPosS16_t *pVert, u16 nHashKey )
		{
			FASSERT( pVert );
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[1].m_nHashTableSize );

			CHashNode *pNode = m_paHashTables[1].m_apHashTableHead[nHashKey];
			FGCPosS16_t *pTestVert;
			while ( pNode )
			{
				pTestVert = (FGCPosS16_t *)pNode->m_pData;
				if (	pTestVert->x == pVert->x
					&&	pTestVert->y == pVert->y
					&&	pTestVert->z == pVert->z )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}

		// 8 bit position hash
		FINLINE u16 GetHashKey( FGCPosS8_t *pVert )
		{
			FASSERT( pVert );

			u16 nHashKey = (u8)pVert->x;
			FASSERT( nHashKey < m_pnHashTableSize[2] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const FGCPosS8_t *pVert, u16 nHashKey )
		{
			FASSERT( pVert );
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[2].m_nHashTableSize );

			CHashNode *pNode = m_paHashTables[2].m_apHashTableHead[nHashKey];
			FGCPosS8_t *pTestVert;
			while ( pNode )
			{
				pTestVert = (FGCPosS8_t *)pNode->m_pData;
				if (	pTestVert->x == pVert->x
					&&	pTestVert->y == pVert->y
					&&	pTestVert->z == pVert->z )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}

		// Color Hash
		FINLINE u16 GetColorHashKey( FGCColor_t *pNewColor )
		{
			FASSERT( pNewColor );

			u16 nHashKey = pNewColor->r;
			FASSERT( nHashKey < m_pnHashTableSize[3] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE u16 GetColorHashKey( u16 nNormIdx )
		{
			u16 nHashKey = (u8)(nNormIdx / 256);
			FASSERT( nHashKey < m_pnHashTableSize[3] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const FGCColor_t *pNewColor, u16 nHashKey )
		{
			FASSERT( pNewColor );
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[3].m_nHashTableSize );

			CHashNode *pNode = m_paHashTables[3].m_apHashTableHead[nHashKey];
			MLH_GCColor_t *pTestColor;
			while ( pNode )
			{
				pTestColor = (MLH_GCColor_t *)pNode->m_pData;
				if (	pTestColor->Color.r == pNewColor->r
					&&	pTestColor->Color.g == pNewColor->g
					&&	pTestColor->Color.b == pNewColor->b
					&&	pTestColor->Color.a == pNewColor->a )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}

		FINLINE s32 FindInHashTable( u16 nVBIndex, u16 nPosIdx, u16 nNormIdx, u16 nHashKey )
		{
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[3].m_nHashTableSize );

			// Note that when we're checking based on position and normal there is no
			// need to check the actual color because we can assume it to be the
			// same if the position and normal are the same
			CHashNode *pNode = m_paHashTables[3].m_apHashTableHead[nHashKey];
			MLH_GCColor_t *pTestColor;
			while ( pNode )
			{
				pTestColor = (MLH_GCColor_t *)pNode->m_pData;
				if (	pTestColor->nVBIdx == nVBIndex
                    &&	pTestColor->nNormIdx == nNormIdx
					&&	pTestColor->nPosIdx == nPosIdx )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}

		// ST Hash
		FINLINE u16 GetHashKey( FGCST16_t *pNewST )
		{
			FASSERT( pNewST );

			u16 nHashKey = (u16)(pNewST->s & 0x3ff);
			FASSERT( nHashKey < m_pnHashTableSize[4] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const FGCST16_t *pNewST, u16 nHashKey )
		{
			FASSERT( pNewST );
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[4].m_nHashTableSize );

			CHashNode *pNode = m_paHashTables[4].m_apHashTableHead[nHashKey];
			FGCST16_t *pTestST;
			while ( pNode )
			{
				pTestST = (FGCST16_t *)pNode->m_pData;
				if (	pTestST->s == pNewST->s
					&&	pTestST->t == pNewST->t )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}

		// NBT Hash
		FINLINE u16 GetHashKey( FGCNBT8_t *pNewNBT )
		{
			FASSERT( pNewNBT );

			u16 nHashKey = (u8)pNewNBT->nx;
			FASSERT( nHashKey < m_pnHashTableSize[5] && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const FGCNBT8_t *pNewNBT, u16 nHashKey )
		{
			FASSERT( pNewNBT );
			FASSERT( nHashKey >= 0 && nHashKey < m_paHashTables[5].m_nHashTableSize );

			CHashNode *pNode = m_paHashTables[5].m_apHashTableHead[nHashKey];
			FGCNBT8_t *pTestNBT;
			while ( pNode )
			{
				pTestNBT = (FGCNBT8_t *)pNode->m_pData;
				if (	pTestNBT->nx == pNewNBT->nx
					&&	pTestNBT->ny == pNewNBT->ny
					&&	pTestNBT->nz == pNewNBT->nz
					&&	pTestNBT->bx == pNewNBT->bx
					&&	pTestNBT->by == pNewNBT->by
					&&	pTestNBT->bz == pNewNBT->bz
					&&	pTestNBT->tx == pNewNBT->tx
					&&	pTestNBT->ty == pNewNBT->ty
					&&	pTestNBT->tz == pNewNBT->tz )
				{
					return pNode->m_nIndex;
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}
};

#ifdef _MMI_TARGET_PS2
// KJ Begin

//
//
//

class CPS2VertHash
{
	public:
		CHashTable	*m_apHashTables[MLHASH_MAX_PC_TABLES];
		KongVert_t	*m_pSampleHashVert[MLHASH_MAX_PC_TABLES];
		u16			m_nHashTableCount;
		u16			m_nHashTableSize;

		u32			m_nDuplicateVerts;
		u32			m_nVertsProcessed;

	public:
        CPS2VertHash( void )
		{
			u32 i;
			for ( i = 0; i < MLHASH_MAX_PC_TABLES; i++ )
			{
                m_apHashTables[i] = NULL;
				m_pSampleHashVert[i] = NULL;
			}

			m_nHashTableCount = 0;
			m_nDuplicateVerts = 0;
			m_nVertsProcessed = 0;
		}

        ~CPS2VertHash( void )
		{
            DEVPRINTF( "\nCPS2VertHash:: Percent Duplicate -  " );
			if ( m_nVertsProcessed )
			{
				DEVPRINTF( "Verts (%d): %4.2f  ", m_nVertsProcessed, ((f32)m_nDuplicateVerts / (f32)m_nVertsProcessed) * 100.f );
			}
			DEVPRINTF( "\n" );

			Clear();
		}

		FINLINE BOOL Init( void )
		{
/*
			// Default values for the PC hash table
			m_nHashTableCount = FDX8VB_TYPE_COUNT;
			m_nHashTableSize = 1024;

			m_paHashTables = new CHashTable[m_nHashTableCount];
			if ( !m_paHashTables )
			{
				return FALSE;
			}

			int i;
			for ( i = 0; i < m_nHashTableCount; i++ )
			{
				if ( !m_paHashTables[i].Init( m_nHashTableSize, 65533 ) )
				{
					return FALSE;
				}
			}
*/
			m_nHashTableSize = 1024;

			Clear();

			return TRUE;
		}

		FINLINE void Clear( void )
		{
			u32 i;
			for ( i = 0; i < m_nHashTableCount; i++ )
			{
				FASSERT( m_apHashTables[i] );
				delete m_apHashTables[i];
				m_apHashTables[i] = NULL;
				m_pSampleHashVert[i] = NULL;
			}
			FASSERT( !m_apHashTables[i] );

			m_nHashTableCount = 0;
		}

		FINLINE s16 GetTableIdx( KongVert_t *pSampleVert )
		{
			FASSERT( pSampleVert );

			s16 nTableIndex;
			for ( nTableIndex = 0; nTableIndex < m_nHashTableCount; nTableIndex++ )
			{
				if (   (pSampleVert->nNumWeights == 0) == (m_pSampleHashVert[nTableIndex]->nNumWeights == 0)
					&& pSampleVert->nBaseUVCount         == m_pSampleHashVert[nTableIndex]->nBaseUVCount
					&& pSampleVert->nLightMapUVCount     == m_pSampleHashVert[nTableIndex]->nLightMapUVCount )
				{
					return nTableIndex;
				}
			}

			return -1;
		}

//NS        FINLINE u16 AddVert( KongVert_t *pNewVert )
        FINLINE u16 AddVert( KongVert_t *pNewVert, u16 &nVBIndex  )
		{
			FASSERT( pNewVert );

			u16 nHashKey = GetHashKey( pNewVert );

			m_nVertsProcessed++;

			// See if we can put this vert in an existing table
			u32 nTableIndex;
			for ( nTableIndex = 0; nTableIndex < m_nHashTableCount; nTableIndex++ )
			{
				if (   (pNewVert->nNumWeights == 0) == (m_pSampleHashVert[nTableIndex]->nNumWeights == 0)
					&& pNewVert->nBaseUVCount         == m_pSampleHashVert[nTableIndex]->nBaseUVCount
					&& pNewVert->nLightMapUVCount     == m_pSampleHashVert[nTableIndex]->nLightMapUVCount )
				{
					break;
				}
			}

			if ( nTableIndex == m_nHashTableCount )
			{
				// Didn't find an existing hash table that will support this vertex, so create another
				if ( m_nHashTableCount == MLHASH_MAX_PC_TABLES )
				{
					FASSERT_NOW;
					return 0xffff;
				}

				m_apHashTables[m_nHashTableCount] = new CHashTable;
				if ( !m_apHashTables[m_nHashTableCount] )
				{
					return 0xffff;
				}

				if ( !m_apHashTables[m_nHashTableCount]->Init( m_nHashTableSize, 65533 ) )
				{
					delete m_apHashTables[m_nHashTableCount];
					return 0xffff;
				}
				nTableIndex = m_nHashTableCount;
				m_nHashTableCount++;

				nVBIndex = nTableIndex;
				m_pSampleHashVert[nTableIndex] = pNewVert;
				return m_apHashTables[nTableIndex]->AddData( pNewVert, nHashKey );
			}

            nVBIndex = nTableIndex;

            // See if the same vert is already in the collection
			s32 nIndex = FindInHashTable( pNewVert, nTableIndex, nHashKey );
			if ( nIndex != -1 )
			{
				m_nDuplicateVerts++;
				return (u16)nIndex;
			}

			return m_apHashTables[nTableIndex]->AddData( pNewVert, nHashKey );
		}

	protected:
		FINLINE u16 GetHashKey( KongVert_t *pVert )
		{
			FASSERT( pVert );

			u16 nOMHashTableSize = m_nHashTableSize - 1;

			f32 fX = (f32)fmath_Abs( pVert->Pos.x - (s32)pVert->Pos.x );
			u16 nHashKey = (u16)(fX * nOMHashTableSize);
			FASSERT( nHashKey < m_nHashTableSize && nHashKey >= 0 );

			return nHashKey;
		}

		FINLINE s32 FindInHashTable( const KongVert_t *pVert, u16 nTableIndex, u16 nHashKey )
		{
			FASSERT( pVert && nTableIndex >= 0 && nTableIndex < m_nHashTableCount );
			FASSERT( nHashKey >= 0 && nHashKey < m_apHashTables[nTableIndex]->m_nHashTableSize );

			u32 i;
			CHashNode *pNode = m_apHashTables[nTableIndex]->m_apHashTableHead[nHashKey];
			KongVert_t *pTestVert;
			while ( pNode )
			{
				pTestVert = (KongVert_t *)pNode->m_pData;
				if (	pTestVert->Pos == pVert->Pos
					&&	pTestVert->Norm == pVert->Norm
					&&	pTestVert->Color == pVert->Color )
				{
					for ( i = 0; i < (u16)(pTestVert->nBaseUVCount + pTestVert->nLightMapUVCount); i++ )
					{
						if ( pTestVert->aUV[i] != pVert->aUV[i] )
						{
							break;
						}
					}
					if ( i == pTestVert->nBaseUVCount + pTestVert->nLightMapUVCount )
					{
						return pNode->m_nIndex;
					}
				}

				pNode = pNode->m_pHashNext;
			}

			return -1;
		}
};




// KJ End
#endif

#endif
