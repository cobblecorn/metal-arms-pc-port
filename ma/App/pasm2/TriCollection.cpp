//////////////////////////////////////////////////////////////////////////////////////
// TriCollection.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "TriCollection.h"


CTriCollection::CTriCollection() {
	CTriCollection::Reset();
}

CTriCollection::~CTriCollection() {
	CTriCollection::Reset();
}

// returns the number of tris added to the collection
// if nMaxVerts is 0, then all kong tris will be inserted in the collection
u32 CTriCollection::Create( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, 
						    TriCollection_SearchType_e nSearchType/*=TRI_COLLECTION_SEARCH_TYPE_NEAREST*/,
							u32 nMaxVerts/*=0*/ ) {
	u32 nTrisAdded, nMaxTris;
	
	nMaxTris = ( nMaxVerts ) ? (nMaxVerts/3) : uKongTris;

	CTriCollection::Reset();

	FASSERT( nSearchType < TRI_COLLECTION_SEARCH_TYPE_COUNT );
	m_nSearchType = nSearchType;
	switch( m_nSearchType ) {
	case TRI_COLLECTION_SEARCH_TYPE_NEAREST:
		nTrisAdded = NearestTriSearch( rKongTris, uKongTris, uStartingKTriIndex, nMaxTris );
		break;
	case TRI_COLLECTION_SEARCH_TYPE_CIRCULAR:
		nTrisAdded = CircularTriSearch( rKongTris, uKongTris, uStartingKTriIndex, nMaxTris );
		break;
	case TRI_COLLECTION_SEARCH_TYPE_LONGEST:
		nTrisAdded = LongestTriSearch( rKongTris, uKongTris, uStartingKTriIndex, nMaxTris );
		break;
	default:
		nTrisAdded = 0;
	}
	// calculate the square root of the bounding sphere radius (all internal calculations are done with R2)
	m_BSphere.m_fRadius = fmath_Sqrt( m_BSphere.m_fRadius );

	return nTrisAdded;
}

// returns the number of tris added to the collection
u32 CTriCollection::AddMoreTris( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumVertsToAdd ) {
	u32 nTrisAdded, nNumTris;

	if( nNumVertsToAdd < 3 ) {
		return 0;
	}
	nNumTris = nNumVertsToAdd/3;// our add function adds whole, unstripped tris

	// square our bounding sphere's radius (so that we can do D2 tests)
	m_BSphere.m_fRadius *= m_BSphere.m_fRadius;

	switch( m_nSearchType ) {
	case TRI_COLLECTION_SEARCH_TYPE_NEAREST:
		nTrisAdded = NearestTriSearch( rKongTris, uKongTris, uStartingKTriIndex, nNumTris );
		break;
	case TRI_COLLECTION_SEARCH_TYPE_CIRCULAR:
		nTrisAdded = CircularTriSearch( rKongTris, uKongTris, uStartingKTriIndex, nNumTris );
		break;
	case TRI_COLLECTION_SEARCH_TYPE_LONGEST:
		nTrisAdded = LongestTriSearch( rKongTris, uKongTris, uStartingKTriIndex, nNumTris );
		break;
	}
	// calculate the square root of the bounding sphere radius
	m_BSphere.m_fRadius = fmath_Sqrt( m_BSphere.m_fRadius );

	return nTrisAdded;
}

KongTri_t *CTriCollection::GetTri( u32 nIndex ) {
	if( nIndex >= m_nNumTris ) {
		return NULL;
	}
	return (KongTri_t *)m_TriList[nIndex];
}

KongVert_t *CTriCollection::GetVert( u32 nIndex ) {
	if( nIndex >= m_nNumVerts ) {
		return NULL;
	}
	return (KongVert_t *)m_VertList[nIndex];
}

void CTriCollection::Reset() {
	m_nNumTris = 0;
	m_TriList.RemoveAll();
	m_nNumVerts = 0;
	m_VertList.RemoveAll();
	m_nSearchType = TRI_COLLECTION_SEARCH_TYPE_NEAREST;
	m_BSphere.Zero();
	m_Min.Zero();
	m_Max.Zero();
	m_bNeedToComputeDistance = TRUE;

	m_bLastCacheValid = FALSE;
	m_LastBSphere.Zero();
	m_LastMin.Zero();
	m_LastMax.Zero();
}

