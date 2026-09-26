//////////////////////////////////////////////////////////////////////////////////////
// TriPacket.h - 
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
#ifndef _TRI_PACKET_H_
#define _TRI_PACKET_H_ 1

#include "fang.h"
#include "TriCollection.h"
#include "KongDef.h"
#include "FixedData.h"

typedef struct {
	u32 nNumVerts;// how many verts are needed to make up this tri, can be less than 3 if it is part of a strip
	KongVert_t *apKongVerts[3];// ape vert pointer into the collection's ape verts
	KongTri_t *pKTri;// kong tri pointer into the collection's tri array	
} TriPacket_Tri_t;

// A tri packet is a collection of tris that have been striped.
class CTriPacket : public CTriCollection
{
public:
	CTriPacket( BOOL bGenerateStrips=TRUE );
	~CTriPacket();
	
	virtual u32 Create( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex,
						TriCollection_SearchType_e nSearchType=TRI_COLLECTION_SEARCH_TYPE_NEAREST,
						u32 nMaxVerts=0 );
	BOOL IsDataStripped() { return m_bGenerateStrips; }
	u32 GetNumPacketVerts() { return m_nNumPacketVerts; }
	KongVert_t *GetPacketVert( u32 nIndex );
	TriPacket_Tri_t *GetPacketTri( u32 nIndex );
	u32 GetNumStrips() { return m_nNumStrips; }
	u32 GetStripVertCount( u32 nIndex );

protected:
	virtual void Reset();

private:
	BOOL m_bGenerateStrips;
	u32 m_nNumPacketVerts;// how many verts are needed to represent all of the tris in the packet
	CPtrArray m_aPacketVerts;// KongVert_t * (there are m_nNumPacketVerts of these)
	CPtrArray m_aTris;// TriPacket_Tri_t * (there are m_nNumTris of these)
	u32 m_nNumStrips;// how many strips are there in the packet
	CUIntArray m_aStripCount;// what is the vert count of each strip

	void *m_pStripObject;// allocated from striper
	CFixedSizeDataOb m_FDApePtrs;// used to store the ape ptrs that are passed to the stripper

	BOOL CreateIndexTriList( void *pPrivateData, u32 (*panIndices)[3], u32 nStartingTri );
	u32 CountNumStrippedVerts( u32 nNumStrips, int *pnStripLens );
	u32 CountNumStrippedTris( u32 nNumStrips, int *pnStripLens );
	u32 GenerateStrippedPacket( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, 
								TriCollection_SearchType_e nSearchType, u32 nMaxVerts );
	u32 GenerateNonStrippedPacket( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, 
								   TriCollection_SearchType_e nSearchType, u32 nMaxVerts );
};

#endif

