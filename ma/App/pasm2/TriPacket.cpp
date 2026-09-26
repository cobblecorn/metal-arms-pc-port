//////////////////////////////////////////////////////////////////////////////////////
// TriPacket.cpp - 
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
// 05/04/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "TriPacket.h"
#include "tc.h"// striping code

#define TRI_PACKET_CREATE_STRIPS	1// set to 1 to create strips of tris

typedef struct {
	u32 *pnFDIndex;// the first of 3 indices into our FD array used by the stripper
	KongTri_t *pKTri;
} _StripperTri_t;

typedef struct {
	KongVert_t *pKongVert;
} _VertLookup_t;

static _StripperTri_t *_FindStripTri( _StripperTri_t *pStripTris, u32 nNumTris, u32 *pnVerts );

CTriPacket::CTriPacket( BOOL bGenerateStrips/*=TRUE*/ ) {
#if TRI_PACKET_CREATE_STRIPS
	m_bGenerateStrips = bGenerateStrips;
#else
	m_bGenerateStrips = FALSE;
#endif
	m_pStripObject = NULL;
	m_aTris.RemoveAll();
	Reset();
}

CTriPacket::~CTriPacket() {
	Reset();
}

// returns the number of tris in the packet
// returns 0 if there was a problem creating the tri packet
// if nMaxVerts is 0, then all kong tris will be inserted in the collection
u32 CTriPacket::Create( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex,
						TriCollection_SearchType_e nSearchType/*=TRI_COLLECTION_SEARCH_TYPE_NEAREST*/,
						u32 nMaxVerts/*=0*/ ) {

	if( m_bGenerateStrips ) {
		return GenerateStrippedPacket( rKongTris, uKongTris, uStartingKTriIndex, nSearchType, nMaxVerts );
	} else {
		return GenerateNonStrippedPacket( rKongTris, uKongTris, uStartingKTriIndex, nSearchType, nMaxVerts );
	}
	return 0;
}

KongVert_t *CTriPacket::GetPacketVert( u32 nIndex ) {
	if( nIndex >= m_nNumPacketVerts ) {
		return NULL;
	}
	return (KongVert_t *)m_aPacketVerts[nIndex];
}

TriPacket_Tri_t *CTriPacket::GetPacketTri( u32 nIndex ) {
	if( nIndex >= m_nNumTris ) {
		return NULL;
	}
	return (TriPacket_Tri_t *)m_aTris[nIndex];
}

u32 CTriPacket::GetStripVertCount( u32 nIndex ) {
	if( nIndex >= m_nNumStrips ) {
		return 0;
	}
	return m_aStripCount[nIndex];
}

void CTriPacket::Reset() {
	s32 i;
		
	m_nNumPacketVerts = 0;
	for( i=0; i < m_aTris.GetSize(); i++ ) {
		delete (TriPacket_Tri_t *)m_aTris[i];
	}
	m_aTris.RemoveAll();

	m_aPacketVerts.RemoveAll();

	if( m_pStripObject ) {
		actcDelete( (ACTCData *)m_pStripObject );
		m_pStripObject = NULL;
	}
	m_FDApePtrs.Free();

	m_nNumStrips = 0;
	m_aStripCount.RemoveAll();

	// call the base class
	CTriCollection::Reset();
}

// inserts all tris currently in our 
BOOL CTriPacket::CreateIndexTriList( void *pPrivateData, u32 (*panIndices)[3], u32 nStartingTri ) {
	u32 nTriIndex, j, nVertIndex;
	_StripperTri_t *pStripTri = (_StripperTri_t *)pPrivateData;
	s32 nIndex;
	_VertLookup_t VertLookup;
		
	for( nTriIndex=nStartingTri, nVertIndex=(nStartingTri * 3); nTriIndex < m_nNumTris; nTriIndex++ ) {
		pStripTri[nTriIndex].pnFDIndex = panIndices[nTriIndex];
		pStripTri[nTriIndex].pKTri = (KongTri_t *)m_TriList[nTriIndex];

		for( j=0; j < 3; j++ ) {
			VertLookup.pKongVert = (KongVert_t *)m_VertList[ nVertIndex++ ];
			nIndex = m_FDApePtrs.InsertObIntoList( (u8 *)&VertLookup, TRUE );
			if( nIndex < 0 ) {
				return FALSE;
			}
			panIndices[nTriIndex][j] = nIndex;
		}
	}
	return TRUE;		
}