// returns the number of tris added to the collection
u32 CTriCollection::NearestTriSearch( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumTris ) {
	u32 i, nTrisBefore;
	KongTri_t *pKTri;
	
	// cache the current number of tris in the collection
	nTrisBefore = m_nNumTris;

	for( i=0; i < nNumTris; i++ ) {
		// grab the KTri to add to the list
		if( m_nNumTris > 0 ) {
			pKTri = FindClosestTriToCenter( rKongTris, uKongTris, uStartingKTriIndex );
		} else {
			pKTri = FindTriWithSmallestRadius( rKongTris, uKongTris, uStartingKTriIndex );
		}

		// add our tri to the collection
		if( pKTri ) {
			AddTri( pKTri );
		} else {
			// couldn't find a tri to add
			break;
		}
	}
	return (m_nNumTris - nTrisBefore);
}

// returns the number of tris added to the collection
u32 CTriCollection::CircularTriSearch( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumTris ) {
	u32 i, j, nTri, nTrisBefore;
	KongTri_t *pKTri, *pLastTri, *pCurKTri;
	
	// cache the current number of tris in the collection
	nTrisBefore = m_nNumTris;

	for( nTri=0; nTri < nNumTris; nTri++ ) {
		// grab the KTri to add to the list
		if( m_nNumTris > 0 ) {
			// look for (in order):
			// tri that shares an edge with oldest tri added
			// tri that shares an edge with any added tri( going from oldest to last added)
			// tri with most connected edges
			// tri that will expand the sphere the least
			pKTri = NULL;
			for( i=0; i < m_nNumTris; i++ ) {
				pLastTri = (KongTri_t *)m_TriList[i];
				for( j=0; j < pLastTri->nNumEdgeTris; j++ ) {
					pCurKTri = pLastTri->paEdgeTris[j];
					if( !pCurKTri->bTriAdded ) {
						pKTri = pCurKTri;
						break;
					}
				}
				if( pKTri ) {
					break;
				}
			}
			if( !pKTri ) {
				// add the most connected edges
				pKTri = FindTriWithMostConnectedEdges( rKongTris, uKongTris, uStartingKTriIndex );
			}
		} else {
			pKTri = FindTriWithMostConnectedEdges( rKongTris, uKongTris, uStartingKTriIndex );
			if( !pKTri ) {
				pKTri = FindTriWithSmallestRadius( rKongTris, uKongTris, uStartingKTriIndex );	
			}
		}
		if( !pKTri ) {
			// unable to find a connected tri, find the one who expands the collection the least
			pKTri = FindClosestTriToCenter( rKongTris, uKongTris, uStartingKTriIndex );
		}

		// add our tri to the collection
		if( pKTri ) {
			AddTri( pKTri );
		} else {
			// couldn't find a tri to add
			break;
		}
	}
	return (m_nNumTris - nTrisBefore);
}

// returns the number of tris added to the collection
u32 CTriCollection::LongestTriSearch( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, u32 nNumTris ) {
	s32 i;
	u32 j, nTri, nTrisBefore;
	KongTri_t *pKTri, *pLastTri, *pCurKTri;
	
	// cache the current number of tris in the collection
	nTrisBefore = m_nNumTris;

	for( nTri=0; nTri < nNumTris; nTri++ ) {
		// grab the KTri to add to the list
		if( m_nNumTris > 0 ) {
			// look for (in order):
			// tri that shares an edge with last added tri
			// tri that shares an edge with any added tri
			// nearest tri to center of the bounding sphere
			pKTri = NULL;
			for( i=(m_nNumTris-1); i >= 0; i-- ) {
				pLastTri = (KongTri_t *)m_TriList[i];
				for( j=0; j < pLastTri->nNumEdgeTris; j++ ) {
					pCurKTri = pLastTri->paEdgeTris[j];
					if( !pCurKTri->bTriAdded ) {
						if( !pKTri ) {
							pKTri = pCurKTri;
						} else {
#if 1
							// see if pCurKTri has more connected edges than pKTri, we
							// want to pick the tri that has the most connected edges 
							// because we will be more likely to extend our strip next time
							if( pCurKTri->nNumEdgeTris > pKTri->nNumEdgeTris ) {
								pKTri = pCurKTri;
							}
#else
							// see if pCurKTri has less connected edges than pKTri, we
							// want to pick the tri that has the fewest connected edges 
							// yet is still connected.
							if( pKTri->nNumEdgeTris > 0 ) {
								// the current ktri is connect, do we have fewer connections?
								if( pCurKTri->nNumEdgeTris > 0 &&
									pCurKTri->nNumEdgeTris < pKTri->nNumEdgeTris ) {
									pKTri = pCurKTri;
								}
							} else {
								// the current ktri is not connected, if we are connected, use the new ktri
								if( pCurKTri->nNumEdgeTris > 0 ) {
									pKTri = pCurKTri;
								}
							}
#endif
						}				
					}
				}
				if( pKTri ) {
					break;
				}
			}
			if( !pKTri ) {
				// add the nearest tri to the center of the bounding sphere
				pKTri = FindClosestTriToCenter( rKongTris, uKongTris, uStartingKTriIndex );
			}
		} else {
			pKTri = FindBestTriToStartAStrip( rKongTris, uKongTris, uStartingKTriIndex );//FindTriWithMostConnectedEdges
			if( !pKTri ) {
				// couldn't find a connected tri, start with the smallest bounding sphere tri
				pKTri = FindTriWithSmallestRadius( rKongTris, uKongTris, uStartingKTriIndex );
			}
		}

		// add our tri to the collection
		if( pKTri ) {
			AddTri( pKTri );
		} else {
			// couldn't find a tri to add
			break;
		}
	}
	return (m_nNumTris - nTrisBefore);
}

