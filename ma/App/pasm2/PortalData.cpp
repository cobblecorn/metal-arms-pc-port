//////////////////////////////////////////////////////////////////////////////////////
// PortalData.cpp - 
//
// Author: Michael Starich   
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
// 07/11/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "PortalData.h"
#include "fmath.h"
#include "fclib.h"
#include "utils.h"
#include "fvis.h"

#define _SIN_45					( 0.70710678118654752440084436210485f )
#define _COS_45					( 0.70710678118654752440084436210485f )

#define _KEEP_CLIPPED_CELL_2	( 3.0f * 3.0f )

#define _AUTO_PORTAL_NAME		"Auto Portal"

#if FANG_DEBUG_BUILD
	#define _INSANE_AMOUNT_OF_CHECKING		TRUE
#else
	#define _INSANE_AMOUNT_OF_CHECKING		FALSE
#endif

////////////////////
// CPortalData_Plane
////////////////////

void CPortalData_Plane::Create( const ApeVisFaces_t &rFace ) {
	m_Pt = rFace.Centroid;
	m_Normal = rFace.Normal;

#if _INSANE_AMOUNT_OF_CHECKING
	// make sure that the normal is unit
	f32 fMag2;
	fMag2 = m_Normal.Mag2();
	FASSERT( fMag2 > 0.90f );
	m_Normal *= ( 1.0f / fmath_AcuSqrt( fMag2 ) );
#endif
}

// returns:
// LOC_OUT if rPt is on the on the +normal side
// LOC_ON if rPt is on the plane
// LOC_IN if rPt is on the -normal side
// 
// rfDistToPlane will always be a positve distance to the plane
PortalData_Loc_e CPortalData_Plane::WhichSideOfPlane( const CFVec3 &rPt, f32 &rfDistToPlane ) const {
	f32 fMag2;
	CFVec3 PtToPlane( rPt );
	
	PtToPlane -= m_Pt;
	fMag2 = PtToPlane.Mag2();
	if( fMag2 <= ON_PLANE_EPSILON_2 ) {
		// rPt is so close to the point defining the plane that we must consider the pt on the plane
		rfDistToPlane = 0.0f;
		return LOC_ON;
	}

	rfDistToPlane = PtToPlane.Dot( m_Normal );

	if( rfDistToPlane > ON_PLANE_EPSILON ) {
		return LOC_OUT;
	} else if( rfDistToPlane >= (-ON_PLANE_EPSILON) ) {
		rfDistToPlane = 0.0f;
		return LOC_ON;
	}

	rfDistToPlane = -rfDistToPlane;

	return LOC_IN;
}

///////////////////
// CPortalData_Face
///////////////////

// returns the number of points contained in the face and puts a 
// CPortalData_Point ptr to each unique one in rPts
u32 CPortalData_Face::GetPtList( CPtrArray &rPts ) const {
	u32 i, j, nNumUnique;
	CPortalData_Edge *pEdge;
	BOOL bAdd1, bAdd2;
	
	rPts.RemoveAll();
	nNumUnique = 0;
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		if( i > 0 ) {
			// make sure that the new pts are unique
			bAdd1 = bAdd2 = TRUE;
			for( j=0; j < nNumUnique; j++ ) {
				if( bAdd1 ) {
					if( (u32)rPts[j] == (u32)pEdge->m_apPoints[0] ) {
						bAdd1 = FALSE;
					}
				}
				if( bAdd2 ) {
					if( (u32)rPts[j] == (u32)pEdge->m_apPoints[1] ) {
						bAdd2 = FALSE;
					}
				}
			}
			if( bAdd1 ) {
				rPts.Add( pEdge->m_apPoints[0] );
				nNumUnique++;
			}
			if( bAdd2 ) {
				rPts.Add( pEdge->m_apPoints[1] );
				nNumUnique++;
			}
		} else {
			// add both pts as the first 2
			rPts.Add( pEdge->m_apPoints[0] );
			rPts.Add( pEdge->m_apPoints[1] );
			nNumUnique = 2;
		}
	}
	
	FASSERT( (int)nNumUnique == rPts.GetSize() );

	return nNumUnique;
}

u32 CPortalData_Face::GetListOfPtsOnThePlane( CPtrArray &rPts ) const {
	u32 i;
	u32 nNumPts = GetPtList( rPts );
	CPortalData_Point *pPt;

	for( i=0; i < nNumPts; ) {
		pPt = (CPortalData_Point *)rPts[i];
		if( pPt->m_Loc != LOC_ON ) {
			// this pt is not on the plane, kick it out
			rPts.RemoveAt( i, 1 );
			nNumPts--;
		} else {
			i++;
		}
	}
	FASSERT( (int)nNumPts == rPts.GetSize() );

	return nNumPts;
}

void CPortalData_Face::RecomputeCentroid() {
	CPtrArray Pts;
	CPortalData_Point *pPt;
	u32 i;
	u32 nNumPts = GetPtList( Pts );
	
	m_Centroid.Zero();
		
	for( i=0; i < nNumPts; i++ ) {
		pPt = (CPortalData_Point *)Pts[i];
		m_Centroid += pPt->m_Pos;
	}
	if( nNumPts ) {
		m_Centroid *= (1.0f/(f32)nNumPts);
	}
}

// for a face to be sandwiched together, 
// the faces must share a plane
// and the normals must be facing in opposite directions
BOOL CPortalData_Face::AreFacesSandwichedTogether( const CPortalData_Face *pFace1, const CPortalData_Face *pFace2 ) {
	f32 fDist;
	
	// make sure each face's centroid is on the plane of the other face
	if( pFace1->m_Plane.WhichSideOfPlane( pFace2->m_Centroid, fDist ) != LOC_ON ) {
		return FALSE;
	}
	if( pFace2->m_Plane.WhichSideOfPlane( pFace1->m_Centroid, fDist ) != LOC_ON ) {
		return FALSE;
	}

	// make sure the normal of the 2 faces are in an opposite directions
	fDist = pFace1->m_Plane.m_Normal.Dot( pFace2->m_Plane.m_Normal );
	if( fDist > -0.99f ) {
		return FALSE;
	}
	
	return TRUE;
}

BOOL CPortalData_Face::RemoveEdge( CPortalData_Edge *pEdge ) {
	s32 nIndex = CPortalData_Cell::FindIndexFromPtr( pEdge, m_nNumEdges, m_apEdges );

	if( nIndex < 0 ) {
		// pEdge isn't part of this face
		return FALSE;
	}

	m_apEdges.RemoveAt( nIndex, 1 );
	m_nNumEdges--;

	return TRUE;
}

BOOL CPortalData_Face::IsFaceOnCellShell( const CPortalData_Cell &rCell ) const {
	CPtrArray apFacePts;
	u32 nNumFacePts, i;
	CPortalData_Point *pPt;
	f32 fDist;

	// get the pt list
	nNumFacePts = GetPtList( apFacePts );
	if( !nNumFacePts ) {
		return FALSE;
	}
	
	// test each face pt for being on the cell's outside shell
	for( i=0; i < nNumFacePts; i++ ) {
		pPt = (CPortalData_Point *)apFacePts[i];

		if( rCell.PointToCell( pPt->m_Pos, fDist ) != LOC_ON ) {
			// pPt is not on the cell's outside shell
			return FALSE;
		}
	}

	return TRUE;
}

CPortalData_Edge *CPortalData_Face::GetLongestEdge() const {
	u32 i;
	CPortalData_Edge *pEdge, *pLongest = NULL;
	f32 fDist2, fLongest = 0.0f;
	CFVec3 Temp;

	if( !m_nNumEdges ) {
		return NULL;
	}
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		Temp = pEdge->m_apPoints[0]->m_Pos - pEdge->m_apPoints[1]->m_Pos;
		fDist2 = Temp.Mag2();
		if( fDist2 > fLongest ) {
			pLongest = pEdge;
			fLongest = fDist2;
		}
	}
	return pLongest;
}

////////////////////
// CPortalData_Point
////////////////////

void CPortalData_Point::Create( const ApeVisPt_t &rPt ) {
	m_Pos = rPt.Pos;
	m_Loc = LOC_UNKNOWN;
	m_fDistToPlane = 0.0f;
}

f32 CPortalData_Point::GetDist2BetweenPts( const CPortalData_Point *pPt1, const CPortalData_Point *pPt2 ) {
	CFVec3 Diff;

	Diff = pPt1->m_Pos - pPt2->m_Pos;
	return Diff.Mag2();
}

///////////////////
// CPortalData_Edge
///////////////////

void CPortalData_Edge::RemoveFace( CPortalData_Face *pFace ) {

	switch( m_nNumFaces ) {

	case 2:
		if( m_apFaces[1] == pFace ) {
			// remove this face
			m_apFaces[1] = NULL;
			m_nNumFaces--;
		} else if( m_apFaces[0] == pFace ) {
			// remove this face
			m_apFaces[0] = m_apFaces[1];
			m_apFaces[1] = NULL;
			m_nNumFaces--;
		}
		break;
		
	case 1:
		if( m_apFaces[0] == pFace ) {
			// remove this face
			m_apFaces[0] = NULL;
			m_nNumFaces--;
		}
		break;

	default:
	case 0:
		// nothing to do
		break;
	}		
}

// creates an empty edge
void CPortalData_Edge::Create( CPortalData_Point &rP1, CPortalData_Point &rP2 ) {

	m_apPoints[0] = &rP1;
	m_apPoints[1] = &rP2;
	
	m_nNumFaces = 0;
	m_apFaces[0] = NULL;
	m_apFaces[1] = NULL;

	m_Loc = LOC_UNKNOWN;
}

///////////////////
// CPortalData_Cell
///////////////////

CPortalData_Cell::CPortalData_Cell() {

	m_pszName = NULL;
	m_bDeleteNameString = FALSE;

	m_nNumFaces = 0;
	m_apFaces.RemoveAll();

	m_nNumEdges = 0;
	m_apEdges.RemoveAll();

	m_nNumPts = 0;
	m_apPts.RemoveAll();
}

CPortalData_Cell::~CPortalData_Cell() {
	Empty();
}

BOOL CPortalData_Cell::Create( const ApeVisCell_t &rCell ) {
	u32 i, j;
	CPortalData_Point *pPt;
	CPortalData_Edge *pEdge;
	CPortalData_Face *pFace;
	const ApeVisEdges_t *pApeEdge;
	const ApeVisFaces_t *pApeFace;
#if _INSANE_AMOUNT_OF_CHECKING
	f32 fDist;
#endif

	if( !AllocateMemory( rCell.nNumFaces, rCell.nNumEdges, rCell.nNumPts ) ) {
		return FALSE;
	}

	// create the pt array
	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];
		
		pPt->Create( rCell.aPts[i] );
	}

	// create the edge array
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];
		pApeEdge = &rCell.aEdges[i];

		pEdge->m_apPoints[0] = (CPortalData_Point *)m_apPts[ pApeEdge->anPtIndices[0] ];
		pEdge->m_apPoints[1] = (CPortalData_Point *)m_apPts[ pApeEdge->anPtIndices[1] ];
		pEdge->m_nNumFaces = pApeEdge->nNumFaces;
		for( j=0; j < pApeEdge->nNumFaces; j++ ) {
			pEdge->m_apFaces[j] = (CPortalData_Face *)m_apFaces[ pApeEdge->anFaceIndices[j] ];
		}
		pEdge->m_Loc = LOC_UNKNOWN;
	}

	// create the face array
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];
		pApeFace = (ApeVisFaces_t *)&rCell.aFaces[i];

		pFace->m_Plane.Create( *pApeFace );
		
		pFace->m_apEdges.RemoveAll();
		pFace->m_nNumEdges = pApeFace->nDegree;
		for( j=0; j < pApeFace->nDegree; j++ ) {
			pFace->m_apEdges.Add( m_apEdges[ pApeFace->aEdgeIndices[j] ] );
		}

		pFace->m_Centroid = pApeFace->Centroid;

		pFace->m_Loc = LOC_UNKNOWN;