u32 CTriPacket::CountNumStrippedVerts( u32 nNumStrips, int *pnStripLens ) {
	u32 i, nStrippedVerts;
	
	nStrippedVerts = 0;
	for( i=0; i < nNumStrips; i++ ) {
		nStrippedVerts += pnStripLens[i];
	}
	return nStrippedVerts;
}

u32 CTriPacket::CountNumStrippedTris( u32 nNumStrips, int *pnStripLens ) {
	u32 i, nStrippedTris;
	
	nStrippedTris = 0;
	for( i=0; i < nNumStrips; i++ ) {
		nStrippedTris += (pnStripLens[i] - 2);
	}
	return nStrippedTris;
}

static _StripperTri_t *_FindStripTri( _StripperTri_t *pStripTris, u32 nNumTris, u32 *pnVerts ) {
	u32 i, j;
	
	for( i=0; i < nNumTris; i++ ) {

		for( j=0; j < 3; j++ ) {
			if( pStripTris[i].pnFDIndex[j] == pnVerts[0] ) {
				continue;
			}
			if( pStripTris[i].pnFDIndex[j] == pnVerts[1] ) {
				continue;
			}
			if( pStripTris[i].pnFDIndex[j] == pnVerts[2] ) {
				continue;
			}
			// this isn't the vert, keep looking
			break;
		}
		if( j == 3 ) {
			return &pStripTris[i];
		}
	}

	FASSERT_NOW;
	return NULL;
}