// searches a kong tri list for the tri with the smallest bounding sphere
KongTri_t *CTriCollection::FindTriWithSmallestRadius( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex ) {
	u32 i, nLastTriIndex;
	KongTri_t *pKTri, *pBestKTri;
	f32 fSmallestRadius;

	// find the tri with the smallest bounding sphere
	nLastTriIndex = uStartingKTriIndex + uKongTris;
	fSmallestRadius = -1.0f;
	for( i=uStartingKTriIndex; i < nLastTriIndex; i++ ) {
		pKTri = (KongTri_t *)rKongTris[i];
		if( !pKTri->bTriAdded ) {
			if( fSmallestRadius > 0.0f ) {
				if( pKTri->BoundSphere.m_fRadius < fSmallestRadius ) {
					// this is smaller than the last one
					fSmallestRadius = pKTri->BoundSphere.m_fRadius;
					pBestKTri = pKTri;
				}
			} else {
				// this is the first available
				fSmallestRadius = pKTri->BoundSphere.m_fRadius;
				pBestKTri = pKTri;
			}
		}
	}
	
	// pBestKTri can be NULL
	return pBestKTri;
}

// searches a kong tri list for the tri with the most connected edges (max 3)
// CAN RETURN NULL IF NO TRI IS CONNECTED TO ANYONE
KongTri_t *CTriCollection::FindTriWithMostConnectedEdges( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex ) {
	u32 i, nLastTriIndex;
	KongTri_t *pKTri, *pBestKTri = NULL;

	// Pick the unused tri with the most edges
	nLastTriIndex = uStartingKTriIndex + uKongTris;
	for( i=uStartingKTriIndex; i < nLastTriIndex; i++ ) {
		pKTri = (KongTri_t *)rKongTris[i];
		if( !pKTri->bTriAdded ) {
			if( pBestKTri != NULL ) {
				if( pBestKTri->nNumEdgeTris < pKTri->nNumEdgeTris ) {
					// a new best tri, one with more connected edges
					pBestKTri = pKTri;
				}
			} else if( pKTri->nNumEdgeTris > 0 ) {
				// the first tri connected on at least 1 edge
				pBestKTri = pKTri;
			}
			if( pBestKTri &&
				pBestKTri->nNumEdgeTris == 3 ) {
				break;
			}
		}
	}
		
	// pBestKTri can be NULL
	return pBestKTri;
}

// searches a kong tri list for a tri with the fewest (yet over 0) connected edges
// CAN RETURN NULL IF NO TRI IS CONNECTED TO ANYONE
KongTri_t *CTriCollection::FindBestTriToStartAStrip( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex ) {
	u32 i, nLastTriIndex;
	KongTri_t *pKTri, *pBestKTri = NULL;

	// Pick the unused tri with the least connected edges
	nLastTriIndex = uStartingKTriIndex + uKongTris;
	for( i=uStartingKTriIndex; i < nLastTriIndex; i++ ) {
		pKTri = (KongTri_t *)rKongTris[i];
		if( !pKTri->bTriAdded ) {
			if( pBestKTri != NULL ) {
				if( pKTri->nNumEdgeTris > 0 && 
					pBestKTri->nNumEdgeTris > pKTri->nNumEdgeTris ) {
					// a new best tri, one with less connected edges
					pBestKTri = pKTri;
				}
			} else if( pKTri->nNumEdgeTris > 0 ) {
				// the first tri connected on at least 1 edge
				pBestKTri = pKTri;
			}
			if( pBestKTri && 
				pBestKTri->nNumEdgeTris == 1 ) {
				break;
			}
		}
	}
		
	// pBestKTri can be NULL
	return pBestKTri;
}