#if _INSANE_AMOUNT_OF_CHECKING
		// make sure that the centroid is actually on the plane
		FASSERT( pFace->m_Plane.WhichSideOfPlane( pFace->m_Centroid, fDist ) == LOC_ON );

		// make sure all edges are on the plane
		for( j=0; j < pFace->m_nNumEdges; j++ ) {
			pEdge = (CPortalData_Edge *)pFace->m_apEdges[j];

			FASSERT( pFace->m_Plane.WhichSideOfPlane( pEdge->m_apPoints[0]->m_Pos, fDist ) == LOC_ON );
			FASSERT( pFace->m_Plane.WhichSideOfPlane( pEdge->m_apPoints[1]->m_Pos, fDist ) == LOC_ON );
		}
#endif
	}

	m_pszName = rCell.szName;
	m_bDeleteNameString = FALSE;
	
#if 0
	m_BoundingSphere = rCell.Sphere;
	// pad the bounding sphere radius
	m_BoundingSphere.m_fRadius *= UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;
#else
	RecomputeBoundingSphere();
#endif
	
	return TRUE;
}

BOOL CPortalData_Cell::Create( const CPortalData_Cell &rCell ) {
	u32 i, j;
	s32 nIndex;
	const CPortalData_Point *pSrcPt;
	CPortalData_Point *pDstPt;
	const CPortalData_Edge *pSrcEdge;
	CPortalData_Edge *pDstEdge;
	const CPortalData_Face *pSrcFace;
	CPortalData_Face *pDstFace;
#if _INSANE_AMOUNT_OF_CHECKING
	f32 fDist;
#endif

	if( !AllocateMemory( rCell.m_nNumFaces, rCell.m_nNumEdges, rCell.m_nNumPts ) ) {
		return FALSE;
	}

	// copy the pt array
	for( i=0; i < m_nNumPts; i++ ) {
		pDstPt = (CPortalData_Point *)m_apPts[i];
		pSrcPt = (CPortalData_Point *)rCell.m_apPts[i];

		// copy the data
		pDstPt->m_Pos = pSrcPt->m_Pos;
		pDstPt->m_Loc = pSrcPt->m_Loc;
		pDstPt->m_fDistToPlane = pSrcPt->m_fDistToPlane;
	}

	// copy the edge array
	for( i=0; i < m_nNumEdges; i++ ) {
		pDstEdge = (CPortalData_Edge *)m_apEdges[i];
		pSrcEdge = (CPortalData_Edge *)rCell.m_apEdges[i];
		
		// copy the data
		nIndex = FindIndexFromPtr( pSrcEdge->m_apPoints[0], rCell.m_nNumPts, rCell.m_apPts );
		FASSERT( nIndex >= 0 );
		pDstEdge->m_apPoints[0] = (CPortalData_Point *)m_apPts[nIndex];

		nIndex = FindIndexFromPtr( pSrcEdge->m_apPoints[1], rCell.m_nNumPts, rCell.m_apPts );
		FASSERT( nIndex >= 0 );
		pDstEdge->m_apPoints[1] = (CPortalData_Point *)m_apPts[nIndex];

		pDstEdge->m_nNumFaces = pSrcEdge->m_nNumFaces;
		for( j=0; j < pDstEdge->m_nNumFaces; j++ ) {
			nIndex = FindIndexFromPtr( pSrcEdge->m_apFaces[j], rCell.m_nNumFaces, rCell.m_apFaces );
			FASSERT( nIndex >= 0 );
			pDstEdge->m_apFaces[j] = (CPortalData_Face *)m_apFaces[nIndex];
		}
		pDstEdge->m_Loc = pSrcEdge->m_Loc;
	}

	// copy the face array
	for( i=0; i < m_nNumFaces; i++ ) {
		pDstFace = (CPortalData_Face *)m_apFaces[i];
		pSrcFace = (CPortalData_Face *)rCell.m_apFaces[i];

		// copy the data
		pDstFace->m_Plane.m_Normal = pSrcFace->m_Plane.m_Normal;
		pDstFace->m_Plane.m_Pt = pSrcFace->m_Plane.m_Pt;
		
		pDstFace->m_Centroid = pSrcFace->m_Centroid;
		
		pDstFace->m_nNumEdges = pSrcFace->m_nNumEdges;

		pDstFace->m_apEdges.RemoveAll();
		for( j=0; j < pDstFace->m_nNumEdges; j++ ) {
			nIndex = FindIndexFromPtr( pSrcFace->m_apEdges[j], rCell.m_nNumEdges, rCell.m_apEdges );
			FASSERT( nIndex >= 0 );
			pDstFace->m_apEdges.Add( m_apEdges[nIndex] );
		}
		
		pDstFace->m_Loc = pSrcFace->m_Loc;

#if _INSANE_AMOUNT_OF_CHECKING
		// make sure that the centroid is actually on the plane
		FASSERT( pDstFace->m_Plane.WhichSideOfPlane( pDstFace->m_Centroid, fDist ) == LOC_ON );

		// make sure all edges are on the plane
		for( j=0; j < pDstFace->m_nNumEdges; j++ ) {
			pDstEdge = (CPortalData_Edge *)pDstFace->m_apEdges[j];

			FASSERT( pDstFace->m_Plane.WhichSideOfPlane( pDstEdge->m_apPoints[0]->m_Pos, fDist ) == LOC_ON );
			FASSERT( pDstFace->m_Plane.WhichSideOfPlane( pDstEdge->m_apPoints[1]->m_Pos, fDist ) == LOC_ON );
		}
#endif
	}

	m_pszName = rCell.m_pszName;
	m_bDeleteNameString = FALSE;
	m_BoundingSphere = rCell.m_BoundingSphere;

	return TRUE;
}

// NOTE: nLoc must be LOC_OUT or LOC_IN and represents which side of the plane to KEEP
BOOL CPortalData_Cell::Create( const CPortalData_Cell &rCell, const CPortalData_Plane &rPlane,
							   PortalData_Loc_e nLoc, BOOL bCellAlreadyClipped/*=FALSE*/ ) {

	// copy rCell so that we can operate on it
	if( !Create( rCell ) ) {
		return FALSE;
	}

	return SliceAlongPlane( rPlane, nLoc, bCellAlreadyClipped );	
}

void CPortalData_Cell::Empty() {
	u32 i;
	CPortalData_Face *pFace;
	CPortalData_Edge *pEdge;
	CPortalData_Point *pPt;

	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		delete pFace;
	}
	m_apFaces.RemoveAll();
	m_nNumFaces = 0;
	
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		delete pEdge;
	}
	m_apEdges.RemoveAll();
	m_nNumEdges = 0;

	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];

		delete pPt;
	}
	m_apPts.RemoveAll();
	m_nNumPts = 0;

	if( m_bDeleteNameString ) {
		delete [] (char *)m_pszName;
	}
	m_pszName = NULL;
	m_bDeleteNameString = FALSE;
}

PortalData_Loc_e CPortalData_Cell::CellToPlane( const CPortalData_Plane &rPlane, BOOL bReCalcLocData/*=TRUE*/ ) {

	if( bReCalcLocData ) {
		CalculateLocDataFromPlane( rPlane );	
	}
	u32 i;
	CPortalData_Face *pFace;
	PortalData_Loc_e nLoc;

	if( !m_nNumFaces ) {
		return LOC_UNKNOWN;
	}

	// walk the faces and determine where the the cell is located relative to the plane
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		if( i > 0 ) {
			switch( pFace->m_Loc ) {
			case LOC_OUT:
				if( nLoc == LOC_IN ) {
					return LOC_IN_AND_OUT;
				} else if( nLoc == LOC_ON ) {
					// update our location
					nLoc = LOC_OUT;
				}
				break;
			case LOC_IN:
				if( nLoc == LOC_OUT ) {
					return LOC_IN_AND_OUT;
				} else if( nLoc == LOC_ON ) {
					// update our location
					nLoc = LOC_IN;
				}
				break;
			case LOC_ON:
				break;
			case LOC_IN_AND_OUT:
				return LOC_IN_AND_OUT;
				break;
			}
		} else {
			// store the 1st state as our location
			nLoc = pFace->m_Loc;
			if( nLoc == LOC_IN_AND_OUT ) {
				return LOC_IN_AND_OUT;
			}
		}
	}

	return nLoc;
}

