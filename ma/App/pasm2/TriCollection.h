//////////////////////////////////////////////////////////////////////////////////////
// TriCollection.h - used to gather up tris into collections
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
// 05/01/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _TRI_COLLECTION_H_
#define _TRI_COLLECTION_H_ 1

#include "fang.h"
#include "KongDef.h"

typedef enum {
	TRI_COLLECTION_SEARCH_TYPE_NEAREST = 0,// this method adds the nearest tris to one another
	TRI_COLLECTION_SEARCH_TYPE_CIRCULAR,// this method adds in a circular attached net 
	TRI_COLLECTION_SEARCH_TYPE_LONGEST,// this method adds tris attached to the last tri

	TRI_COLLECTION_SEARCH_TYPE_COUNT
} TriCollection_SearchType_e;

// A tri collection is a group of kong triangles that have been selected from a larger group
// using a particular search type.
// This collection is just a raw collection, it is NOT a tri strip, fan, or anything other 
// sophisticated grouping.
class CTriCollection
{
public:
	CTriCollection();
	~CTriCollection();
	
	virtual u32 Create( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, 
						TriCollection_SearchType_e nSearchType=TRI_COLLECTION_SEARCH_TYPE_NEAREST,
						u32 nMaxVerts=0 );
	u32 AddMoreTris( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumVertsToAdd );

	// helper functions:
	virtual u32 GetNumTris() { return m_nNumTris; }
	virtual KongTri_t *GetTri( u32 nIndex );
	virtual u32 GetNumVerts() { return m_nNumVerts; }
	virtual KongVert_t *GetVert( u32 nIndex );

	TriCollection_SearchType_e GetSearchType() { return m_nSearchType; }
	void RecalculateBoundingInfo();
	void GetBoundingSphere( CFSphere &rSphere ) { rSphere = m_BSphere; }
	void GetMinMax( CFVec3A &rMin, CFVec3A &rMax ) { rMin = m_Min; rMax = m_Max; }	

protected:
	virtual void Reset();
	void RemoveLastTri();

	u32 m_nNumTris;
	CPtrArray m_TriList;// KongTri_t *
	u32 m_nNumVerts;
	CPtrArray m_VertList;// KongVert_t *

	TriCollection_SearchType_e m_nSearchType;
	BOOL m_bNeedToComputeDistance;
	
	CFSphere m_BSphere;
	CFVec3 m_Min;
	CFVec3 m_Max;

private:
	u32 NearestTriSearch( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumTris );
	u32 CircularTriSearch( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumTris );
	u32 LongestTriSearch( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumTris );

	KongTri_t *FindTriWithSmallestRadius( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex );
	KongTri_t *FindTriWithMostConnectedEdges( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex );
	KongTri_t *FindBestTriToStartAStrip( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex );
	KongTri_t *FindClosestTriToCenter( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex );
	void AddTri( KongTri_t *pKTri );

	// vars used to remove a tri from the collection
	BOOL m_bLastCacheValid;
	CFSphere m_LastBSphere;
	CFVec3 m_LastMin;
	CFVec3 m_LastMax;
};

#endif