// find the nearest tri to the center of our bounding sphere
KongTri_t *CTriCollection::FindClosestTriToCenter( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex ) {
	f32 fSmallestDist2;
	KongTri_t *pKTri, *pCurKTri, *pTemp;
	u32 i, j, nLastTriIndex;
	CFVec3 P1ToP2;

	FASSERT( m_nNumTris > 0 );

	nLastTriIndex = uStartingKTriIndex + uKongTris;
	// compute the distance2 to each unadded tri from the center of our bounding sphere
	if( m_bNeedToComputeDistance ) {
		for( i=uStartingKTriIndex; i < nLastTriIndex; i++ ) {
			pKTri = (KongTri_t *)rKongTris[i];
			if( !pKTri->bTriAdded ) {
				P1ToP2 = pKTri->BoundSphere.m_Pos - m_BSphere.m_Pos;
				pKTri->fDist2 = P1ToP2.Mag2();
			}
		}
		m_bNeedToComputeDistance = FALSE;
	}

	// find the closest tri to the center that is connected to something already in our list
	fSmallestDist2 = -1.0f;
	pKTri = NULL;
	for( i=0; i < m_nNumTris; i++ ) {
		pCurKTri = (KongTri_t *)m_TriList[i];
		for( j=0; j < pCurKTri->nNumEdgeTris; j++ ) {
			pTemp = pCurKTri->paEdgeTris[j];
			if( !pTemp->bTriAdded ) {
				// is pTemp close enough?
				if( pTemp->fDist2 <= m_BSphere.m_fRadius ) {
					// this tri is within the radius2
					return pTemp;			
				} else {
					if( fSmallestDist2 >= 0.0f ) {
						// see if we are closer than whatever tri set this value
						if( pTemp->fDist2 < fSmallestDist2 ) {
							fSmallestDist2 = pTemp->fDist2;
							pKTri = pTemp;
						}
					} else {
						// set the smallest dist2 value
						fSmallestDist2 = pTemp->fDist2;
						pKTri = pTemp;
					}
				}
			}
		}
	}
	if( pKTri ) {
		// there is a connected tri inside of our current radius, add that tri
		return pKTri;
	}
	// find the closest unused tri to the center
	fSmallestDist2 = -1.0f;
	pKTri = NULL;
	for( i=uStartingKTriIndex; i < nLastTriIndex; i++ ) {
		pCurKTri = (KongTri_t *)rKongTris[i];
		if( !pCurKTri->bTriAdded ) {
			if( pCurKTri->fDist2 <= m_BSphere.m_fRadius ) {
				// this tri is within the radius2
				return pCurKTri;			
			} else {
				if( fSmallestDist2 >= 0.0f ) {
					// see if we are closer than whatever tri set this value
					if( pCurKTri->fDist2 < fSmallestDist2 ) {
						fSmallestDist2 = pCurKTri->fDist2;
						pKTri = pCurKTri;
					}
				} else {
					// set the smallest dist2 value
					fSmallestDist2 = pCurKTri->fDist2;
					pKTri = pCurKTri;
				}
			}
		}
	}

	// pKTri can be NULL
	return pKTri;
}