// returns:
// TRUE if there is a plane that divides the 2 cells (rPlane will contain the plane definition and where or not the place is actually a face)
// FALSE if no plane exists, the cells are colliding
BOOL CPortalData_Cell::DoesPlaneExistBetween( CPortalData_Cell &rCell, CPortalData_Plane &rPlane, BOOL &rbIsPlaneACellFace ) {
	u32 i, j;
	const CPortalData_Face *pFace;
	PortalData_Loc_e nSide1, nSide2;
	CFVec3 Edge1, Edge2;
	const CPortalData_Edge *pEdge1, *pEdge2;
	f32 fMag2;
	
	if( !m_nNumFaces || !rCell.m_nNumFaces ) {
		return FALSE;
	}

	// test the faces of this cell to see if one of them is a dividing plane
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		// set the plane to the face 
		rPlane.m_Pt = pFace->m_Plane.m_Pt;
		rPlane.m_Normal = pFace->m_Plane.m_Normal;

		nSide1 = rCell.CellToPlane( rPlane, TRUE );
		if( nSide1 != LOC_IN_AND_OUT ) {
			nSide2 = CellToPlane( rPlane, TRUE );

			if( (nSide2 == LOC_OUT && nSide1 == LOC_IN) ||
				(nSide2 == LOC_IN && nSide1 == LOC_OUT) ) {
				// the 2 cells are on different sides of the plane, we have a dividing plane
				rbIsPlaneACellFace = TRUE;
				return TRUE;
			}
		}
	}

	// test the faces of rCell to see if one of them is a dividing plane
	for( i=0; i < rCell.m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)rCell.m_apFaces[i];

		// set the plane to the face 
		rPlane.m_Pt = pFace->m_Plane.m_Pt;
		rPlane.m_Normal = pFace->m_Plane.m_Normal;

		nSide1 = CellToPlane( rPlane, TRUE );
		if( nSide1 != LOC_IN_AND_OUT ) {
			nSide2 = rCell.CellToPlane( rPlane, TRUE );

			if( (nSide2 == LOC_OUT && nSide1 == LOC_IN) ||
				(nSide2 == LOC_IN && nSide1 == LOC_OUT) ) {
				// the 2 cells are on different sides of the plane, we have a dividing plane
				rbIsPlaneACellFace = TRUE;
				return TRUE;
			}
		}
	}

	// Test planes whose normals are cross products of two edges, one from each cell
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge1 = (CPortalData_Edge *)m_apEdges[i];

		Edge1 = pEdge1->m_apPoints[1]->m_Pos - pEdge1->m_apPoints[0]->m_Pos;
				
		// for every edge in rCell
		for( j=0; j < rCell.m_nNumEdges; j++ ) {
			pEdge2 = (CPortalData_Edge *)rCell.m_apEdges[j];

			rPlane.m_Pt = pEdge2->m_apPoints[1]->m_Pos;

			Edge2 = rPlane.m_Pt - pEdge2->m_apPoints[0]->m_Pos;

			// move the plane point to the midpoint between pt0 and pt1 to avoid zero length vecs later
			rPlane.m_Pt -= (Edge2 * 0.5f);
		
			rPlane.m_Normal = Edge1.Cross( Edge2 );
			fMag2 = rPlane.m_Normal.Mag2();
			if( fMag2 >= ON_PLANE_EPSILON_2 ) {
				rPlane.m_Normal *= ( 1.0f / fmath_AcuSqrt( fMag2 ) );
				
				nSide1 = CellToPlane( rPlane, TRUE );
				if( nSide1 != LOC_IN_AND_OUT ) {
					nSide2 = rCell.CellToPlane( rPlane, TRUE );

					if( (nSide2 == LOC_OUT && nSide1 == LOC_IN) ||
						(nSide2 == LOC_IN && nSide1 == LOC_OUT) ) {
						// the 2 cells are on different sides of the plane, we have a dividing plane
						rbIsPlaneACellFace = FALSE;
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

// clip any edges that cross rPlane, creates new edges and pts, but
// does NOT actually remove either side of the cell, to do that call
// SliceAlongPlane()
BOOL CPortalData_Cell::ClipAlongPlane( const CPortalData_Plane &rPlane ) {
	u32 i, j, nNumClippedEdges, nNumClippedFaces, nNumPts, nNumOnEdges;
	CPortalData_Edge *pExistingEdge, *pNewEdge;
	CFVec3 ImpactPt;
	f32 fPercent, fDist;
	CPortalData_Point *pIn, *pOut, *pNewPt, *pPt1, *pPt2;
	CPortalData_Face *pFace;
	CPtrArray apPtsOnThePlane;
#if _INSANE_AMOUNT_OF_CHECKING
	f32 fTemp;	
#endif

	// first calculate what side of the plane each pt is on
	CalculateLocDataFromPlane( rPlane );

	// clip all crossing edges
	nNumClippedEdges = 0;
	nNumOnEdges = 0;
	for( i=0; i < m_nNumEdges; i++ ) {
		pExistingEdge = (CPortalData_Edge *)m_apEdges[i];

		if( pExistingEdge->m_Loc == LOC_IN_AND_OUT ) {
			// always clip from the inside out
			if( pExistingEdge->m_apPoints[0]->m_Loc == LOC_IN ) {
				// the 1st pt is on the inside	
				pIn = pExistingEdge->m_apPoints[0];
				pOut = pExistingEdge->m_apPoints[1];
			} else {
				// the 2nd pt is on the inside
				pIn = pExistingEdge->m_apPoints[1];
				pOut = pExistingEdge->m_apPoints[0];
			}

			fDist = pIn->m_fDistToPlane + pOut->m_fDistToPlane;
			fPercent = pIn->m_fDistToPlane / fDist;
			ImpactPt.ReceiveLerpOf( fPercent, pIn->m_Pos, pOut->m_Pos );

#if _INSANE_AMOUNT_OF_CHECKING
			// make sure that the clipped point is actually on the plane	
			FASSERT( rPlane.WhichSideOfPlane( ImpactPt, fTemp ) == LOC_ON );		
#endif			
						
			// allocate a new pt
			pNewPt = new CPortalData_Point;
			if( !pNewPt ) {
				return FALSE;
			}
			// fill in the new pt
			pNewPt->m_fDistToPlane = 0.0f;
			pNewPt->m_Loc = LOC_ON;
			pNewPt->m_Pos = ImpactPt;

			// add the new pt to our cell's list
			m_apPts.Add( pNewPt );
			m_nNumPts++;
			
			// allocate a new edge
			pNewEdge = new CPortalData_Edge;
			if( !pNewEdge ) {
				return FALSE;
			}
			// fill in the new edge (always new pt to outside pt)
			pNewEdge->m_Loc = LOC_OUT;
			pNewEdge->m_apPoints[0] = pNewPt;
			pNewEdge->m_apPoints[1] = pOut;
			pNewEdge->m_nNumFaces = pExistingEdge->m_nNumFaces;
			pNewEdge->m_apFaces[0] = pExistingEdge->m_apFaces[0];
			pNewEdge->m_apFaces[1] = pExistingEdge->m_apFaces[1];

			// add the new edge to our cell's list
			m_apEdges.Add( pNewEdge );
			m_nNumEdges++;

			// fixup the existing edge
			pExistingEdge->m_Loc = LOC_IN;
			pExistingEdge->m_apPoints[0] = pIn;
			pExistingEdge->m_apPoints[1] = pNewPt;

			// fixup the face's edge list
			for( j=0; j < pExistingEdge->m_nNumFaces; j++ ) {
				pFace = pExistingEdge->m_apFaces[j];
				pFace->m_apEdges.Add( pNewEdge );
				pFace->m_nNumEdges++;
			}
			
			// we have successfully clipped pExistingEdge
			nNumClippedEdges++;
		} else if( pExistingEdge->m_Loc == LOC_ON ) {
			// we need to count these edges, even though we don't need to clip them
			nNumOnEdges++;
		}
	}

	// now create new edges across each face that was clipped
	nNumClippedFaces = 0;
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		if( pFace->m_Loc == LOC_IN_AND_OUT ) {
			
			// create an edge (only if an ON edge doesn't already exist)
			for( j=0; j < pFace->m_nNumEdges; j++ ) {
				pExistingEdge = (CPortalData_Edge *)pFace->m_apEdges[j];

				if( pExistingEdge->m_Loc == LOC_ON ) {
					// we already have an ON edge, don't create one
					break;
				}
			}
			if( j == pFace->m_nNumEdges ) {
				// create a pt list of pts on the plane
				nNumPts = pFace->GetListOfPtsOnThePlane( apPtsOnThePlane );
				FASSERT( nNumPts == 2 );

				// create an edge between these two pts
				pPt1 = (CPortalData_Point *)apPtsOnThePlane[0];
				pPt2 = (CPortalData_Point *)apPtsOnThePlane[1];

				// allocate a new edge
				pNewEdge = new CPortalData_Edge;
				if( !pNewEdge ) {
					return FALSE;
				}
				// fill in the new edge
				pNewEdge->m_Loc = LOC_ON;
				pNewEdge->m_nNumFaces = 1;
				pNewEdge->m_apFaces[0] = pFace;
				pNewEdge->m_apFaces[1] = NULL;
				pNewEdge->m_apPoints[0] = pPt1;
				pNewEdge->m_apPoints[1] = pPt2;
				
				// add the new edge to the face's list
				pFace->m_apEdges.Add( pNewEdge );
				pFace->m_nNumEdges++;

				// add the new edge to our cell's list
				m_apEdges.Add( pNewEdge );
				m_nNumEdges++;

				nNumClippedFaces++;
			}
		}
	}
	FASSERT( (nNumClippedEdges + nNumOnEdges) == nNumClippedFaces );

	return TRUE;
}

// NOTE: clips and then eliminates everything not on the same side as nLoc (which must be LOC_OUT or LOC_IN)
BOOL CPortalData_Cell::SliceAlongPlane( const CPortalData_Plane &rPlane, PortalData_Loc_e nLoc, BOOL bCellAlreadyClipped/*=FALSE*/ ) {

	FASSERT( nLoc == LOC_OUT || nLoc == LOC_IN );

	if( !bCellAlreadyClipped ) {
		// clip the cell along rPlane
		if( !ClipAlongPlane( rPlane ) ) {
			return FALSE;
		}
	}

	// remove all unwanted edges
	u32 i, j;
	CPortalData_Edge *pEdge;
	CPortalData_Face *pFace;
	CPortalData_Point *pPoint;
	
	for( i=0; i < m_nNumEdges; ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		FASSERT( pEdge->m_Loc != LOC_UNKNOWN && pEdge->m_Loc != LOC_IN_AND_OUT );
		
		if( pEdge->m_Loc != nLoc &&
			pEdge->m_Loc != LOC_ON ) {
			// remove this edge
			m_apEdges.RemoveAt( i, 1 );
			m_nNumEdges--;
			
			// remove this edge from the faces
			for( j=0; j < pEdge->m_nNumFaces; j++ ) {
				pFace = pEdge->m_apFaces[j];

				pFace->RemoveEdge( pEdge );				
			}

			// free the edge memory
			delete pEdge;

		} else {
			// leave the edge alone
			i++;
		}
	}

	// remove all empty faces
	for( i=0; i < m_nNumFaces; ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		FASSERT( pFace->m_Loc != LOC_UNKNOWN );	

		if( pFace->m_Loc == LOC_IN_AND_OUT ) {
			// these faces should have some edges in them
			FASSERT( pFace->m_nNumEdges >= 3 );

			// recompute the centroid
			pFace->RecomputeCentroid();
			
			// mark the face as on the correct side
			pFace->m_Loc = nLoc;

			// leave this face alone
			i++;

		} else if( pFace->m_Loc != nLoc &&
				   pFace->m_Loc != LOC_ON ) {
			// if this face has any edges (they better be ON edges), fix the edges up
			for( j=0; j < pFace->m_nNumEdges; j++ ) {
				pEdge = (CPortalData_Edge *)pFace->m_apEdges[j];
				FASSERT( pEdge->m_Loc == LOC_ON );

				// remove from the edge any reference to the face
				pEdge->RemoveFace( pFace );
				FASSERT( pEdge->m_nNumFaces == 1 );

				// remove this edge from the face
				pFace->RemoveEdge( pEdge );
			}
			FASSERT( pFace->m_nNumEdges == 0 );
			pFace->m_apEdges.RemoveAll();
						
			// remove this face
			m_apFaces.RemoveAt( i, 1 );
			m_nNumFaces--;

			// free the face memory
			delete pFace;			

		} else {
			// leave the face alone
			i++;
		}
	}

	// remove any unused pts
	for( i=0; i < m_nNumPts; ) {
		pPoint = (CPortalData_Point *)m_apPts[i];

		FASSERT( pPoint->m_Loc != LOC_UNKNOWN && pPoint->m_Loc != LOC_IN_AND_OUT );
		
		if( pPoint->m_Loc != nLoc &&
			pPoint->m_Loc != LOC_ON ) {
			// remove this pt
			m_apPts.RemoveAt( i, 1 );
			m_nNumPts--;

			// free the pt memory
			delete pPoint;

		} else {
			// keep this pt
			i++;
		}
	}

	// create a face with all of the edges that are on the plane
	pFace = new CPortalData_Face;
	if( !pFace ) {
		return FALSE;
	}
	// fill in the face
	pFace->m_Loc = LOC_ON;
	pFace->m_nNumEdges = GetListOfEdgesOnThePlane( pFace->m_apEdges );
	FASSERT( pFace->m_nNumEdges >= 3 );
	pFace->RecomputeCentroid();
	pFace->m_Plane.m_Pt = pFace->m_Centroid;
	pFace->m_Plane.m_Normal = (nLoc == LOC_OUT) ? -rPlane.m_Normal : rPlane.m_Normal;

	for( i=0; i < pFace->m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)pFace->m_apEdges[i];

		// fix up the faces that this edge is apart of
		FASSERT( pEdge->m_nNumFaces == 1 );
		pEdge->m_apFaces[1] = pFace;
		pEdge->m_nNumFaces++;
	}

	// add the face to the cell's face list
	m_apFaces.Add( pFace );
	m_nNumFaces++;
#if 0
	// recompute this cells bounding sphere
	RecomputeBoundingSphere();
#else
	RecomputeBoundingSphere();
#endif

	return TRUE;
}

void CPortalData_Cell::RecomputeBoundingSphere() {
	CFVec3 Min, Max;

	CFSphere spBoxMethod;
	CalculateBoundingBox( Min, Max );
	utils_GetBoundingSphereFromMinMax( spBoxMethod.m_Pos, spBoxMethod.m_fRadius, Min, Max );

	CalculateCentroid( m_BoundingSphere.m_Pos );
	m_BoundingSphere.m_fRadius = CalculateFurthestDistFromPt( m_BoundingSphere.m_Pos );

	if ( spBoxMethod.m_fRadius < m_BoundingSphere.m_fRadius )
	{
		m_BoundingSphere = spBoxMethod;
	}

	// pad the bounding sphere radius
	m_BoundingSphere.m_fRadius *= UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;
}

// returns:
// LOC_IN if rPt is inside the Cell
// LOC_OUT if rPt is outside the Cell
// LOC_ON if rPt is co-planar with one of the Cell's boundaries
// LOC_UNKNOWN if there is an error
//
// NOTE: rfDistToClosestPlane will only be set if rPt is on or in rCell
PortalData_Loc_e CPortalData_Cell::PointToCell( const CFVec3 &rPt, f32 &rfDistToClosestPlane ) const {
	u32 i;
	f32 fDot;
	const CPortalData_Face *pFace;;
	PortalData_Loc_e nLoc;

	if( !m_nNumFaces ) {
		return LOC_UNKNOWN;
	}

	if( !m_BoundingSphere.IsIntersecting( rPt ) ) {
		// can't be inside the cell if we aren't inside the cell's bounding sphere
		return LOC_OUT;
	}

	// see if rPt is on the inside of each face
	rfDistToClosestPlane = FMATH_MAX_FLOAT;
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];
		
		nLoc = pFace->m_Plane.WhichSideOfPlane( rPt, fDot );
		if( nLoc == LOC_OUT ) {
			return LOC_OUT;
		} else if( fDot < rfDistToClosestPlane ) {
			rfDistToClosestPlane = fDot;
		}		
	}

	return (rfDistToClosestPlane > 0.0f) ? LOC_IN : LOC_ON;
}