u32 CTriPacket::GenerateStrippedPacket( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, 
									    TriCollection_SearchType_e nSearchType, u32 nMaxVerts ) {
	u32 i, nMaxVertIndices, nMaxStrippedTris, nStripTriCount;
	u8 *pAllocatedMemory, *pTemp;
	TriPacket_Tri_t *pTri;
	_StripperTri_t *pStripTri, *pLookupTri;
	u32 (*pnIndices)[3];
	ACTCData *pTC;
	int *pnPTypes, *pnPLens, nPCount, nAdditionalVerts;
	u32 *pnPVerts, nStrippedVerts, nNumNewTris, nOldTriCount, nTrisToAdd, nStrip, nNumTris;
	_VertLookup_t *pV;

	Reset();

	// a tri collection of nMaxVerts
	if( !CTriCollection::Create( rKongTris, uKongTris, uStartingKTriIndex, nSearchType, nMaxVerts ) ) {
		// could not create the basic collection
		Reset();
		return 0;
	}
	// set our max vars (the most indices and tris that this packet could possible hold with perfect stripping)
	nMaxVertIndices = (nMaxVerts) ? (nMaxVerts-2)*3 : (uKongTris * 3);
	nMaxStrippedTris = nMaxVertIndices - 2;

	// create a strip object
	m_pStripObject = actcNew();
	if( !m_pStripObject ) {
		// could not init the stripper
		Reset();
		return 0;
	}
	pTC = (ACTCData *)m_pStripObject;
	// set some stripper settings
	actcParami( pTC, ACTC_OUT_MIN_FAN_VERTS, INT_MAX );
	actcParami( pTC, ACTC_OUT_HONOR_WINDING, ACTC_FALSE );
	actcParami( pTC, ACTC_OUT_MAX_PRIM_VERTS, INT_MAX );
	actcParamu( pTC, ACTC_IN_MIN_VERT, 0 );
//	actcParamu( pTC, ACTC_IN_MAX_VERT, nMaxVertIndices );// causes memory leaks???
	actcParami( pTC, ACTC_IN_MAX_EDGE_SHARING, 2 );

	// allocate a fixed data array to hold our ape verts (as u32s)	
	if( !m_FDApePtrs.Allocate( sizeof( _VertLookup_t ), sizeof( _VertLookup_t ) * nMaxVertIndices ) ) {
		// could not allocate our worst case index list
		Reset();
		return 0;
	}

	// allocate enough working memory for stripping
	pAllocatedMemory = (u8 *)fang_Malloc( (sizeof( _StripperTri_t ) * nMaxStrippedTris) + // pStripTri
										  (sizeof( u32 ) * 3 * nMaxStrippedTris) + // pnIndices
										  (sizeof( int ) * nMaxStrippedTris) + // pnPTypes
										  (sizeof( int ) * nMaxStrippedTris) + // pnLens
										  (sizeof( u32 ) * nMaxVertIndices), // pnVerts
										  4 );
	if( !pAllocatedMemory ) {
		// could not allocate for our worst case tri count
		Reset();
		return 0;
	}

	// fixup our pointers
	pTemp = pAllocatedMemory;
	pStripTri = (_StripperTri_t *)pTemp;
	pTemp += ( sizeof( _StripperTri_t ) * nMaxStrippedTris );

	pnIndices = ( u32(*)[3] )pTemp;
	pTemp += ( sizeof( u32 ) * 3 * nMaxStrippedTris );
	
	pnPTypes = (int *)pTemp;
	pTemp += ( sizeof( int ) * nMaxStrippedTris );
	
	pnPLens = (int *)pTemp;
	pTemp += ( sizeof( int ) * nMaxStrippedTris );
	
	pnPVerts = (u32 *)pTemp;

	// create an index list from the tris that are in our collection
	if( !CreateIndexTriList( pStripTri, pnIndices, 0 ) ) {
		// could not create our index vert list
		fang_Free( pAllocatedMemory );
		Reset();
		return 0;
	}

	// strip our tris
	nPCount = actcTrianglesToPrimitives( pTC, m_nNumTris, (unsigned int(*)[3])pnIndices, pnPTypes, pnPLens, (unsigned int *)pnPVerts, INT_MAX );
    if( nPCount < 0 ) {
		// could not strip for some reason, fail
		fang_Free( pAllocatedMemory );
		Reset();
		return 0;
    }

	nStrippedVerts = CountNumStrippedVerts( nPCount, pnPLens );

	// if we didn't add all tris to our collection, see if we can get more tris into this packet
	if( nMaxVerts ) {		
		// figure out how many more verts we can stuff into this packet
//		nStrippedVerts = CountNumStrippedVerts( nPCount, pnPLens );
		nAdditionalVerts = nMaxVerts - nStrippedVerts;
		while( nAdditionalVerts > 0 ) {
			// cache the current num of tris in the collection
			nOldTriCount = m_nNumTris;

			// how many tris should we add
			nTrisToAdd = nAdditionalVerts / 3;
			if( !nTrisToAdd ) {
				// make sure that we are adding at least 1 tri, even if it goes over, we will handle that later
				nTrisToAdd = 1;
			}

			// add more tris to the collection
			nNumNewTris = CTriCollection::AddMoreTris( rKongTris, uKongTris, uStartingKTriIndex, nTrisToAdd*3 );
			if( !nNumNewTris ) {
				// unable to add any more tris to the collection, no need to restrip
				break;
			}
			// add our new tris to our index lists
			if( !CreateIndexTriList( pStripTri, pnIndices, nOldTriCount ) ) {
				// we were unable to update our tri list, stick to the original packet, remove the new tris
				for( i=0; i < nNumNewTris; i++ ) {
					CTriCollection::RemoveLastTri();
				}
				CTriCollection::RecalculateBoundingInfo();
				break;
			}
			// restrip our tris
			nPCount = actcTrianglesToPrimitives( pTC, m_nNumTris, (unsigned int(*)[3])pnIndices, pnPTypes, pnPLens, (unsigned int *)pnPVerts, INT_MAX );
			if( nPCount < 0 ) {
				fang_Free( pAllocatedMemory );
				Reset();
				return 0;
			}
			// figure out how many more verts we can stuff into this packet
			nStrippedVerts = CountNumStrippedVerts( nPCount, pnPLens );
			nAdditionalVerts = nMaxVerts - nStrippedVerts;
			if( nAdditionalVerts < 0 ) {
				// we added too many tris, remove enough to get back in acceptable range and restrip for one last time
				while( nAdditionalVerts < 0 ) {
					CTriCollection::RemoveLastTri();
					nPCount = actcTrianglesToPrimitives( pTC, m_nNumTris, (unsigned int(*)[3])pnIndices, pnPTypes, pnPLens, (unsigned int *)pnPVerts, INT_MAX );
					if( nPCount < 0 ) {
						fang_Free( pAllocatedMemory );
						Reset();
						return 0;
					}
					nStrippedVerts = CountNumStrippedVerts( nPCount, pnPLens );
					nAdditionalVerts = nMaxVerts - nStrippedVerts;
					
					FASSERT( CountNumStrippedTris( nPCount, pnPLens ) == m_nNumTris );
				}
				FASSERT( nStrippedVerts <= nMaxVerts );
				CTriCollection::RecalculateBoundingInfo();
				break;
			}
		}
	}

	nStripTriCount = 0;
	m_nNumPacketVerts = nStrippedVerts;
	m_nNumStrips = nPCount;
	// walk the strips
	for( nStrip=0; nStrip < m_nNumStrips; nStrip++ ) {
		FASSERT( pnPTypes[nStrip] == ACTC_PRIM_STRIP );
		
		nNumTris = pnPLens[nStrip] - 2;

		nStripTriCount += nNumTris;

		// walk the tris in the strip
		for( i=0; i < nNumTris; i++ ) {
			// find the stripper tri that uses the 3 indices pointed to by pnPVerts[i] (the i'th tri of this strip)
			pLookupTri = _FindStripTri( pStripTri, m_nNumTris, &pnPVerts[i] );

			// create our tri packet tris from our stripped tri
			pTri = new TriPacket_Tri_t;
			if( !pTri ) {
				// could not allocate a tri
				fang_Free( pAllocatedMemory );
				Reset();
				return 0;
			}
			// fill in our tri
			if( i > 0 ) {
				// all tris after the 1st one only use 1 vert
				pTri->nNumVerts = 1;
				pV = (_VertLookup_t *)m_FDApePtrs.GetObPtr( pnPVerts[i+2] );
				pTri->apKongVerts[0] = pV->pKongVert;
				
				m_aPacketVerts.Add( pTri->apKongVerts[0] );
			} else {
				// use all 3 verts for the 1st tri
				pTri->nNumVerts = 3;
				pV = (_VertLookup_t *)m_FDApePtrs.GetObPtr( pnPVerts[i] );
				pTri->apKongVerts[0] = pV->pKongVert;
				pV = (_VertLookup_t *)m_FDApePtrs.GetObPtr( pnPVerts[i+1] );
				pTri->apKongVerts[1] = pV->pKongVert;
				pV = (_VertLookup_t *)m_FDApePtrs.GetObPtr( pnPVerts[i+2] );
				pTri->apKongVerts[2] = pV->pKongVert;
				
				m_aPacketVerts.Add( pTri->apKongVerts[0] );
				m_aPacketVerts.Add( pTri->apKongVerts[1] );
				m_aPacketVerts.Add( pTri->apKongVerts[2] );
			}
			pTri->pKTri = pLookupTri->pKTri;
			m_aTris.Add( pTri );
		}
		// move the stripped vert pointer
		pnPVerts += pnPLens[nStrip];
		// record the size of this strip
		m_aStripCount.Add( pnPLens[nStrip] );
	}
	
	FASSERT( m_nNumTris == nStripTriCount );

#if 0
	////////////////////////////////////////////////////////////////////
	// THIS ACTUALLY HURTS STRIPPING BY STEALING TRIS FROM FUTURE STRIPS
	////////////////////////////////////////////////////////////////////
	u32 nOldVertCount;

	// if we can get more tris into this packet go ahead and add them unstripped at the end
	if( nMaxVerts ) {
		nOldTriCount = m_nNumTris;
		nOldVertCount = m_nNumVerts;

		nTrisToAdd = nAdditionalVerts/3;
		nNumNewTris = CTriCollection::AddMoreTris( rKongTris, uKongTris, uStartingKTriIndex, nTrisToAdd*3 );
		
		for( i=0; i < nNumNewTris; i++ ) {
			pTri = new TriPacket_Tri_t;
			if( !pTri ) {
				// could not allocate a tri
				fang_Free( pAllocatedMemory );
				Reset();
				return 0;
			}
			m_nNumStrips++;
			m_aStripCount.Add( 3 );
			// fill in our tri
			pTri->nNumVerts = 3;
			pTri->apKongVerts[0] = (KongVert_t *)m_VertList[ nOldVertCount++ ];
			pTri->apKongVerts[1] = (KongVert_t *)m_VertList[ nOldVertCount++ ];
			pTri->apKongVerts[2] = (KongVert_t *)m_VertList[ nOldVertCount++ ];
			pTri->pKTri = (KongTri_t *)m_TriList[ nOldTriCount++ ];
			// add it to our list
			m_aTris.Add( pTri );
			m_aPacketVerts.Add( pTri->apKongVerts[0] );
			m_aPacketVerts.Add( pTri->apKongVerts[1] );
			m_aPacketVerts.Add( pTri->apKongVerts[2] );

			m_nNumPacketVerts += 3;	
		}
		FASSERT( m_nNumPacketVerts <= nMaxVerts );
	}
#endif
	// free our temp stripping memory
	fang_Free( pAllocatedMemory );

	actcDelete( pTC );
	m_pStripObject = NULL;

	m_FDApePtrs.Free();	

	// sanity check...
	FASSERT( m_aTris.GetSize() == (int)m_nNumTris );

	return m_nNumTris;
}

