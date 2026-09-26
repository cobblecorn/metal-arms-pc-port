//////////////////////////////////////////////////////////////////////////////////////
// PortalData.h - 
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
#ifndef _PORTAL_DATA_H_
#define _PORTAL_DATA_H_ 1

#include "fang.h"
#include "ape_file_def.h"

#define PORTALDATA_SMALL_PORTAL_DISTANCE	0.75f	// how far + & - along a portal's normal to check for volume ids

typedef enum {
	LOC_UNKNOWN = -1,
	LOC_OUT = 0,
	LOC_IN = 1,
	LOC_ON = 2,
	LOC_IN_AND_OUT = 3	
} PortalData_Loc_e;

// forward declarations:
class CPortalData_Point;
class CPortalData_Plane;
class CPortalData_Face;
class CPortalData_Edge;
class CPortalData_Cell;
class CPortalData_Vol;
class CPortalData_Portal;


class CPortalData_Plane
{
public:
	// data:
	CFVec3 m_Pt;
	CFVec3 m_Normal;

	// interface:
	void Create( const ApeVisFaces_t &rFace );
	PortalData_Loc_e WhichSideOfPlane( const CFVec3 &rPt, f32 &rfDistToPlane ) const;
};

class CPortalData_Face
{
public:
	// data:
	CPortalData_Plane m_Plane;// the normal point outward
	
	CFVec3 m_Centroid;
	
	u32 m_nNumEdges;
	CPtrArray m_apEdges;// CPortalData_Edge * contained in the cell

	PortalData_Loc_e m_Loc;// what side of a cutting plane is this on

	// interface:
	// will create a list of unique pts that make up this face
	u32 GetPtList( CPtrArray &rPts ) const;	
	u32 GetListOfPtsOnThePlane( CPtrArray &rPts ) const;
	void RecomputeCentroid();
	static BOOL AreFacesSandwichedTogether( const CPortalData_Face *pFace1, const CPortalData_Face *pFace2 );
	BOOL RemoveEdge( CPortalData_Edge *pEdge );
	BOOL IsFaceOnCellShell( const CPortalData_Cell &rCell ) const;
	CPortalData_Edge *GetLongestEdge() const;
};

class CPortalData_Point
{
public:
	// data:
	CFVec3 m_Pos;
	
	PortalData_Loc_e m_Loc;// what side of a cutting plane is this on
	f32 m_fDistToPlane;// how close is this point to the cutting plane

	// interface:
	void Create( const ApeVisPt_t &rPt );
	static f32 GetDist2BetweenPts( const CPortalData_Point *pPt1, const CPortalData_Point *pPt2 );
};

class CPortalData_Edge
{
public:
	// data:
	CPortalData_Point *m_apPoints[2];// CPortalData_Point * contained in the cell

	u32 m_nNumFaces;
	CPortalData_Face *m_apFaces[2];// CPortalData_Face * contained in the cell

	PortalData_Loc_e m_Loc;// what side of a cutting plane is this on

	void RemoveFace( CPortalData_Face *pFace );
	void Create( CPortalData_Point &rP1, CPortalData_Point &rP2 );
};

class CPortalData_Cell
{
public:
	// data:
	cchar *m_pszName;
	BOOL m_bDeleteNameString;

	u32 m_nNumFaces;
	CPtrArray m_apFaces;// CPortalData_Face *

	u32 m_nNumEdges;
	CPtrArray m_apEdges;// CPortalData_Edge *

	u32 m_nNumPts;
	CPtrArray m_apPts;// CPortalData_Point *

	CFSphere m_BoundingSphere;

	// interface:
	CPortalData_Cell();
	~CPortalData_Cell();

	// ALTERS DATA CONTAINED IN THE CLASS
	BOOL Create( const ApeVisCell_t &rCell );
	BOOL Create( const CPortalData_Cell &rCell );
	BOOL Create( const CPortalData_Cell &rCell, const CPortalData_Plane &rPlane, PortalData_Loc_e nLoc, BOOL bCellAlreadyClipped=FALSE );
	void Empty();	

	PortalData_Loc_e CellToPlane( const CPortalData_Plane &rPlane, BOOL bReCalcLocData=TRUE );
	BOOL DoesPlaneExistBetween( CPortalData_Cell &rCell, CPortalData_Plane &rPlane, BOOL &rbIsPlaneACellFace );	
	BOOL ClipAlongPlane( const CPortalData_Plane &rPlane );
	BOOL SliceAlongPlane( const CPortalData_Plane &rPlane, PortalData_Loc_e nLoc, BOOL bCellAlreadyClipped=FALSE );
	void RecomputeBoundingSphere();
	