// returns:
// LOC_IN if both pts of edge are inside the Cell (or both are on the outside edge and the midpt is inside)
// LOC_OUT if both pts of edge are outside the Cell
// LOC_ON if both pts of edge are on a outside plane of the Cell (the same plane)
// LOC_IN_AND_OUT if one pt is inside the Cell and the other is outside
PortalData_Loc_e CPortalData_Cell::EdgeToCell( const CPortalData_Edge &rEdge ) const {
	PortalData_Loc_e nLoc1, nLoc2;	

	return EdgeToCell( rEdge, nLoc1, nLoc2 );
}

// returns:
// LOC_IN if both pts of edge are inside the Cell (or both are on the outside edge and the midpt is inside)
// LOC_OUT if both pts of edge are outside the Cell
// LOC_ON if both pts of edge are on a outside plane of the Cell (the same plane)
// LOC_IN_AND_OUT if one pt is inside the Cell and the other is outside
PortalData_Loc_e CPortalData_Cell::EdgeToCell( const CPortalData_Edge &rEdge, 
											   PortalData_Loc_e &rnLoc1, PortalData_Loc_e &rnLoc2 ) const {
	f32 fDist;

	rnLoc1 = PointToCell( rEdge.m_apPoints[0]->m_Pos, fDist );
	rnLoc2 = PointToCell( rEdge.m_apPoints[1]->m_Pos, fDist );

	// if both pts are on the plane, test the mid pt and see if that is in or on
	if( rnLoc1 == LOC_ON && rnLoc2 == LOC_ON ) {
		CFVec3 MidPt;
		MidPt.ReceiveLerpOf( 0.5f, rEdge.m_apPoints[0]->m_Pos, rEdge.m_apPoints[1]->m_Pos );

		if( PointToCell( MidPt, fDist ) == LOC_ON ) {
			// the whole edge is on the plane
			return LOC_ON;
		} else {
			// the mid pt is not on the plane, so we must be inside
			return LOC_IN;
		}
	}

	if( rnLoc1 == rnLoc2 ) {
		// whatever side they are on, both pts are in the same state
		return rnLoc1;
	}
	if( rnLoc1 == LOC_ON ) {
		// if pt1 is on, mark the edge as wherever pt2 is
		return rnLoc2;
	}
	if( rnLoc2 == LOC_ON ) {
		// if pt2 is on, mark the edge as wherever pt1 is
		return rnLoc1;
	}

	// the 2 pts must be on different sides of the plane
	return LOC_IN_AND_OUT;
}

// returns:
// TRUE if the segement intersects the cell (or is contained in the cell)
// FALSE if the segment does not intersects the cell (or the entire cell is on a plane of the cell)
BOOL CPortalData_Cell::DoesSegmentIntersectCell( const CFVec3 &rStart, const CFVec3 &rEnd ) const {
	PortalData_Loc_e nStartLoc, nEndLoc;
	f32 fStartDist, fEndDist;

	// Test the start and end point to determine if they are in the cell
	nStartLoc = PointToCell( rStart, fStartDist );
	nEndLoc = PointToCell( rEnd, fEndDist );
		
	// If the start is in, then we have an easy out
	if( nStartLoc == LOC_IN ) {
		return TRUE;
	}
	// If the end is in, then we have an easy out
	if( nEndLoc == LOC_IN ) {
		return TRUE;
	}

	// if both pts are on the plane, test the mid pt and see if that is in or on
	if( nStartLoc == LOC_ON && nEndLoc == LOC_ON ) {
		CFVec3 MidPt;
		f32 fDist;
		MidPt.ReceiveLerpOf( 0.5f, rStart, rEnd );

		if( PointToCell( MidPt, fDist ) == LOC_ON ) {
			// the whole segment is on the plane, consider that no intersecting
			return FALSE;
		} else {
			// the mid pt is not on the plane, so we must be crossing
			return TRUE;
		}
	} else if( nStartLoc == LOC_ON || nEndLoc == LOC_ON ) {
		// one of the pts is on the plane and the other must be outside, so there is no collision
		return FALSE;
	}
	
	
	// Unfortunately, neither the start nor the end of the ray were in the
	// cell.  But there still could be collision, so we need to do the hard
	// check to see if the ray passes through the convex hull:
	
	// This code is a modified "Slabs Method" which is basically the same as 3D clipping code:
	u32 i;
	CFVec3 vTemp;
	f32 fTest, fMin = 0.0f, fMax = 1.0f;
	const CPortalData_Face *pFace;

	for( i=0; i < m_nNumFaces; i++, pFace++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		vTemp = rStart - pFace->m_Plane.m_Pt;
		fStartDist = vTemp.Dot( pFace->m_Plane.m_Normal );

		vTemp = rEnd - pFace->m_Plane.m_Pt;
		fEndDist = vTemp.Dot( pFace->m_Plane.m_Normal );
			
		// If both points are outside the plane, then there
		// can be no collision with this cell
		if( fEndDist > ON_PLANE_EPSILON &&
			fStartDist > ON_PLANE_EPSILON ) {
			return FALSE;
		}

		// If both points are inside the plane, then we cannot generate the
		// impact point using this plane, so go to the next.
		if( fEndDist < (-ON_PLANE_EPSILON) &&
			fStartDist < (-ON_PLANE_EPSILON) ) {
			continue;
		}
		
		// clamp near the plane to onto the plane surface
		if( fStartDist >= (-ON_PLANE_EPSILON) && 
			fStartDist <= ON_PLANE_EPSILON ) {
			fStartDist = 0.0f;
		}
		if( fEndDist >= (-ON_PLANE_EPSILON) && 
			fEndDist <= ON_PLANE_EPSILON ) {
			fEndDist = 0.0f;
		}

		// Check to see if the ray is parallel			
		if( fEndDist == fStartDist ) {
			continue;
		}
			
		// Determine the ray min and max inside the cell
		fTest = fStartDist / (fStartDist - fEndDist);
		if( fStartDist > 0 && fTest < fMax ) {
			fMax = fTest;
		} else if( fTest > fMin ) {
			fMin = fTest;
		}

		// If the min is ever greater than the max, then the ray is outside the cell			
		if( fMin > fMax ) {
			return FALSE;
		}
	}
	return TRUE;
}

// returns:
// TRUE if an impact point was found, rImpactPt will contain the impact against rnFaceNum
// FALSE if no impace pt could be found
//
// NOTE: this function assumes that rEdge does collide with the cell, very expensive if that is not the case.
BOOL CPortalData_Cell::ClipEdgeToCell( const CPortalData_Edge &rEdge, CFVec3 &rImpactPt, s32 &rnFaceNum ) const {
	f32 fStartDist, fEndDist;

	const CFVec3 *pStart = &rEdge.m_apPoints[0]->m_Pos;
	const CFVec3 *pEnd = &rEdge.m_apPoints[1]->m_Pos;

	// This code is a modified "Slabs Method" which is basically the same as 3D clipping code:
	u32 i;
	CFVec3 vTemp;
	f32 fTest, fDelta, fBest = 2.0f;
	const CPortalData_Face *pFace;
	rnFaceNum = -1;

	for( i=0; i < m_nNumFaces; i++, pFace++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		vTemp = *pStart - pFace->m_Plane.m_Pt;
		fStartDist = vTemp.Dot( pFace->m_Plane.m_Normal );

		vTemp = *pEnd - pFace->m_Plane.m_Pt;
		fEndDist = vTemp.Dot( pFace->m_Plane.m_Normal );
			
		// If both points are outside the plane, then there
		// can be no collision with this cell
		if( fEndDist > ON_PLANE_EPSILON &&
			fStartDist > ON_PLANE_EPSILON ) {
			return FALSE;
		}

		// If both points are inside the plane, then we cannot generate the
		// impact point using this plane, so go to the next.
		if( fEndDist < (-ON_PLANE_EPSILON) &&
			fStartDist < (-ON_PLANE_EPSILON) ) {
			continue;
		}
		
		// Check to see if the ray is parallel			
		if( fEndDist == fStartDist ) {
			continue;
		}

		// the end pts should be on different sides of the plane to generate an impact pt
		if( fEndDist < 0.0f && fStartDist > 0.0f ) {
			// the pts are on opposite sides of the plane, generate an impact pt
			fDelta = fStartDist - fEndDist;
			fTest = fStartDist / fDelta;
		} else if( fEndDist > 0.0f && fStartDist < 0.0f ) {
			// the pts are on opposite sides of the plane, generate an impact pt
			fDelta = fEndDist - fStartDist;
			fTest = -fStartDist / fDelta;			
		} else {
			// the pts aren't on opposite sides of the plane
			continue;
		}
		// take the closest
		if( fTest < fBest ) {
			fBest = fTest;
			rnFaceNum = i;
		}
	}

	if( rnFaceNum < 0 ) {
		// never found an intersection
		return FALSE;
	}

	rImpactPt.ReceiveLerpOf( fBest, *pStart, *pEnd );

	return TRUE;
}

// returns:
// TRUE if any edge of *this intersects rCell
// FALSE if no edge of *this intersects rCell
BOOL CPortalData_Cell::DoesAnyEdgeIntersectCell( const CPortalData_Cell &rCell ) const {
	u32 i;
	CPortalData_Edge *pEdge;

	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		if( rCell.DoesSegmentIntersectCell( pEdge->m_apPoints[0]->m_Pos, pEdge->m_apPoints[1]->m_Pos ) ) {
			return TRUE;
		}
	}
	return FALSE;
}