u32 CTriPacket::GenerateNonStrippedPacket( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, 
										   TriCollection_SearchType_e nSearchType, u32 nMaxVerts ) {
	u32 i, nVertIndex;
	TriPacket_Tri_t *pTri;

	Reset();

	// for now just create a simple collection and setup our vars
	if( !CTriCollection::Create( rKongTris, uKongTris, uStartingKTriIndex, nSearchType, nMaxVerts ) ) {
		// could not create the basic collection
		Reset();
		return 0;
	}
	m_nNumPacketVerts = m_nNumVerts;
	m_nNumStrips = m_nNumTris;
	for( i=0, nVertIndex=0; i < m_nNumTris; i++ ) {
		pTri = new TriPacket_Tri_t;
		if( !pTri ) {
			// could not allocate a tri
			Reset();
			return 0;
		}
		// fill in our tri
		pTri->nNumVerts = 3;
		pTri->apKongVerts[0] = (KongVert_t *)m_VertList[ nVertIndex++ ];
		pTri->apKongVerts[1] = (KongVert_t *)m_VertList[ nVertIndex++ ];
		pTri->apKongVerts[2] = (KongVert_t *)m_VertList[ nVertIndex++ ];

		pTri->pKTri = (KongTri_t *)m_TriList[i];
		// add it to our list
		m_aTris.Add( pTri );
		m_aPacketVerts.Add( pTri->apKongVerts[0] );
		m_aPacketVerts.Add( pTri->apKongVerts[1] );
		m_aPacketVerts.Add( pTri->apKongVerts[2] );
		// each "strip" has 3 tris
		m_aStripCount.Add( 3 );
	}

	// sanity check...
	FASSERT( m_aTris.GetSize() == (int)m_nNumTris );

	return m_nNumTris;
}