	// DOES NOT ALTER THE DATA CONTAINED IN THE CLASS
	PortalData_Loc_e PointToCell( const CFVec3 &rPt, f32 &rfDistToClosestPlane ) const;
	PortalData_Loc_e EdgeToCell( const CPortalData_Edge &rEdge ) const;
	PortalData_Loc_e EdgeToCell( const CPortalData_Edge &rEdge, PortalData_Loc_e &rnLoc1, PortalData_Loc_e &rnLoc2 ) const;
	BOOL DoesSegmentIntersectCell( const CFVec3 &rStart, const CFVec3 &rEnd ) const;
	BOOL ClipEdgeToCell( const CPortalData_Edge &rEdge, CFVec3 &rImpactPt, s32 &rnFaceNum ) const;
	BOOL DoesAnyEdgeIntersectCell( const CPortalData_Cell &rCell ) const;
	BOOL FindInOutAndCrossingEdge( const CPortalData_Cell &rCell, 
								   s32 &rnInEdgeIndex, s32 &rnOutEdgeIndex, CPortalData_Face **ppFace ) const;
	BOOL IsAnyPtInsideCell( const CPortalData_Cell &rCell, BOOL bTreatOnAsIn=FALSE ) const;
	u32 GetListOfEdgesOnThePlane( CPtrArray &rEdges ) const;
	void CalculateBoundingBox( CFVec3 &rMin, CFVec3 &rMax ) const;
	void FindDeepestPtsFromPlane( const CPortalData_Plane &rPlane, f32 &rfFrontDepth2, f32 &rfBackDepth2 ) const;
	BOOL AutoPortalFromFaceOnPlane( CPtrArray &rapPortals, u32 &rnNumPortals, cchar *pszName=NULL ) const;
	s32 FindCoPlanarFace( const CPortalData_Plane &rPlane ) const;
	void CalculateCentroid( CFVec3 &rCenter ) const;
	f32 CalculateFurthestDistFromPt( const CFVec3 &rPt ) const;

	// STATIC FUNCTIONS
	static BOOL AutoPortalFromFace( const CPortalData_Face *pFace, CPtrArray &rapPortals, u32 &rnNumPortals, cchar *pszName=NULL );
	static s32 FindIndexFromPtr( const void *pPtr, u32 nNumElements, const CPtrArray &rArray );
	static s32 DoCellsIntersect( CPortalData_Cell &rCellA, CPortalData_Cell &rCellB, 
								 s32 &rnInEdgeIndex, s32 &rnOutEdgeIndex, CPortalData_Face **ppFace,
								 BOOL bTestForPlanesBetweenCells=TRUE );
	
private:
	BOOL AllocateMemory( u32 nNumFaces, u32 nNumEdges, u32 nNumPts );	
	void CalculateLocDataFromPlane( const CPortalData_Plane &rPlane );	
};

class CPortalData_Vol 
{
public:
	// data:
	u32 m_nNumCells;
	CPtrArray m_apCells;// CPortalData_Cell *

	CFSphere m_BoundingSphere;

	// interface:
	CPortalData_Vol();
	~CPortalData_Vol();
	BOOL Create( const ApeVisVolume_t &rVol );
	PortalData_Loc_e IsPtInsideVolume( const CFVec3 &rPt ) const;
	u32 GetNumOfCellFaces() const;
	BOOL ClipOverlappingCells( CString &rsErrorLine );
	void Empty();
	void CalculateBoundingBox( CFVec3 &rMin, CFVec3 &rMax ) const;
	void RecomputeBoundingSphere();
	static BOOL ClipOverlappingVols( CPortalData_Vol &rVol1, CPortalData_Vol &rVol2, 
									 CString &rsErrorLine, CPtrArray &rapPortals, u32 &rnNumPortals );
	BOOL IsCellFullyContainedInACellOfVol( const CPortalData_Cell &rCell ) const;
	BOOL IsCellFullyOutsideAllCellsOfVol( const CPortalData_Cell &rCell ) const;

private:
	
};

class CPortalData_Portal
{
public:
	CPortalData_Portal();
	~CPortalData_Portal();

	// data:
	cchar *m_pszName;
	BOOL m_bDeleteNameString;
	CFVec3 m_aCorners[4];
	CFVec3 m_Normal;
	CFVec3 m_Centroid;
	CFVec3 m_Min;
	CFVec3 m_Max;

	u32 m_nFlags;// see FVIS_PORTAL_FLAG_* in fvis.h

	// interface:
	void Create( const CPortalData_Face &rFace );
	void Create( const ApeVisPortal_t &rPortal );
	BOOL DoesPortalIntersect( const CPortalData_Portal &rPortal ) const;
	void ComputeBoundingSphere( CFSphere &rSphere ) const;
};

#endif