// it is assumed that rCell.DoesAnyEdgeIntersectCell( *this ) returns FALSE
BOOL CPortalData_Cell::FindInOutAndCrossingEdge( const CPortalData_Cell &rCell,
												 s32 &rnInEdgeIndex, s32 &rnOutEdgeIndex, CPortalData_Face **ppFace ) const {
	PortalData_Loc_e nEdgeLoc, nP1Loc, nP2Loc;
	u32 i;
	CPortalData_Edge *pEdge;
	
	rnInEdgeIndex = -1;
	rnOutEdgeIndex = -1;
	
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		nEdgeLoc = rCell.EdgeToCell( *pEdge, nP1Loc, nP2Loc );

		if( nEdgeLoc == LOC_OUT ) {
			// we only want out edges where both pts are actually out
			if( nP1Loc == LOC_OUT && nP2Loc == LOC_OUT ) {
				rnOutEdgeIndex = i;
			}
		} else if( nEdgeLoc == LOC_IN ) {
			rnInEdgeIndex = i;
		}
		
		if( rnOutEdgeIndex > -1 && rnInEdgeIndex > -1 ) {
			// all edges have been found
			break;
		}
	}

	if( rnOutEdgeIndex > -1 && rnInEdgeIndex > -1 ) {
		// compute the midpt of the in edge and the out edge, create an edge between them
		CPortalData_Edge Edge;
		CPortalData_Point P1, P2;
		CFVec3 ImpactPt;
		s32 nFaceIndex;

		// get the midpt of the in edge
		pEdge = (CPortalData_Edge *)m_apEdges[rnInEdgeIndex];
		P1.m_Pos.ReceiveLerpOf( 0.5f, pEdge->m_apPoints[0]->m_Pos, pEdge->m_apPoints[1]->m_Pos );

		// get the midpt of the out edge
		pEdge = (CPortalData_Edge *)m_apEdges[rnOutEdgeIndex];
		P2.m_Pos.ReceiveLerpOf( 0.5f, pEdge->m_apPoints[0]->m_Pos, pEdge->m_apPoints[1]->m_Pos );
		
		// create a temp edge
		Edge.Create( P1, P2 );

		if( !rCell.ClipEdgeToCell( Edge, ImpactPt, nFaceIndex ) ) {
			FASSERT_NOW;// this cell must not be convex!!!
			return FALSE;
		}

		*ppFace = (CPortalData_Face *)rCell.m_apFaces[nFaceIndex];
		return TRUE;
	}

	return FALSE;
}

// return TRUE if one of the pts that makes up rCell is inside (or ON if bTreatOnAsIn == TRUE) this cell.
// FALSE if all of rCell's pts are either outside or on (except if bTreatOnAsIn == TRUE) the plane of this cell.
BOOL CPortalData_Cell::IsAnyPtInsideCell( const CPortalData_Cell &rCell, BOOL bTreatOnAsIn/*=FALSE*/ ) const {
	const CPortalData_Point *pPoint;
	PortalData_Loc_e nLoc;
	f32 fDist;
	u32 i;

	for( i=0; i < rCell.m_nNumPts; i++ ) {
		pPoint = (CPortalData_Point *)rCell.m_apPts[i];

		 nLoc = PointToCell( pPoint->m_Pos, fDist );

		 if( nLoc == LOC_IN ) {
			 return TRUE;
		 }
		 if( bTreatOnAsIn && nLoc == LOC_ON ) {
			 return TRUE;
		 }
	}

	return FALSE;
}

u32 CPortalData_Cell::GetListOfEdgesOnThePlane( CPtrArray &rEdges ) const {
	u32 i, nNumEdges;
	CPortalData_Edge *pEdge;

	rEdges.RemoveAll();
	nNumEdges = 0;
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];
		
		if( pEdge->m_Loc == LOC_ON ) {
			rEdges.Add( pEdge );
			nNumEdges++;
		}
	}
	FASSERT( (int)nNumEdges == rEdges.GetSize() );

	return nNumEdges;
}

void CPortalData_Cell::CalculateBoundingBox( CFVec3 &rMin, CFVec3 &rMax ) const {
	u32 i;
	CPortalData_Point *pPt;
	
	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];

		if( i > 0 ) {
			rMin.Min( pPt->m_Pos );
			rMax.Max( pPt->m_Pos );
		} else {
			rMin = pPt->m_Pos;
			rMax = pPt->m_Pos;
		}
	}
}

// NOTE: IT IS ASSUMED THAT THE CELL HAS ALREADY BEEN CLIPPED BEFORE CALLING THIS FUNCTION
void CPortalData_Cell::FindDeepestPtsFromPlane( const CPortalData_Plane &rPlane,
											    f32 &rfFrontDepth2, f32 &rfBackDepth2 ) const {
	u32 i;
	CPortalData_Point *pPt, *pBehind, *pInFront;
	CFVec3 Vec;
	
	rfFrontDepth2 = 0.0f;
	rfBackDepth2 = 0.0f;
	pBehind = NULL;
	pInFront = NULL;
	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];

		switch( pPt->m_Loc ) {
		case LOC_OUT:
			if( pPt->m_fDistToPlane > rfFrontDepth2 ) {
				pInFront = pPt;
				rfFrontDepth2 = pPt->m_fDistToPlane;
			}
			break;
		case LOC_IN:
			if( pPt->m_fDistToPlane > rfBackDepth2 ) {
				pBehind = pPt;
				rfBackDepth2 = pPt->m_fDistToPlane;
			}
			break;
		} 
	}
	FASSERT( pInFront && pBehind );	
}

// returns:
// TRUE if no face needed auto portaling, or if was done successfully.
// FALSE if there was a problem during the auto portaling process.
//
// NOTE: IT IS ASSUMED THAT THE CELL HAS ALREADY BEEN SLICED BEFORE CALLING THIS FUNCTION
BOOL CPortalData_Cell::AutoPortalFromFaceOnPlane( CPtrArray &rapPortals, u32 &rnNumPortals, cchar *pszName/*=NULL*/ ) const {
	u32 i;
	CPortalData_Face *pFace;
	
	// find the ON face
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		if( pFace->m_Loc == LOC_ON ) {
			// we found the face that we should use to auto portal
			break;
		}
	}
	if( i >= m_nNumFaces ) {
		// we never found a face to auto portal from
		return TRUE;
	}

	// AUTO PORTAL
	return AutoPortalFromFace( pFace, rapPortals, rnNumPortals, pszName );	
}

// returns:
// TRUE if auto portaling was needed (one already existed nearby), or if was done successfully.
// FALSE if there was a problem during the auto portaling process.
BOOL CPortalData_Cell::AutoPortalFromFace( const CPortalData_Face *pFace, CPtrArray &rapPortals, 
										   u32 &rnNumPortals, cchar *pszName/*=NULL*/ ) {
	u32 i;
	CPortalData_Portal *pPort, *pNewPort;

	// allocate a portal
	pNewPort = new CPortalData_Portal;
	if( !pNewPort ) {
		return FALSE;
	}

	// create the portal from the face
	pNewPort->Create( *pFace );

	// make sure that the new portal has a bounding box of at least a min thickness
	CFVec3 Thickness;
	f32 fDelta;
	Thickness = pNewPort->m_Max - pNewPort->m_Min;
	if( Thickness.x < PORTALDATA_SMALL_PORTAL_DISTANCE ) {
		fDelta = PORTALDATA_SMALL_PORTAL_DISTANCE - Thickness.x;
		fDelta *= 0.5f;
		pNewPort->m_Max.x += fDelta;
		pNewPort->m_Min.x -= fDelta;
	}
	if( Thickness.y < PORTALDATA_SMALL_PORTAL_DISTANCE ) {
		fDelta = PORTALDATA_SMALL_PORTAL_DISTANCE - Thickness.y;
		fDelta *= 0.5f;
		pNewPort->m_Max.y += fDelta;
		pNewPort->m_Min.y -= fDelta;
	}
	if( Thickness.z < PORTALDATA_SMALL_PORTAL_DISTANCE ) {
		fDelta = PORTALDATA_SMALL_PORTAL_DISTANCE - Thickness.z;
		fDelta *= 0.5f;
		pNewPort->m_Max.z += fDelta;
		pNewPort->m_Min.z -= fDelta;
	}

	// see if there already is a portal in this location
	for( i=0; i < rnNumPortals; i++ ) {
		pPort = (CPortalData_Portal *)rapPortals[i];

		if( pPort->DoesPortalIntersect( *pNewPort ) ) {
			// we shouldn't add auto portal since there is a placed portal already nearby
			delete pNewPort;
			return TRUE;
		}
	}

	// allocate memory to store the name
	if( pszName ) {
		i = fclib_strlen( pszName );
		char *pszNewName = new char[ i+1 ];
		if( pszNewName ) {
			fclib_strncpy( pszNewName, pszName, i+1 );
			pNewPort->m_pszName = pszNewName;
			pNewPort->m_bDeleteNameString = TRUE;
		}
	}

	// add the portal to the list
	rapPortals.Add( pNewPort );
	rnNumPortals++;

	return TRUE;
}

s32 CPortalData_Cell::FindIndexFromPtr( const void *pPtr, u32 nNumElements, const CPtrArray &rArray ) {
	u32 i;

	for( i=0; i < nNumElements; i++ ) {
		if( (u32)pPtr == (u32)rArray[i] ) {
			return i;
		}
	}

	return -1;
}

// returns:
// 0 if the two cells don't intersect
// 1 if A intersects B
// 2 if B intersects A
// -1 if the two cells have edges that intersect each other
// -2 if A doesn't have an in, out, and crossing edge in B
// -3 if B doesn't have an in, out, and crossing edge in A
// -4 if the 2 cells overlap, but couldn't determine who intersects who
//
// NOTE: if return value is greater than 0, then the EdgeIndices and ppFace will be filled in properly.
s32 CPortalData_Cell::DoCellsIntersect( CPortalData_Cell &rCellA, CPortalData_Cell &rCellB,
									    s32 &rnInEdgeIndex, s32 &rnOutEdgeIndex, CPortalData_Face **ppFace,
										BOOL bTestForPlanesBetweenCells/*=TRUE*/ ) {
	
	// test the cell's bounding spheres
	if( !rCellA.m_BoundingSphere.IsIntersecting( rCellB.m_BoundingSphere ) ) {
		// bounding sphere aren't intersecting, the cells can't intersect
		return 0;
	}

	if( bTestForPlanesBetweenCells ) {
		// test for a plane between the 2 cells
		CPortalData_Plane Plane;
		BOOL bIsPlaneAFace;

		if( rCellA.DoesPlaneExistBetween( rCellB, Plane, bIsPlaneAFace ) ) {
			// these cells can't intersect, a plane exists between them
			return 0;
		}
	}

	// see if A intersects B
	if( rCellA.DoesAnyEdgeIntersectCell( rCellB ) ) {
		// 1) rCellA must have 1 edge inside rCellB
		// 2) 1 edge outside rCellB
		// 3) B must not have any intersecting edges to rCellA
		if( rCellB.DoesAnyEdgeIntersectCell( rCellA ) ) {
			// B can't intersect A
			return -1;
		}
		if( !rCellA.FindInOutAndCrossingEdge( rCellB, rnInEdgeIndex, rnOutEdgeIndex, ppFace ) ) {
			// can't find the required edges
			return -2;
		}

		// A intersects B properly		
		return 1;

	} else if( rCellB.DoesAnyEdgeIntersectCell( rCellA ) ) {
		// 1) rCellB must have 1 edge inside rCellA
		// 2) 1 edge outside rCellA
		// 3) ALREADY MET: rCellA must not have any intersecting edges to rCellB
		if( !rCellB.FindInOutAndCrossingEdge( rCellA, rnInEdgeIndex, rnOutEdgeIndex, ppFace ) ) {
			// can't find the required edges
			return -3;
		}

		// B intersects A properly
		return 2;
	}

	// see if any pts of either cell are inside the other cell
	if( rCellA.IsAnyPtInsideCell( rCellB, FALSE ) ) {
		return -4;
	}
	if( rCellB.IsAnyPtInsideCell( rCellA, FALSE ) ) {
		return -4;
	}

	// finally check for perfect cell overlap, where both cells have a face ON the other cell,
	// but since there is no plane between the cells, this must be an error
	//
	// TBD...

	return 0;	
}

