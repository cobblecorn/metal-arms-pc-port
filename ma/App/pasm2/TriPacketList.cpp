//////////////////////////////////////////////////////////////////////////////////////
// TriPacketList.cpp - 
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
// 05/03/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "TriPacketList.h"


CTriPacketList::CTriPacketList() {
	m_nNumPackets = 0;
	m_aPackets.RemoveAll();
	Reset();
}

CTriPacketList::~CTriPacketList() {
	Reset();
}

// returns the number of packets, 0 if there was an error
// if nMaxVerts is 0, then all kong tris will be inserted in the collection
u32 CTriPacketList::Create( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, BOOL bGenerateStrips, 
						    TriCollection_SearchType_e nSearchType/*=TRI_COLLECTION_SEARCH_TYPE_NEAREST*/,
							u32 nMaxVertsPerPacket/*=0*/ ) {
	u32 nTriCount, nTrisInPacket;
	CTriPacket *pTriPacket;
	u32 nRemainingVerts;

	Reset();
	
	FASSERT( nSearchType < TRI_COLLECTION_SEARCH_TYPE_COUNT );
	
	nTriCount = 0;
	while( nTriCount < uKongTris ) {
		// allocate a new packet
		pTriPacket = new CTriPacket( bGenerateStrips );
		if( !pTriPacket ) {
			Reset();
			return 0;
		}
		// add our packet to our list
		m_nNumPackets++;
		m_aPackets.Add( pTriPacket );

		if( nMaxVertsPerPacket ) {
			// we are limiting the number of verts per packet,
			// make sure that the limit isn't higher than the number
			// of available verts
			nRemainingVerts = (uKongTris - nTriCount) * 3;
			if( nRemainingVerts < nMaxVertsPerPacket ) {
				nMaxVertsPerPacket = nRemainingVerts;
			}
		}
		// fill the packet with tris
		nTrisInPacket = pTriPacket->Create( rKongTris, uKongTris, uStartingKTriIndex, nSearchType, nMaxVertsPerPacket );
		if( !nTrisInPacket ) {
			// no tris were added to a packet, error
			Reset();
			return 0;
		}
		nTriCount += nTrisInPacket;
	}

	return m_nNumPackets;
}

CTriPacket *CTriPacketList::GetTriPacket( u32 nIndex ) {
	if( nIndex >= m_nNumPackets ) {
		return NULL;
	}
	return (CTriPacket *)m_aPackets[nIndex];
}

void CTriPacketList::Reset() {
	u32 i;

	for( i=0; i < m_nNumPackets; i++ ) {
		delete (CTriPacket *)m_aPackets[i];
	}
	m_nNumPackets = 0;
	m_aPackets.RemoveAll();	
}