void CTriCollection::AddTri( KongTri_t *pKTri ) {
	f32 fDist;
	CFVec3 CenterToTriCenter;

	pKTri->bTriAdded = TRUE;// always mark the tris that have been added

	// update our cache, just in case we want to remove this tri
	m_bLastCacheValid = TRUE;
	m_LastBSphere = m_BSphere;
	m_LastMin = m_Min;
	m_LastMax = m_Max;

	// update all of our vars
	m_nNumTris++;
	m_TriList.Add( pKTri );
	FASSERT( (int)m_nNumTris == m_TriList.GetSize() );

	m_nNumVerts += 3;
	m_VertList.Add( pKTri->apKongVerts[0] );
	m_VertList.Add( pKTri->apKongVerts[1] );
	m_VertList.Add( pKTri->apKongVerts[2] );
	FASSERT( (int)m_nNumVerts == m_VertList.GetSize() );

	if( m_nNumTris > 1 ) {
		// update our sphere and min/max vars
		CenterToTriCenter = pKTri->BoundSphere.m_Pos - m_BSphere.m_Pos;
		fDist = CenterToTriCenter.Mag();
		fDist += pKTri->BoundSphere.m_fRadius;
		fDist *= fDist;
		if( fDist > m_BSphere.m_fRadius ) {
			m_BSphere.m_fRadius = fDist;
		}
	
		m_Min.Min( pKTri->apKongVerts[0]->Pos );
		m_Min.Min( pKTri->apKongVerts[1]->Pos );
		m_Min.Min( pKTri->apKongVerts[2]->Pos );

		m_Max.Max( pKTri->apKongVerts[0]->Pos );
		m_Max.Max( pKTri->apKongVerts[1]->Pos );
		m_Max.Max( pKTri->apKongVerts[2]->Pos );
	} else {
		// init the min/max & bounding sphere
		m_Min = pKTri->apKongVerts[0]->Pos;
		m_Min.Min( pKTri->apKongVerts[1]->Pos );
		m_Min.Min( pKTri->apKongVerts[2]->Pos );

		m_Max = pKTri->apKongVerts[0]->Pos;
		m_Max.Max( pKTri->apKongVerts[1]->Pos );
		m_Max.Max( pKTri->apKongVerts[2]->Pos );

		m_BSphere = pKTri->BoundSphere;
		// remember to square the radius
		m_BSphere.m_fRadius *= m_BSphere.m_fRadius;
	}
}

void CTriCollection::RemoveLastTri() {
	
	if( !m_nNumTris ) {
		// nothing to remove
		return;
	}
	// grab the kong tri that we are about to remove
	KongTri_t *pKTri = (KongTri_t *)m_TriList[m_nNumTris-1];
	pKTri->bTriAdded = FALSE;
	// remove the tri from our list
	m_nNumTris--;
	m_TriList.RemoveAt( m_nNumTris, 1 );
	FASSERT( (int)m_nNumTris == m_TriList.GetSize() );
	// remove the 3 verts from our list
	m_nNumVerts -= 3;
	m_VertList.RemoveAt( m_nNumVerts, 3 );
	FASSERT( (int)m_nNumVerts == m_VertList.GetSize() );

	// restore the min, max & sphere to the cached state
	if( m_bLastCacheValid ) {
		m_BSphere = m_LastBSphere;
		m_LastMin = m_Min;
		m_LastMax = m_Max;
		// calculate the square root of the bounding sphere radius (all internal calculations are done with R2)
		m_BSphere.m_fRadius = fmath_Sqrt( m_BSphere.m_fRadius );
		m_bLastCacheValid = FALSE;
	}
}

// this is meant to be called outside of the create/add functions
// it does not square the bounding sphere radius
void CTriCollection::RecalculateBoundingInfo() {
	u32 i;
	KongVert_t *pKongVert;
	KongTri_t *pKTri;
	f32 fDist;
	CFVec3 CenterToTriCenter;

	if( !m_nNumTris ) {
		return;
	}
	// compute the min/max
	pKongVert = (KongVert_t *)m_VertList[0];
	m_Min = pKongVert->Pos;
	m_Max = pKongVert->Pos;
	for( i=1; i < m_nNumVerts; i++ ) {
		pKongVert = (KongVert_t *)m_VertList[i];
		m_Min.Min( pKongVert->Pos );
		m_Max.Max( pKongVert->Pos );
	}
	// compute the bounding sphere
	pKTri = (KongTri_t *)m_TriList[0];
	m_BSphere = pKTri->BoundSphere;
	for( i=1; i < m_nNumTris; i++ ) {
		pKTri = (KongTri_t *)m_TriList[i];

		CenterToTriCenter = pKTri->BoundSphere.m_Pos - m_BSphere.m_Pos;
		fDist = CenterToTriCenter.Mag();
		fDist += pKTri->BoundSphere.m_fRadius;
		if( fDist > m_BSphere.m_fRadius ) {
			m_BSphere.m_fRadius = fDist;
		}
	}
}