BOOL CPortalData_Cell::AllocateMemory( u32 nNumFaces, u32 nNumEdges, u32 nNumPts ) {
	u32 i;
	CPortalData_Point *pPt;
	CPortalData_Edge *pEdge;
	CPortalData_Face *pFace;

	// free any existing allocated memory
	Empty();

	// allocate the pt array
	for( i=0; i < nNumPts; i++ ) {
		pPt = new CPortalData_Point;
		if( !pPt ) {
			goto _FAILURE;
		}
		m_apPts.Add( pPt );
		m_nNumPts++;
	}

	// allocate the face array
	for( i=0; i < nNumFaces; i++ ) {
		pFace = new CPortalData_Face;
		if( !pFace ) {
			goto _FAILURE;
		}
		m_apFaces.Add( pFace );
		m_nNumFaces++;
	}

	// allocate the edge array
	for( i=0; i < nNumEdges; i++ ) {
		pEdge = new CPortalData_Edge;
		if( !pEdge ) {
			Empty();
			return FALSE;
		}
		m_apEdges.Add( pEdge );
		m_nNumEdges++;
	}

	return TRUE;

_FAILURE:
	Empty();
	return FALSE;
}

void CPortalData_Cell::CalculateLocDataFromPlane( const CPortalData_Plane &rPlane ) {
	u32 i, j;
	CPortalData_Point *pPt;
	CPortalData_Edge *pEdge;
	PortalData_Loc_e nLoc1, nLoc2;
	CPortalData_Face *pFace;

	// find out where all of the pts are located relative to the plane
	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];
		
		pPt->m_Loc = rPlane.WhichSideOfPlane( pPt->m_Pos, pPt->m_fDistToPlane );
	}
	
	// run through the edges and assign their location relative to the plane
	for( i=0; i < m_nNumEdges; i++ ) {
		pEdge = (CPortalData_Edge *)m_apEdges[i];

		nLoc1 = pEdge->m_apPoints[0]->m_Loc;
		nLoc2 = pEdge->m_apPoints[1]->m_Loc;
		
		if( nLoc1 == nLoc2 ) {
			// whatever side they are on, both pts are in the same state
			pEdge->m_Loc = nLoc1;
			continue;
		}
		if( nLoc1 == LOC_ON ) {
			// if pt1 is on, mark the edge as wherever pt2 is
			pEdge->m_Loc = nLoc2;
			continue;
		}
		if( nLoc2 == LOC_ON ) {
			// if pt2 is on, mark the edge as wherever pt1 is
			pEdge->m_Loc = nLoc1;
			continue;
		}

		// the 2 pts must be on different sides of the plane
		pEdge->m_Loc = LOC_IN_AND_OUT;
	}

	// run through the faces and assign their location
	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		for( j=0; j < pFace->m_nNumEdges; j++ ) {
			pEdge = (CPortalData_Edge *)pFace->m_apEdges[j];

			if( j > 0 ) {
				switch( pEdge->m_Loc ) {
				case LOC_OUT:
					if( pFace->m_Loc == LOC_IN ) {
						pFace->m_Loc = LOC_IN_AND_OUT;
					} else if( pFace->m_Loc == LOC_ON ) {
						pFace->m_Loc = LOC_OUT;
					}
					break;
				case LOC_IN:
					if( pFace->m_Loc == LOC_OUT ) {
						pFace->m_Loc = LOC_IN_AND_OUT;
					} else if( pFace->m_Loc == LOC_ON ) {
						pFace->m_Loc = LOC_IN;
					}
					break;
				case LOC_ON:
					// stick with whatever is currently set
					break;
				case LOC_IN_AND_OUT:
					pFace->m_Loc = pEdge->m_Loc;
					break;
				}
			} else {
				// put us wherever the 1st edge is locationed
				pFace->m_Loc = pEdge->m_Loc;
			}

			if( pFace->m_Loc == LOC_IN_AND_OUT ) {
				break;
			}
		}
	}
}

// returns:
// -1 if no face is coplanar with rPlane
// the face index of the face that is coplanar with rPlane
s32 CPortalData_Cell::FindCoPlanarFace( const CPortalData_Plane &rPlane ) const {
	u32 i;
	CPortalData_Face *pFace;
	f32 fDist;

	for( i=0; i < m_nNumFaces; i++ ) {
		pFace = (CPortalData_Face *)m_apFaces[i];

		if( rPlane.WhichSideOfPlane( pFace->m_Centroid, fDist ) == LOC_ON ) {
			return i;
		}
	}
	return -1;
}

void CPortalData_Cell::CalculateCentroid( CFVec3 &rCenter ) const {
	u32 i;
	CPortalData_Point *pPt;
	
	rCenter.Zero();
	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];

		rCenter += pPt->m_Pos;	
	}
	if( m_nNumPts ) {
		rCenter *= (1.0f/(f32)m_nNumPts);
	}
}

f32 CPortalData_Cell::CalculateFurthestDistFromPt( const CFVec3 &rPt ) const {
	f32 fDist2, fMag2 = 0;
	u32 i;
	CPortalData_Point *pPt;
	CFVec3 Temp;

	for( i=0; i < m_nNumPts; i++ ) {
		pPt = (CPortalData_Point *)m_apPts[i];

		Temp = rPt - pPt->m_Pos;
		fDist2 = Temp.Mag2();
		if( fDist2 > fMag2 ) {
			fMag2 = fDist2;
		}
	}
	return fmath_AcuSqrt( fMag2 );	
}

////////////////// 
// CPortalData_Vol
//////////////////

CPortalData_Vol::CPortalData_Vol() {
	m_nNumCells = 0;
	m_apCells.RemoveAll();
}

CPortalData_Vol::~CPortalData_Vol() {
	Empty();
}

BOOL CPortalData_Vol::Create( const ApeVisVolume_t &rVol ) {
	u32 i;
	CPortalData_Cell *pCell;

	Empty();

	for( i=0; i < rVol.nNumCells; i++ ) {
		// allocate a cell
		pCell = new CPortalData_Cell;
		if( !pCell ) {
			Empty();
			return FALSE;
		}
		m_apCells.Add( pCell );
		m_nNumCells++;

		if( !pCell->Create( rVol.aCells[i] ) ) {
			// could not create a cell from the ape cell
			Empty();
			return FALSE;
		}
	}
#if 0
	m_BoundingSphere = rVol.Sphere;
	// pad the bounding sphere radius
	m_BoundingSphere.m_fRadius *= UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;
#else
	RecomputeBoundingSphere();
#endif

	return TRUE;
}

// returns:
// LOC_IN if rPt is inside 1 of the cells that make up this volume
// LOC_OUT if rPt is outside all cells that make up this volume
// LOC_ON if rPt is on the face of one of the cells that make up this volume
PortalData_Loc_e CPortalData_Vol::IsPtInsideVolume( const CFVec3 &rPt ) const {
	u32 i;
	CPortalData_Cell *pCell;
	PortalData_Loc_e nLoc;
	f32 fDist;

	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		nLoc = pCell->PointToCell( rPt, fDist );
		if( nLoc == LOC_IN || nLoc == LOC_ON ) {
			return nLoc;
		}
	}

	return LOC_OUT;
}

// returns:
// the number of faces that make up the cells of this volume.
u32 CPortalData_Vol::GetNumOfCellFaces() const {
	u32 i, nFaceCount;
	CPortalData_Cell *pCell;
	
	nFaceCount = 0;
	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		nFaceCount += pCell->m_nNumFaces;
	}

	return nFaceCount;
}

// returns:
// TRUE if clipping was done or none was needed
// FALSE if there was a problem (rsErrorLine will contain the error line)
BOOL CPortalData_Vol::ClipOverlappingCells( CString &rsErrorLine ) {
	CPortalData_Cell *pCellA, *pCellB, *pMaleCell, *pFemaleCell;
	u32 i, j;
	CPortalData_Plane Plane;
	s32 nInEdge, nOutEdge;
	BOOL bClip, bRecalculateBSphere = FALSE;
	CPortalData_Face *pFace;
	
	if( m_nNumCells < 2 ) {
		// can't overlap if we don't have at least 2 cells
		return TRUE;
	}
	
	for( i=0; i < (m_nNumCells-1); i++ ) {
		pCellA = (CPortalData_Cell *)m_apCells[i];

		for( j=(i+1); j < m_nNumCells; j++ ) {
			pCellB = (CPortalData_Cell *)m_apCells[j];

			switch( CPortalData_Cell::DoCellsIntersect( *pCellA, *pCellB, nInEdge, nOutEdge, &pFace ) ) {

			case 0:
				// the cells don't collide, do nothing
				bClip = FALSE;
				break;

			case 1:
				// A intersects B
				bClip = TRUE;
				pMaleCell = pCellA;
				pFemaleCell = pCellB;
				break;

			case 2:
				// B intersects A
				bClip = TRUE;
				pMaleCell = pCellB;
				pFemaleCell = pCellA;
				break;

			//////////////
			// ERROR CASES
			//////////////
			case -1:
				rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
									"\tCell '%s' and '%s' are part of the same volume and both intersect each other.\n"
									"\tFix it so that 1 of these cells properly intersects the other.",
									pCellA->m_pszName, pCellB->m_pszName );
				return FALSE;
				break;

			case -2:
				rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
									"\tCell '%s' must have an edge inside, an edge outside, and only cross 1 face of '%s'.",
									pCellA->m_pszName, pCellB->m_pszName );
				return FALSE;
				break;

			case -3:
				rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
									"\tCell '%s' must have an edge inside, an edge outside, and only cross 1 face of '%s'.",
									pCellB->m_pszName, pCellA->m_pszName );
				return FALSE;
				break;

			case -4:
				rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
									"\tCell '%s' and Cell '%s' overlap, but it is unclear which intersects the other.\n"
									"\tAdjust the cells so that it is clear.", 
									pCellA->m_pszName, pCellB->m_pszName );
				return FALSE;
				break;
				break;

			default:
				rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
									"\tUnknown return code from 'CPortalData_Cell::DoCellsIntersect', that's strange" );
				FASSERT_NOW;
				return FALSE;
				break;
			}

			if( bClip ) {
				// at this point it is always male penatrating female

				// clip the male (ouch!!!)
				if( !pMaleCell->ClipAlongPlane( pFace->m_Plane ) ) {
					rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
										"\tError while clipping Cell '%s' to Cell '%s', check your cells.", pMaleCell->m_pszName, pFemaleCell->m_pszName );
					return FALSE;
				}

				if( !pMaleCell->SliceAlongPlane( pFace->m_Plane, LOC_OUT, TRUE ) ) {
					rsErrorLine.Format( "Intra-Volume Clipping Error:\n"
										"\tError while creating the outside portion of Cell '%s' created by clipping to Cell '%s', double check these cells.", pMaleCell->m_pszName, pFemaleCell->m_pszName );
					return FALSE;
				}

				// mark that we need to recompute the volume's bounding sphere
				bRecalculateBSphere = TRUE;
			}
		}
	}

	if( bRecalculateBSphere ) {
		RecomputeBoundingSphere();
	}

	return TRUE;
}

void CPortalData_Vol::Empty() {
	u32 i;
	CPortalData_Cell *pCell;

	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];
		
		pCell->Empty();

		delete pCell;
	}
	m_apCells.RemoveAll();
	m_nNumCells = 0;
}

void CPortalData_Vol::CalculateBoundingBox( CFVec3 &rMin, CFVec3 &rMax ) const {
	u32 i;
	CPortalData_Cell *pCell;
	CFVec3 CellMin, CellMax;
	
	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		pCell->CalculateBoundingBox( CellMin, CellMax );

		if( i > 0 ) {
			rMin.Min( CellMin );
			rMax.Max( CellMax );
		} else {
			rMin = CellMin;
			rMax = CellMax;
		}
	}
}

void CPortalData_Vol::RecomputeBoundingSphere() {
#if 0
	CFVec3 Min, Max;

	CalculateBoundingBox( Min, Max );
	utils_GetBoundingSphereFromMinMax( m_BoundingSphere.m_Pos, m_BoundingSphere.m_fRadius, Min, Max );
#else
	u32 i;
	CPortalData_Cell *pCell;
	f32 fDist;
	
	// get the center of the bounding sphere
	m_BoundingSphere.m_Pos.Zero();
	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		m_BoundingSphere.m_Pos += pCell->m_BoundingSphere.m_Pos;
	}
	if( m_nNumCells ) {
		m_BoundingSphere.m_Pos *= (1.0f/(f32)m_nNumCells);
	}

	// compute the radius
	m_BoundingSphere.m_fRadius = 0.0f;
	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		fDist = pCell->CalculateFurthestDistFromPt( m_BoundingSphere.m_Pos );
		if( fDist > m_BoundingSphere.m_fRadius ) {
			m_BoundingSphere.m_fRadius = fDist;
		}
	}
#endif
}

// fixes any overlap between rVol1 and rVol2, returns:
// TRUE if clipping was done or none was needed
// FALSE if there was a problem (rsErrorLine will contain the error line)
//
// NOTE: portals may be added to the portal list if during clipping, auto portaling was done.
// Also, each vol should have already been fixed up so that there is no overlap within either one.
BOOL CPortalData_Vol::ClipOverlappingVols( CPortalData_Vol &rVol1, CPortalData_Vol &rVol2, 
										   CString &rsErrorLine, CPtrArray &rapPortals, u32 &rnNumPortals ) {
	u32 i, j, nStrLen;
	CPortalData_Cell *pCellA, *pCellB, *pMaleCell, *pFemaleCell, *pNewCell;
	s32 nInEdge, nOutEdge, nFaceIndex;
	BOOL bClip, bRecalculateVol1BSphere, bRecalculateVol2BSphere;
	CPortalData_Face *pFace, *pFace2;
	f32 fInsideDist2, fOutsideDist2;
	CPortalData_Vol *pVolWhoseCellWasClipped;
	CPortalData_Plane Plane;
	BOOL bIsPlaneAFace;
	CString sName;
	char *pszNewName;

	// first test the 2 volume's bounding spheres
	if( !rVol1.m_BoundingSphere.IsIntersecting( rVol2.m_BoundingSphere ) ) {
		// if the vol bounding spheres don't overlap, then none of the cells do
		return TRUE;
	}

	bRecalculateVol1BSphere = FALSE;
	bRecalculateVol2BSphere = FALSE;

	for( i=0; i < rVol1.m_nNumCells; i++ ) {
		pCellA = (CPortalData_Cell *)rVol1.m_apCells[i];

#if _INSANE_AMOUNT_OF_CHECKING
		// get the name of the 1st cell in the 2nd volume
		pCellB = (CPortalData_Cell *)rVol2.m_apCells[0];
#endif

		// test for full containment
		if( rVol2.IsCellFullyContainedInACellOfVol( *pCellA ) ) {
			// pCellA is fully contained in 1 of the cells of Vol2, leave it alone
			continue;
		}

		// test for cell being completely outside all cells of the other volume
		if( rVol2.IsCellFullyOutsideAllCellsOfVol( *pCellA ) ) {
			// pCellA is completely outside of all cells of Vol2, leave it alone
			continue;
		}

		for( j=0; j < rVol2.m_nNumCells; j++ ) {
			pCellB = (CPortalData_Cell *)rVol2.m_apCells[j];

			// test for full containment
			if( rVol1.IsCellFullyContainedInACellOfVol( *pCellB ) ) {
				// pCellB is fully contained in 1 of the cells of Vol1, leave it alone
				continue;
			}

			// test for cell being completely outside all cells of the other volume
			if( rVol1.IsCellFullyOutsideAllCellsOfVol( *pCellB ) ) {
				// pCellB is completely outside of all cells of Vol1, leave it alone
				continue;
			}

			if( pCellA->DoesPlaneExistBetween( *pCellB, Plane, bIsPlaneAFace ) ) {
				// these cells can't intersect, a plane exists between them, see if we should auto-portal
				if( bIsPlaneAFace ) {
					// find the coplanar face from each cell
					nFaceIndex = pCellA->FindCoPlanarFace( Plane );
					if( nFaceIndex >= 0 ) {
						pFace = (CPortalData_Face *)pCellA->m_apFaces[nFaceIndex];

						nFaceIndex = pCellB->FindCoPlanarFace( Plane );
						if( nFaceIndex >= 0 ) {
							pFace2 = (CPortalData_Face *)pCellB->m_apFaces[nFaceIndex];

							// see if 1 face is completely contained on the face of the other
							if( pFace->AreFacesSandwichedTogether( pFace, pFace2 ) ) {
								// we should auto portal this face
								if( pFace->IsFaceOnCellShell( *pCellB ) ) {
									// auto portal to pFace
									sName.Format( "%s between a face of cell %s and cell %s", _AUTO_PORTAL_NAME, pCellA->m_pszName, pCellB->m_pszName );
									if( !pCellA->AutoPortalFromFace( pFace, rapPortals, rnNumPortals, sName ) ) {
										rsErrorLine.Format( "Auto portaling error:\n"
															"\tError while auto portaling the adjacent non-intersecting Cell '%s' and Cell '%s'.",
															pCellA->m_pszName, pCellB->m_pszName );
										return FALSE;									
									}
								} else if( pFace2->IsFaceOnCellShell( *pCellA ) ) {
									// auto portal to pFace2
									sName.Format( "%s between a face of Cell %s and Cell %s", _AUTO_PORTAL_NAME, pCellB->m_pszName, pCellA->m_pszName );
									if( !pCellB->AutoPortalFromFace( pFace2, rapPortals, rnNumPortals, sName ) ) {
										rsErrorLine.Format( "Auto portaling error:\n"
															"\tError while auto portaling the adjacent non-intersecting Cell '%s' and Cell '%s'.",
															pCellA->m_pszName, pCellB->m_pszName );
										return FALSE;									
									}									
								}
							}
						}
					}
				}
				continue;				
			}

			// figure out if the 2 cells intersect
			switch( CPortalData_Cell::DoCellsIntersect( *pCellA, *pCellB, nInEdge, nOutEdge, &pFace, FALSE ) ) {

			case 0:
				// the cells don't collide, do nothing
				bClip = FALSE;
				break;

			case 1:
				// A intersects B
				bClip = TRUE;
				pMaleCell = pCellA;
				pFemaleCell = pCellB;
				pVolWhoseCellWasClipped = &rVol1;
				// mark that we need to recompute the volume 1's bounding sphere
				bRecalculateVol1BSphere = TRUE;
				break;

			case 2:
				// B intersects A
				bClip = TRUE;
				pMaleCell = pCellB;
				pFemaleCell = pCellA;
				pVolWhoseCellWasClipped = &rVol2;
				// mark that we need to recompute the volume 2's bounding sphere
				bRecalculateVol2BSphere = TRUE;
				break;

			//////////////
			// ERROR CASES
			//////////////
			case -1:
				rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
									"\tCell '%s' and '%s' are part of the different volumes and both intersect each other.\n"
									"\tFix it so that 1 of these cells properly intersects the other.",
									pCellA->m_pszName, pCellB->m_pszName );
				return FALSE;
				break;

			case -2:
				rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
									"\tCell '%s' must have an edge inside, an edge outside, and only cross 1 face of '%s'.",
									pCellA->m_pszName, pCellB->m_pszName );
				return FALSE;
				break;

			case -3:
				rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
									"\tCell '%s' must have an edge inside, an edge outside, and only cross 1 face of '%s'.",
									pCellB->m_pszName, pCellA->m_pszName );
				return FALSE;
				break;

			case -4:
				rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
									"\tCell '%s' and Cell '%s' overlap, but it is unclear which intersects the other.\n"
									"\tAdjust the cells so that it is clear.", 
									pCellA->m_pszName, pCellB->m_pszName );
				return FALSE;
				break;
				break;

			default:
				rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
									"\tUnknown return code from 'CPortalData_Cell::DoCellsIntersect', that's strange" );
				FASSERT_NOW;
				return FALSE;
				break;
			}

			if( bClip ) {
				// at this point it is always male penatrating female

				// clip the male (ouch!!!)
				if( !pMaleCell->ClipAlongPlane( pFace->m_Plane ) ) {
					rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
										"\tError while clipping Cell '%s' to Cell '%s', check your cells.", 
										pMaleCell->m_pszName, pFemaleCell->m_pszName );
					return FALSE;
				}

				// determine if we should throw the inside male portion away (and possibly auto portal) or keep it
				pMaleCell->FindDeepestPtsFromPlane( pFace->m_Plane, fOutsideDist2, fInsideDist2 );
				if( fInsideDist2 >= _KEEP_CLIPPED_CELL_2 ) {
					// keep the inside portion, so make a copy of the clipped cell
					pNewCell = new CPortalData_Cell;
					if( !pNewCell ) {
						rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
											"\tError while clipping Cell '%s' to Cell '%s', could not allocate memory.", 
											pMaleCell->m_pszName, pFemaleCell->m_pszName );
						return FALSE;
					}

					// create an inside copy of the existing clipped cell
					if( !pNewCell->Create( *pMaleCell, pFace->m_Plane, LOC_IN, TRUE ) ) {
						rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
											"\tError while creating the inside portion of Cell '%s' created by clipping to Cell '%s', double check these cells.", 
											pMaleCell->m_pszName, pFemaleCell->m_pszName );
						return FALSE;
					}

					// change the name of the new cell to reflect
					sName.Format( "Clipped Cell - Created by clipping Cell %s to Cell %s", pMaleCell->m_pszName, pFemaleCell->m_pszName );
					nStrLen = sName.GetLength() + 1;
					pszNewName = new char[nStrLen];
					if( pszNewName ) {
						fclib_strncpy( pszNewName, sName, nStrLen );	
						pNewCell->m_pszName = pszNewName;
						pNewCell->m_bDeleteNameString = TRUE;
					}

					// add the new cell to the correct volume
					pVolWhoseCellWasClipped->m_apCells.Add( pNewCell );
					pVolWhoseCellWasClipped->m_nNumCells++;
				}

				// create the outside portion of the cell
				if( !pMaleCell->SliceAlongPlane( pFace->m_Plane, LOC_OUT, TRUE ) ) {
					rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
										"\tError while creating the outside portion of Cell '%s' created by clipping to Cell '%s', double check these cells.", 
										pMaleCell->m_pszName, pFemaleCell->m_pszName );
					return FALSE;
				}

				if( fInsideDist2 < _KEEP_CLIPPED_CELL_2 ) {
					// try to auto portaling about the clipped face
					sName.Format( "%s between clipped Cell %s and %s", _AUTO_PORTAL_NAME, pMaleCell->m_pszName, pFemaleCell->m_pszName );
					if( !pMaleCell->AutoPortalFromFaceOnPlane( rapPortals, rnNumPortals, sName ) ) {
						rsErrorLine.Format( "Inter-Volume Clipping Error:\n"
											"\tError while auto portaling the intersection of Cell '%s' and Cell '%s', double check these cells.", 
											pMaleCell->m_pszName, pFemaleCell->m_pszName );
						return FALSE;
					}
				}
			}
		}
	}

	if( bRecalculateVol1BSphere ) {
		rVol1.RecomputeBoundingSphere();
	}
	if( bRecalculateVol2BSphere ) {
		rVol2.RecomputeBoundingSphere();
	}

	return TRUE;
}

// returns:
// TRUE if rCell is fully contained in 1 of the cells of this volume.
// FALSE if it is not fully contianed
BOOL CPortalData_Vol::IsCellFullyContainedInACellOfVol( const CPortalData_Cell &rCell ) const {
	u32 i, j;
	CPortalData_Cell *pCell;
	CPortalData_Point *pPt;
	PortalData_Loc_e nLoc;
	f32 fDist;

	// walk all of the cells of this volume
	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		// walk all of the pts of rCell
		for( j=0; j < rCell.m_nNumPts; j++ ) {
			pPt = (CPortalData_Point *)rCell.m_apPts[j];

			// find out where the pt is to the cell
			nLoc = pCell->PointToCell( pPt->m_Pos, fDist );
			if( !( nLoc == LOC_ON || nLoc == LOC_IN ) ) {
				// this pt is not inside the cell
				break;
			}
		}
		if( j == rCell.m_nNumPts ) {
			// all pts were on or inside the cell
			return TRUE;
		}
	}
	return FALSE;
}

// ruturns:
// TRUE if all the edges of rCell are outside of all cells of this volume and
//		no point of this volume is inside (or on) rCell.
// FALSE if rCell crosses at least 1 cell of this volume.
BOOL CPortalData_Vol::IsCellFullyOutsideAllCellsOfVol( const CPortalData_Cell &rCell ) const {
	u32 i, j;
	CPortalData_Cell *pCell;
	CPortalData_Edge *pEdge;
	PortalData_Loc_e nLoc;
	
	// walk all of the cells in this volume
	for( i=0; i < m_nNumCells; i++ ) {
		pCell = (CPortalData_Cell *)m_apCells[i];

		// walk all the edges in rCell
		for( j=0; j < rCell.m_nNumEdges; j++ ) {
			pEdge = (CPortalData_Edge *)rCell.m_apEdges[j];

			nLoc = pCell->EdgeToCell( *pEdge );
			if( nLoc != LOC_OUT ) {
				// this edge is not outside pCell
				return FALSE;
			}
		}

		// see if any of pCell's points are inside rCell
		if( rCell.IsAnyPtInsideCell( *pCell, TRUE ) ) {
			return FALSE;
		}
	}

	// if we made it here, no edge of rCell was anything but OUT of every cell of this volume
	return TRUE;
}

/////////////////////
// CPortalData_Portal
/////////////////////

CPortalData_Portal::CPortalData_Portal() {
	m_pszName = NULL;
	m_bDeleteNameString = FALSE;
	m_nFlags = FVIS_PORTAL_FLAG_NONE;
}

CPortalData_Portal::~CPortalData_Portal() {

	if( m_bDeleteNameString ) {
		delete [] (char *)m_pszName;
		m_pszName = NULL;
		m_bDeleteNameString = FALSE;
	}
}

void CPortalData_Portal::Create( const CPortalData_Face &rFace ) {
	CFMtx33 Mtx, InvMtx;
	CPortalData_Point *pPt;
	CPortalData_Edge *pEdge;
	CFVec3 Temp, Right, Up, Temp2;
	u32 i, nNumPts;
	CPtrArray apPts;
	f32 fDot;
		
	FASSERT( rFace.m_nNumEdges > 1 );

	// grab a list of the unique pts that make up this face
	nNumPts = rFace.GetPtList( apPts );
	FASSERT( nNumPts > 2 );

	if( nNumPts != 4 ) {
		// use the longest edge as our up vec
		pEdge = (CPortalData_Edge *)rFace.GetLongestEdge();
		Mtx.m_vUp =  pEdge->m_apPoints[0]->m_Pos - pEdge->m_apPoints[1]->m_Pos;
		Mtx.m_vUp.Unitize();
		Mtx.m_vFront = rFace.m_Plane.m_Normal;
		Mtx.m_vRight = Mtx.m_vUp.Cross( Mtx.m_vFront );
		
		InvMtx = Mtx.GetInverse();

		for( i=0; i < nNumPts; i++ ) {
			pPt = (CPortalData_Point *)apPts[i];
			
			Temp = InvMtx * pPt->m_Pos;
			if( i > 0 ) {
				m_Min.Min( Temp );
				m_Max.Max( Temp );
			} else {
				m_Min = Temp;
				m_Max = Temp;
			}	
		}
		
		m_Min = Mtx * m_Min;
		m_Max = Mtx * m_Max;
		Temp = m_Max - m_Min;
		
		// fill in our data fields
		m_Centroid = (m_Min + m_Max) * 0.5f;

		Right = Mtx.m_vRight * Temp.Dot( Mtx.m_vRight );
		Up = Mtx.m_vUp * Temp.Dot( Mtx.m_vUp );

		m_aCorners[0] = m_Centroid;
		m_aCorners[0] += (Right * -0.5f);
		m_aCorners[0] += (Up * -0.5f);

		m_aCorners[1] = m_aCorners[0];
		m_aCorners[1] += Up;

		m_aCorners[2] = m_aCorners[1];
		m_aCorners[2] += Right;

		m_aCorners[3] = m_aCorners[0];
		m_aCorners[3] += Right;

		m_Normal = -rFace.m_Plane.m_Normal;
	} else {
		// just use the 4 pts of the face
		for( i=0; i < nNumPts; i++ ) {
			pPt = (CPortalData_Point *)apPts[i];

			m_aCorners[i] = pPt->m_Pos;
		}
		// test the winding
		Up = m_aCorners[1] - m_aCorners[0];
		Right =  m_aCorners[2] - m_aCorners[0];
		Temp = Up.Cross( Right );

		Up = Right;
		Right =  m_aCorners[3] - m_aCorners[0];
		Temp2 = Up.Cross( Right );

		fDot = Temp.Dot( Temp2 );
		if( fDot < 0.0f ) {
			// swap the 2nd and 3rd pts
			Temp = m_aCorners[2];
			m_aCorners[2] = m_aCorners[3];
			m_aCorners[3] = Temp;			
		}

		// compute the normal
		Up = m_aCorners[1] - m_aCorners[0];
		Right =  m_aCorners[2] - m_aCorners[0];
		m_Normal = Up.Cross( Right );
		m_Normal.Unitize();
	}

	Temp.Zero();
	for( i=0; i < 4; i++ ) {
	
		if( i > 0 ) {
			m_Min.Min( m_aCorners[i] );
			m_Max.Max( m_aCorners[i] );
		} else {
			m_Min = m_aCorners[i];
			m_Max = m_aCorners[i];
		}
		Temp += m_aCorners[i];
	}
	Temp *= (1.0f/4.0f);
	m_Centroid = Temp;

	m_pszName = _AUTO_PORTAL_NAME;
	m_bDeleteNameString = FALSE;

	// this is an regular autoportal, flag it as such
	m_nFlags = FVIS_PORTAL_FLAG_AUTOPORTAL | FVIS_PORTAL_FLAG_VISIBILITY | FVIS_PORTAL_FLAG_SOUND;
}

void CPortalData_Portal::Create( const ApeVisPortal_t &rPortal ) {
	u32 i;

	m_pszName = rPortal.szName;
	m_bDeleteNameString = FALSE;
	
	for( i=0; i < 4; i++ ) {
		m_aCorners[i] = rPortal.aCorners[i];

		if( i > 0 ) {
			m_Min.Min( m_aCorners[i] );
			m_Max.Max( m_aCorners[i] );
		} else {
			m_Min = m_aCorners[i];
			m_Max = m_aCorners[i];
		}

	}

	m_Normal = rPortal.Normal;

	m_Centroid = rPortal.Centroid;

	// figure out what flags should be set
	m_nFlags = FVIS_PORTAL_FLAG_VISIBILITY | FVIS_PORTAL_FLAG_SOUND;
	if( rPortal.nFlags & APE_PORTAL_FLAG_MIRROR ) {
		// mirrors are 1 sided and do not support visibility or sound
		m_nFlags |= (FVIS_PORTAL_FLAG_MIRROR | FVIS_PORTAL_FLAG_ONE_WAY);
		m_nFlags &= ~(FVIS_PORTAL_FLAG_VISIBILITY | FVIS_PORTAL_FLAG_SOUND);
	}
	if( rPortal.nFlags & APE_PORTAL_FLAG_SOUND_ONLY ) {
		// Sound only does not support visibility
		m_nFlags &= ~FVIS_PORTAL_FLAG_VISIBILITY;
	}
	if( rPortal.nFlags & APE_PORTAL_FLAG_ONE_WAY ) {
		m_nFlags |= FVIS_PORTAL_FLAG_ONE_WAY;
	}
	if( rPortal.nFlags & APE_PORTAL_FLAG_ANTI ) {
		// Anti-portals do not support visibility or sound
		m_nFlags |= FVIS_PORTAL_FLAG_ANTIPORTAL;
		m_nFlags &= ~(FVIS_PORTAL_FLAG_VISIBILITY | FVIS_PORTAL_FLAG_SOUND);
	}
}

// returns:
// TRUE if this portal's centroid intersects with rPortal's bounding box
// FALSE if the 2 do not intersect
BOOL CPortalData_Portal::DoesPortalIntersect( const CPortalData_Portal &rPortal ) const {

	if( m_Centroid.x < rPortal.m_Min.x ||
		m_Centroid.y < rPortal.m_Min.y ||
		m_Centroid.z < rPortal.m_Min.z ) {
		// the 2 portals couldn't possibly intersect
		return FALSE;
	}
	if( m_Centroid.x > rPortal.m_Max.x ||
		m_Centroid.y > rPortal.m_Max.y ||
		m_Centroid.z > rPortal.m_Max.z ) {
		// the 2 portals couldn't possibly intersect
		return FALSE;
	}

	// the portals must intersect
	return TRUE;
}

void CPortalData_Portal::ComputeBoundingSphere( CFSphere &rSphere ) const {

	rSphere.m_Pos = m_Centroid;
	u32 i;
	f32 fMag2, fDist2 = 0.0f;
	CFVec3 Temp;
	for( i=0; i < 4; i++ ) {
		Temp = m_aCorners[i] - m_Centroid;
		fMag2 = Temp.Mag2();
		if( fMag2 > fDist2 ) {
			fDist2 = fMag2;
		}
	}
	rSphere.m_fRadius = fmath_AcuSqrt( fMag2 );
	rSphere.m_fRadius *= UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;
}
