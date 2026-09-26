//////////////////////////////////////////////////////////////////////////////////////
// KongTriClipper.cpp - Classes used to clip Kong Tris to FVis data
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

#include "stdafx.h"
#include "Fang.h"
#include "Pasm.h"
#include "KongTriClipper.h"
#include "fvis.h"
#include "utils.h"
#include "fworld_coll.h"
#include "CompileDlg.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local defines
//////////////////////////////////////////////////////////////////////////////////////

#define	_MAX_CLIPPED_KONG_TRIS		32768
#define	_MAX_CLIPPED_KONG_VERTS		102400

#define _SHOW_DETAILED_RESULTS		FALSE


//////////////////////////////////////////////////////////////////////////////////////
// Local classes and structures
//////////////////////////////////////////////////////////////////////////////////////

//
//	Note that the clipper poly can have more than three verts.  However, 
//	this implementation will always ensure that it is possible to fan the poly.
//
struct _ClipperPoly
{
	u16 *pVertIndices;
	u16 nVertCount;
	u16 nFanAnchorVert;
};


enum
{
	_VERT_FLAG_NONE					= 0x00000000,
	_VERT_FLAG_ORIG_VERT_0			= 0x00000001,
	_VERT_FLAG_ORIG_VERT_1			= 0x00000002,
	_VERT_FLAG_ORIG_VERT_2			= 0x00000004,
};

//
//
//
struct _ClipperVert
{
	u16		nEdgeFlags;
	u16		nInitialVolumeID;	// The volume that initially created this clipped vert
	u16		nAdjacentVolumeID;	// The volume that shares this vert
	BOOL8	bPaired;			// This flag is set when the vert is paired up with a vert from the same voluemclipped on the other side of the plane
	BOOL8	bUnPaired;			// This flag is set when the vert is shared by two or more volumes
	u8		nUseCount;
	u8		nPad[3];

	CFVec3A	vPos;
	CFVec3	vNorm;
	CFVec2	aUV[KONG_MAX_UVS_PER_MAT];
	CFColorRGBA Color;
	KongVert_t *pKongPoolVert;

	void Set( _ClipperVert *pVert )
	{
		nEdgeFlags = pVert->nEdgeFlags;
		vPos = pVert->vPos;
		vNorm = pVert->vNorm;
		Color = pVert->Color;
		u32 i;
		for ( i = 0; i < KONG_MAX_UVS_PER_MAT; i++ )
		{
			aUV[i] = pVert->aUV[i];
		}
	}

	void AssignToKongVert( KongVert_t *pKongVert )
	{
		pKongVert->Pos   = vPos.v3;
		pKongVert->Norm  = vNorm;
		pKongVert->Color = Color;
		u32 i;
		for ( i = 0; i < KONG_MAX_UVS_PER_MAT; i++ )
		{
			pKongVert->aUV[i] = aUV[i];
		}
	}

	void Set( KongVert_t *pKongVert, u16 nEdgeFlagsSetting )
	{
		nEdgeFlags = nEdgeFlagsSetting;
		vPos.v3 = pKongVert->Pos;
		vNorm = pKongVert->Norm;
		Color = pKongVert->Color;
		u32 i;
		for ( i = 0; i < KONG_MAX_UVS_PER_MAT; i++ )
		{
			aUV[i] = pKongVert->aUV[i];
		}
	}

	void ReceivePositionLerp( f32 fLerp, _ClipperVert *pVert1, _ClipperVert *pVert2 )
	{
		// Linearly interpolate the vertex position
		vPos.v3  = (pVert1->vPos.v3 * (1.f - fLerp)) + (pVert2->vPos.v3 * fLerp);
	}
	void ReceiveAttributeLerp( f32 fLerp, _ClipperVert *pVert1, _ClipperVert *pVert2 )
	{
		// Linearly interpolate the vertex attributes
//		vNorm = pVert1->vNorm;  // Since we're clipping a poly, the normal is the same
		vNorm = (pVert1->vNorm * (1.f - fLerp)) + (pVert2->vNorm * fLerp);
		f32 fMagSq = vNorm.Mag2();
		if ( fMagSq == 0.f )
		{
			FASSERT_NOW;
			vNorm.Set( 0.f, 1.f, 0.f );
		}
		else
		{
			vNorm.Mul( fmath_AcuInvSqrt( fMagSq ) );
		}
		Color = (pVert1->Color * (1.f - fLerp)) + (pVert2->Color * fLerp);
		u32 i;
		for ( i = 0; i < KONG_MAX_UVS_PER_MAT; i++ )
		{
			aUV[i] = (pVert1->aUV[i] * (1.f - fLerp)) + (pVert2->aUV[i] * fLerp);
		}
		// Set the edge flags
		nEdgeFlags = pVert1->nEdgeFlags | pVert2->nEdgeFlags;
	}
};


//////////////////////////////////////////////////////////////////////////////////////
// Local global variables
//////////////////////////////////////////////////////////////////////////////////////

// Buffer for holding the resulting clipped Kong tris
static KongTri_t	_ClippedKongTris[_MAX_CLIPPED_KONG_TRIS];
static KongVert_t	_ClippedKongVerts[_MAX_CLIPPED_KONG_VERTS];
static u32			_nKongTrisUsed;
static u32			_nKongVertsUsed;

// Vis data for tracking clipping performed on the tris
static CKTC_ClipPlane	aClipPlanes[KTC_MAX_CLIP_PLANES];
static CKTC_ClipCell	aClipCells[KTC_MAX_CLIP_CELLS];
static CKTC_ClipVolume	aClipVolumes[KTC_MAX_CLIP_VOLUMES];

// Vertex clipping buffers
static _ClipperVert _avClipBuffers[2][KTC_MAX_CLIP_BUFFER_POINTS];
static _ClipperVert _avClippedVertPool[KTC_MAX_CLIPPED_VERTS];
static u16 _ClippedVertIndices[KTC_MAX_CLIPPED_VERT_INDICES];
static u16 _nClippedVertCount;
static u32 _nClippedVertIndexCount;
static u32 _nNoGoodTriangulationPointCount;

// Static clipper vars
CKongClipper	*CKongClipper::m_pThis;
CKTC_ClipPlane	*CKongClipper::m_pllClipPlanePool = NULL;
CKTC_ClipCell	*CKongClipper::m_pllClipCellPool = NULL;
CKTC_ClipVolume	*CKongClipper::m_pllClipVolumePool = NULL;
u32				CKongClipper::m_nClipVolumePoolCount = 0;
u32				CKongClipper::m_nClipCellPoolCount = 0;
u32				CKongClipper::m_nClipPlanePoolCount = 0;


//////////////////////////////////////////////////////////////////////////////////////
// Local function prototypes
//////////////////////////////////////////////////////////////////////////////////////
static u16* _GetBestTriangulation( u16 *aVertIndices, u16 nVertCount, CFVec3 *pTriNormal );
static f32 _Triangulate( u16 *aVertIndices, u16 nVertCount, u16 nStartVert, CFVec3 *pTriNormal, u16 **anSolution );
BOOL _SphereHitsTri( CFSphere *pSphere, CFVec3A &Vert0, CFVec3A &Vert1, CFVec3A &Vert2, CFVec3 &vTriNormal );

//////////////////////////////////////////////////////////////////////////////////////
// Code Implementation
//////////////////////////////////////////////////////////////////////////////////////


//
//	This function adds new verts to the clipped vert pool.  However, it first
//	checks to see if the same vert is in the pool.
//
u16 AddClippedVert( _ClipperVert *pVert )
{
	u16 i;
	f32 fTolerance = 0.000001f;
	BOOL bClippedVert = TRUE;
	if (   pVert->nEdgeFlags == _VERT_FLAG_ORIG_VERT_0
		|| pVert->nEdgeFlags == _VERT_FLAG_ORIG_VERT_1
		|| pVert->nEdgeFlags == _VERT_FLAG_ORIG_VERT_2 )
	{
		// This is one of the original verts, so we will not have any tolerance.
		bClippedVert = FALSE;
		fTolerance = 0.f;
	}

	for ( i = 0; i < _nClippedVertCount; i++ )
	{
		// Test the position tolerance
		CFVec3 vDiff;
		vDiff = _avClippedVertPool[i].vPos.v3 - pVert->vPos.v3;

		if ( vDiff.Mag2() <= fTolerance )
		{
			if ( pVert->nInitialVolumeID == _avClippedVertPool[i].nInitialVolumeID || !bClippedVert )
			{
				_avClippedVertPool[i].bPaired = TRUE;
			}
			else
			{
				_avClippedVertPool[i].bUnPaired = TRUE;
				_avClippedVertPool[i].nAdjacentVolumeID = pVert->nInitialVolumeID;
			}
			_avClippedVertPool[i].nUseCount++;
			return i;
		}
	}

	if ( _nClippedVertCount == KTC_MAX_CLIPPED_VERTS )
	{
		// No more room in the clip buffer, so we cannot add this
		return 0xffff;
	}

	// We didn't find the same vert already in the buffer so we need to add a new one:
	pVert->pKongPoolVert = NULL;
	fang_MemCopy( &_avClippedVertPool[_nClippedVertCount], pVert, sizeof( _ClipperVert ) );
	_avClippedVertPool[_nClippedVertCount].bUnPaired = FALSE;
	_avClippedVertPool[_nClippedVertCount].nUseCount = 1;
	_avClippedVertPool[_nClippedVertCount].nAdjacentVolumeID = 0xffff;
	if ( !bClippedVert )
	{
		// This is one of the original tri verts, not a clipped vert, so it
		// doesn't need to be paired up.
		_avClippedVertPool[_nClippedVertCount].bPaired = TRUE;
	}
	else
	{
		_avClippedVertPool[_nClippedVertCount].bPaired = FALSE;
	}

	_nClippedVertCount++;

	return _nClippedVertCount - 1;
}


//
//
//
CKongClipper::CKongClipper( void )
{
	u32 i;
	m_pThis = NULL;

	m_anVolTriCounter = NULL;
	m_pVolTriLinkListTail = NULL;
	m_pVolTriLinkListHead = NULL;

	if ( !m_pllClipPlanePool )
	{
		m_pllClipPlanePool = &aClipPlanes[0];
		for ( i = 0; i < KTC_MAX_CLIP_PLANES - 1; i++ )
		{
			aClipPlanes[i].m_pNext = &aClipPlanes[i+1];
		}
		aClipPlanes[i].m_pNext = NULL;
		m_nClipPlanePoolCount = KTC_MAX_CLIP_PLANES;
	}

	if ( !m_pllClipCellPool )
	{
		m_pllClipCellPool = &aClipCells[0];
		for ( i = 0; i < KTC_MAX_CLIP_CELLS - 1; i++ )
		{
			aClipCells[i].m_pNext = &aClipCells[i+1];
		}
		aClipCells[i].m_pNext = NULL;
		m_nClipCellPoolCount = KTC_MAX_CLIP_CELLS;
	}

	if ( !m_pllClipVolumePool )
	{
		m_pllClipVolumePool = &aClipVolumes[0];
		for ( i = 0; i < KTC_MAX_CLIP_VOLUMES - 1; i++ )
		{
			aClipVolumes[i].m_pNext = &aClipVolumes[i+1];
		}
		aClipVolumes[i].m_pNext = NULL;
		m_nClipVolumePoolCount = KTC_MAX_CLIP_VOLUMES;
	}

	// Clear the results
	fang_MemZero( &m_Results, sizeof( ClipperResults ) );
}


//
//
//
CKongClipper::~CKongClipper( void )
{
#if _SHOW_DETAILED_RESULTS
	if ( m_Results.m_nTrisSubmitted )
	{
		DEVPRINTF( "\nCKongClipper::\n" );
		DEVPRINTF( "	--------------\n" );
		DEVPRINTF( "	Tris Submitted		    : %d\n", m_Results.m_nTrisSubmitted );
		DEVPRINTF( "	--------------\n" );
		DEVPRINTF( "	Tris In No Cells	    : %d\n", m_Results.m_nTrisInNoCells );
		DEVPRINTF( "  + Intersecting 1 Cell	    : %d\n", m_Results.m_nTrisInOneCell );
		DEVPRINTF( "  + Intersecting 1 Volume   : %d\n", m_Results.m_nTrisInOneVolume );
		DEVPRINTF( "  + Postclip Intersect 1 Vol: %d\n", m_Results.m_nPostclippedTrisInOneVolume );
		DEVPRINTF( "  + Postclip Edges in 1 Vol : %d\n", m_Results.m_nAllEdgesClippedToOneVolume );
		DEVPRINTF( "  + Tris from clipping	    : %d\n", _nKongTrisUsed );
		DEVPRINTF( "  = Total triangles		    : %d\n", m_Results.m_nTotalWorldTris );
		DEVPRINTF( "	--------------\n" );
		f32 fPercent = 0.f;
		if ( m_Results.m_nTrisClipped )
		{
			fPercent = 100.f * ((f32)m_Results.m_nTrisClipped / (f32)m_Results.m_nTrisSubmitted);
		}
		DEVPRINTF( "    Tris Clipped			: %d (%3.1f%% of Submitted)\n", m_Results.m_nTrisClipped, fPercent );
		DEVPRINTF( "    Tris saved by merger	: %d\n", m_Results.m_nTrisSavedByMerger );
		DEVPRINTF( "    Resulting Zero Area		: %d\n", m_Results.m_nResultingZeroAreaTris );
		DEVPRINTF( "	Resulting Clip Verts	: %d\n", _nKongVertsUsed );
		DEVPRINTF( "	Volumes Considered for Clipping	: %d\n", m_Results.m_nTotalVolumesConsidered );
		DEVPRINTF( "	Volumes Trivially Discarded		: %d\n", m_Results.m_nTotalVolumesTriviallyDiscarded );
		if ( m_Results.m_nTrisClipped )
		{
			DEVPRINTF( "	Average Volumes per Tri Clipped	: %3.2f\n\n", (f32)m_Results.m_nTotalVolumesOfClippedTris / (f32)m_Results.m_nTrisClipped );
		}
		DEVPRINTF( "	Seconds spent in clipper		: %6.3f\n\n", (f32)m_Results.m_nTicksClipping * 0.001f );
	}
#endif

	FreeMemoryAllocations();
}


//
//
//
BOOL CKongClipper::FreeMemoryAllocations( void )
{
	if ( m_anVolTriCounter )
	{
		fang_Free( m_anVolTriCounter );
		m_anVolTriCounter = NULL;
	}

	return TRUE;
}


//
//
//
BOOL CKongClipper::AddTriToVolume( KongTri_t *pTri, FVisVolume_t *pVolume, BOOL bClipped, u32 nMatID ) 
{
	pTri->nTriFlags |= KONGTRI_FLAG_TRI_ADDED_TO_VOLUME;
	pTri->nUser[0] = pVolume->nVolumeID;
	pTri->nUser[1] = (u16)nMatID;
	m_anVolTriCounter[pVolume->nVolumeID]++;

	// Link the tri to the tri list
	if ( !m_pVolTriLinkListHead )
	{
		m_pVolTriLinkListHead = pTri;
		m_pVolTriLinkListTail = pTri;
	}
	else
	{
		m_pVolTriLinkListTail->pUser = pTri;
		m_pVolTriLinkListTail = pTri;
	}

	pTri->pUser = NULL;

	m_Results.m_nTotalWorldTris++;

	return TRUE;
}


//
//
//
u32 CKongClipper::ClipAndSeparateKong( CApeToKongFormat *pApeToKong, KongMesh_t *pKongMesh, CKongToChimp *paChimps, u32 nVolumeCount, CCompileDlg *pDlg/*=NULL*/ ) 
{
	u32 i;
	u32 nErrorCode;
	KongSeg_t *pSeg;
	KongMat_t *pMat;
	KongTri_t *pTri;

	_nKongVertsUsed = 0;
	_nKongTrisUsed = 0;

	_nNoGoodTriangulationPointCount = 0;

	// Clear the results
	fang_MemZero( &m_Results, sizeof( ClipperResults ) );

	// Start the timer
	CTickTimer Timer( &m_Results.m_nTicksClipping );

	// Free any existing allocations
	FreeMemoryAllocations();

	fang_MemZero( _ClippedKongVerts, sizeof( KongVert_t ) * _MAX_CLIPPED_KONG_VERTS );

	// Allocate space for counting up and linking the new tris in the appropriate volume
	m_anVolTriCounter = (u16 *)fang_MallocAndZero( sizeof( u16 ) * nVolumeCount, 4 );
	m_pVolTriLinkListHead = NULL;
	m_pVolTriLinkListTail = NULL;
			
	// Get a pointer to the segment
	pSeg = &pKongMesh->paSegs[0];

	s32 nTotalTris = 0;
	if ( pDlg )
	{
		pMat = pKongMesh->GetFirstMaterial( pSeg );
		while ( pMat )
		{
			nTotalTris += pMat->nNumTris;
			pMat = pKongMesh->GetNextMaterial( pMat );
		}
		pDlg->ResetSubOp( "Clipping Triangles to Volume Boundaries...", nTotalTris / 1000 );
		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
		nTotalTris = 1000;
	}

	// Cycle through all of the materials processing the tris
	pMat = pKongMesh->GetFirstMaterial( pSeg );
	while ( pMat )
	{
		pTri = pKongMesh->GetFirstTri( pMat );
		while ( pTri )
		{
			if ( pDlg && --nTotalTris == 0 )
			{
				pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
				nTotalTris = 1000;

				if ( pDlg->m_bAbortCompile )
				{
					return KTC_GENERAL_ERROR;
				}
			}

			nErrorCode = ClipTri( pTri, pMat->nMatIdx );		
			if ( nErrorCode != KTC_NO_ERROR )
			{
				return nErrorCode;
			}
			pTri = pKongMesh->GetNextTri( pTri );
		}

		pMat = pKongMesh->GetNextMaterial( pMat );
	}

	// Allocate the Chimps
	for ( i = 0; i < nVolumeCount; i++ ) 
	{
		if ( m_anVolTriCounter[i] ) 
		{
			if ( !paChimps[i].CreateEmptyMeshFromTemplate( *pApeToKong, m_anVolTriCounter[i], TRUE ) ) 
			{
				return KTC_UNABLE_TO_CREATE_CHIMPS;
			}
		}
	}

	// Add the triangles to the chimps
	pTri = m_pVolTriLinkListHead;
	while ( pTri )
	{
		KongTri_t *pNext = (KongTri_t *)pTri->pUser;

		if ( !paChimps[ pTri->nUser[0] ].AddTriToMesh( *pTri, pTri->nUser[1] ) ) 
		{
			return KTC_ERROR_ADDING_CHECK_TO_VOLUMES;
		}

		pTri = pNext;
	}

	// Close up the volumes (Chimps)
	for ( i = 0; i < nVolumeCount; i++ ) 
	{
		if ( m_anVolTriCounter[i] ) 
		{
			if ( !paChimps[i].CloseMesh( TRUE ) ) 
			{
				return KTC_ERROR_CLOSING_CHIMPS;
			}
			if ( paChimps[i].m_pKongMesh->nTriangleCount > m_Results.m_nMostTrisPerVolume ) 
			{
				m_Results.m_nMostTrisPerVolume = paChimps[i].m_pKongMesh->nTriangleCount;
				m_Results.m_nVolIdxWithMostTris = i;
			}
		}
	}

	if ( _nNoGoodTriangulationPointCount )
	{
		DEVPRINTF( "CKongClipper::ClipAndSeparateKong() - Could not find a good point to start triangulation for %d polys.\n", _nNoGoodTriangulationPointCount );
	}

	return KTC_NO_ERROR;
}


//
//
//
FINLINE u32 CKongClipper::ClipTri( KongTri_t *pTri, u32 nMatID )
{
#if FANG_USE_PORTAL
	FASSERT( pTri );

	m_pThis = this;

	m_Results.m_nTrisSubmitted++;

	// Generate a centroid
	
	//////////////////////////////////////////////////////////////////////////
	// FIRST IF ALL POINTS ARE IN THE SAME CELL THEN NO CLIPPING NEEDS
	// TO BE PERFORMED.
	//////////////////////////////////////////////////////////////////////////

	// Find out what cells the points are in
	FVisCell_t *apCell[3];
	CFVec3A vTemp;
	fworld_GetCellContainingPoint( vTemp.Set( pTri->apKongVerts[0]->Pos ), &apCell[0] );
	fworld_GetCellContainingPoint( vTemp.Set( pTri->apKongVerts[1]->Pos ), &apCell[1] );
	fworld_GetCellContainingPoint( vTemp.Set( pTri->apKongVerts[2]->Pos ), &apCell[2] );

#if 0
	// Debug code to track the treatment of a given point
	u32 iPoint, iCount = 0;
	CFVec3 vDiff, vTest1( -286.8f, -233.8f, -873.0f ), vTest2( -99.6f, 7.3f, -88.9f );
	for ( iPoint = 0; iPoint < 3; iPoint++ )
	{
		vDiff = vTest1 - pTri->apKongVerts[iPoint]->Pos;
		f32 fDist = vDiff.Mag2();
		if ( fDist < 9 )
		{
			iCount++;
		}
		vDiff = vTest2 - pTri->apKongVerts[iPoint]->Pos;
		fDist = vDiff.Mag2();
		if ( fDist < 15 )
		{
			iCount++;
		}
		if ( iCount == 2 )
		{
			iPoint = 3;
		}
	}
#endif

	// NULL the points if they are in the slop volume
	u32 i, ii, iii;
	for ( i = 0; i < 3; i++ )
	{
		if ( apCell[i] && apCell[i]->nParentVolumeIdx == 0 )
		{
			apCell[i] = NULL;
		}
	}

	// NOTES:
	// - If all of the points are in cells in the same volume, one of the tri edges could still
	//		cross across the volume boundary and intersect another non-volume cell
	// - If all of the points are in invalid cells, the tri could cross through a cell

	// Clear the clipping lists
	m_VolumeClipList.Clear( ReleaseClipVolume );

	// Find all of the volumes that the tri intersects using the bounding sphere
	fworld_GetVolumesIntersectingSphere( &pTri->BoundSphere, VolumeVsSphereCallback, NULL, -1, FVIS_PORTAL_FLAG_NONE );

	//////////////////////////////////////////////////////////////////////////
	// NEXT WE SEE IF A SPHERE THAT ENCOMPASSES THE TRIANGLE INTERSECTS MORE
	// THAN ONE VOLUME.  IF IT DOESN'T, THEN WE KNOW THAT THE TRIANGLE IS
	// INTERSECTING CELLS OF THE SAME VOLUME.
	//////////////////////////////////////////////////////////////////////////

	if( m_VolumeClipList.m_nCount == 0 )
	{
		// This triangle does not intersect any cell, so put it in the "Slop Volume"
		m_Results.m_nTrisInNoCells++;
		if ( !AddTriToVolume( pTri, FWorld_pWorld->paVolumes, FALSE, nMatID ) )
		{
			return KTC_CALLBACK_ERROR;
		}
		return KTC_NO_ERROR;
	}

	if( m_VolumeClipList.m_nCount == 1 && (apCell[0] || apCell[1] || apCell[2]) )
	{
		// This triangle is entirely contained within one volume, so call the callback and return
		// Note:  We do not use this quick out if the triangle's verts are not in a cell
		// because in that case the triangle might still not intersect the volume, so clip it.
		if ( apCell[0] == apCell[1] && apCell[0] == apCell[2] )
		{
			m_Results.m_nTrisInOneCell++;
		}
		else
		{
			m_Results.m_nTrisInOneVolume++;
		}
		if ( !AddTriToVolume( pTri, ((CKTC_ClipVolume *)m_VolumeClipList.m_pHead)->m_pVolume, FALSE, nMatID ) )
		{
			return KTC_CALLBACK_ERROR;
		}

		return KTC_NO_ERROR;
	}

	m_bContainment = FALSE;
	if( apCell[0] && apCell[0] == apCell[1] && apCell[0] == apCell[2] )
	{
		// This triangle's points are all in the same cell, so if this
		// cell doesn't contain any other cell, it must be the only cell
		// containing the triangle

		// Check this specific Cell to see if it contains any cells of the other volumes...
		FASSERT( apCell[0]->nParentVolumeIdx < FWorld_pWorld->nVolumeCount );

		CKTC_ClipVolume *pClipVol = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
		while ( pClipVol && !m_bContainment )
		{
			// Check all of the cells of the volume
			FVisCell_t *pCell = &FWorld_pWorld->paCells[pClipVol->m_pVolume->nCellFirstIdx];
			for ( ii = 0; ii < pClipVol->m_pVolume->nCellCount && !m_bContainment; ii++, pCell++ )
			{
				if ( pCell == apCell[0] )
				{
					continue;
				}

				for ( iii = 0; iii < pCell->nPlaneCount; iii++ )
				{
					if ( apCell[0]->IntersectPoint( &pCell->paBoundingPlanes[iii].vPoint ) > 0.01f )
					{
						// pClipVol contains a cell inside the tested cell, so stop here
						m_bContainment = TRUE;
						break;
					}
				}
			}

			pClipVol = pClipVol->Next();
		}

		if ( !m_bContainment )
		{
			// We didn't find that this cell contains any others, so it's the winner!
			m_Results.m_nTrisInOneCell++;

			if( !AddTriToVolume( pTri, &FWorld_pWorld->paVolumes[apCell[0]->nParentVolumeIdx], FALSE, nMatID ) )
			{
				return KTC_CALLBACK_ERROR;
			}

			return KTC_NO_ERROR;
		}
	}

	// Check for containment if we didn't already find it
	CKTC_ClipVolume *pClipVol = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
	while ( pClipVol && !m_bContainment )
	{
		CKTC_ClipVolume *pClipVol2 = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
		while ( pClipVol2 && !m_bContainment )
		{
			if ( pClipVol2 == pClipVol )
			{
				pClipVol2 = pClipVol2->Next();
				continue;
			}

			// Check all of the cells of the sub-volume
			FVisCell_t *pCell = &FWorld_pWorld->paCells[pClipVol2->m_pVolume->nCellFirstIdx];
			for ( ii = 0; ii < pClipVol2->m_pVolume->nCellCount && !m_bContainment; ii++, pCell++ )
			{
				for ( iii = 0; iii < pCell->nPlaneCount; iii++ )
				{
					if ( pClipVol->m_pVolume->IntersectPoint( &pCell->paBoundingPlanes[iii].vPoint ) > 0.01f )
					{
						// pClipVol contains a cell of pClipVol2, so stop here
						m_bContainment = TRUE;
						break;
					}
				}
			}

			pClipVol2 = pClipVol2->Next();
		}

		pClipVol = pClipVol->Next();
	}

#if FANG_DEBUG_BUILD
	// Check to make sure that all of the cells the points are in are reported as collisions
	for ( i = 0; i < 3; i++ )
	{
		if ( !apCell[i] )
		{
			continue;
		}

		CKTC_ClipVolume *pClipVol = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
		while ( pClipVol )
		{
			FVisVolume_t *pVol = pClipVol->m_pVolume;
			for ( ii = pVol->nCellFirstIdx; ii < (u16)(pVol->nCellFirstIdx + pVol->nCellCount); ii++ )
			{
				if ( &FWorld_pWorld->paCells[ii] == apCell[i] )
				{
					break;
				}
			}
			if ( ii != (u16)(pVol->nCellFirstIdx + pVol->nCellCount) )
			{
				break;
			}

			pClipVol = (CKTC_ClipVolume *)pClipVol->m_pNext;
		}
		FASSERT( pClipVol );
	}
#endif // FANG_DEBUG_BUILD

	//////////////////////////////////////////////////////////////////////////
	// WE WERE NOT ABLE TO TRIVIALLY RESOLVE THE LOCATION OF THE TRIANGLE SO
	// WE NEED TO SEND IT THROUGH THE CLIPPING CODE TO RESOLVE ITS LOCATION.
	//////////////////////////////////////////////////////////////////////////

	u32 nReturn = ClipAgainstClipVolumes( pTri, apCell, &pTri->BoundSphere, nMatID ); 

	return nReturn;
#endif
}


//
//
//
u32 CKongClipper::ClipAgainstClipVolumes( KongTri_t *pTri, FVisCell_t **apPointCells, CFSphere *pBoundingSphere, u32 nMatID )
{
	u32 i, ii, iii;

	u32 nClippingPlanesCount = 0;
	u32 nClippingCellCount = 0;

	_nClippedVertCount = 0;
	_nClippedVertIndexCount = 0;

	m_Results.m_nTotalVolumesConsidered += m_VolumeClipList.m_nCount;

	//////////////////////////////////////////////////////////////////////////
	// DO PRECISE CLIPPING OF THE TRIANGLE AGAINST ALL OF THE VOLUMES' CELL
	// BOUNDARIES TO DETERMINE IF THE TRIANGLE ACTUALLY INTERSECTS.
	//////////////////////////////////////////////////////////////////////////

	// Do all of the points of any volume lie on one side of the tri?
	CKTC_ClipVolume *pNext, *pClipVol = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
	while ( pClipVol )
	{
		pNext = pClipVol->Next();

		// Check all of the cells of the volume for tri containment
		FVisCell_t *pCell = &FWorld_pWorld->paCells[pClipVol->m_pVolume->nCellFirstIdx];
		for ( ii = 0; ii < pClipVol->m_pVolume->nCellCount; ii++, pCell++ )
		{
			// First check the bounding sphere of the cell against the tri sphere
			if ( !pBoundingSphere->IsIntersecting( pCell->spBoundingWS ) )
			{
				continue;
			}

			_avClipBuffers[0][0].Set( pTri->apKongVerts[0], _VERT_FLAG_ORIG_VERT_0 );
			_avClipBuffers[0][1].Set( pTri->apKongVerts[1], _VERT_FLAG_ORIG_VERT_1 );
			_avClipBuffers[0][2].Set( pTri->apKongVerts[2], _VERT_FLAG_ORIG_VERT_2 );

			u16 nSourceBuffer = 0;
			u16 nVertCount = 3;

			CKTC_ClipCell *pClipCell = GetClipCellNode();
			if ( !pClipCell )
			{
				DEVPRINTF( "CKongClipper - ERROR - Exceeded cell clip buffer.\n" );
				return KTC_EXCEEDED_CELL_CLIP_BUFFER;
			}
			pClipCell->m_pCell = pCell;
			pClipCell->m_ClipPlaneList.Clear( ReleaseClipPlane );

			// Cycle through all of the cell's planes looking for clipping situations
			for ( iii = 0; iii < pCell->nPlaneCount; iii++ )
			{
				u32 nStartingBuffer = nSourceBuffer;
				if ( !ClipToPlane( &nSourceBuffer, &nVertCount, &pCell->paBoundingPlanes[iii].vNormal, &pCell->paBoundingPlanes[iii].vPoint ) )
				{
					// The tri is entirely outside the cell
					nVertCount = 0;
					break;
				}
				else if ( nStartingBuffer != nSourceBuffer )
				{
					// The clipping code flipped buffers which means that it clipped some
					// points against this plane so we need to add it to the clip list

					// Setup the clipping FOR
					CKTC_ClipPlane *pClipPlane = (CKTC_ClipPlane *)pClipCell->m_ClipPlaneList.Add( GetClipPlaneNode() );
					if ( !pClipPlane )
					{
						DEVPRINTF( "CKongClipper - ERROR - Exceeded plane clip buffer.\n" );
						return KTC_EXCEEDED_PLANE_CLIP_BUFFER;
					}

					pClipPlane->nPlaneIdx = iii;
				}
			}

			if ( nVertCount != 0 )
			{
				// Add the vertices to the clipped buffer and set the cell info
				if ( _nClippedVertIndexCount + nVertCount > KTC_MAX_CLIPPED_VERT_INDICES )
				{
					DEVPRINTF( "CKongClipper - ERROR - Exceeded clipped vertex buffer.\n" );
					return KTC_EXCEEDED_VERTEX_CLIP_BUFFER;
				}

				// Calculate the indices for these verts
				pClipCell->m_nFirstClippedVertIdx = _nClippedVertIndexCount;
				u32 iPriorVert = nVertCount - 1;
				for ( iii = 0; iii < nVertCount; iii++ )
				{
					// Make sure the prior vert is not a duplicate
					if ( _avClipBuffers[nSourceBuffer][iii].vPos == _avClipBuffers[nSourceBuffer][iPriorVert].vPos )
					{
						iPriorVert = iii;
						continue;
					}

					_avClipBuffers[nSourceBuffer][iii].nInitialVolumeID = pClipVol->m_pVolume->nVolumeID;

					_ClippedVertIndices[_nClippedVertIndexCount] = AddClippedVert( &_avClipBuffers[nSourceBuffer][iii] );

					if ( _ClippedVertIndices[_nClippedVertIndexCount] == 0xffff )
					{
						DEVPRINTF( "CKongClipper - ERROR - Exceeded clipped vertex index buffer.\n" );
						return KTC_EXCEEDED_VERTEX_CLIP_BUFFER;
					}
					_nClippedVertIndexCount++;

					iPriorVert = iii;
				}

				// Record the number of valid indices we created:
				pClipCell->m_nClippedVertIdxCount = _nClippedVertIndexCount - pClipCell->m_nFirstClippedVertIdx;

				if ( pClipCell->m_nClippedVertIdxCount > 2 )
				{
					// We ended up with a polygon in the cell, so the tri is in the cell
					pClipVol->m_ClipCellList.Add( pClipCell );
					nClippingPlanesCount += pClipCell->m_ClipPlaneList.m_nCount;
					nClippingCellCount++;
				}
				else
				{
					// This cell ended up not containing any of the triangle so don't use it

					m_Results.m_nResultingZeroAreaTris++;

					// If one of the points was in this cell, then we clear the pointer because
					// it clipped into a zero area triangle (the point was on the border or
					// was part of a thin triangle whose tip was clipped).
					if ( apPointCells[0] == pClipCell->m_pCell )
					{
						apPointCells[0] = NULL;
					}
					if ( apPointCells[1] == pClipCell->m_pCell )
					{
						apPointCells[1] = NULL;
					}
					if ( apPointCells[2] == pClipCell->m_pCell )
					{
						apPointCells[2] = NULL;
					}
					ReleaseClipCell( pClipCell );
				}
			}
			else
			{
				// This cell ended up not containing any of the triangle so don't use it
				ReleaseClipCell( pClipCell );
			}
		}

		// If we didn't get intersection of the tri with any of the volumes cells, then bail
		if ( pClipVol->m_ClipCellList.m_nCount == 0 )
		{
			m_VolumeClipList.Remove( pClipVol, ReleaseClipVolume );
			m_Results.m_nTotalVolumesTriviallyDiscarded++;
		}

		// On to the next...
		pClipVol = pNext;
	}

	//////////////////////////////////////////////////////////////////////////
	// IF WE EXCLUDED ALL VOLUMES IN THE VOLUME LIST, THEN THIS TRIANGLE
	// DOES NOT RESIDE IN ANY CELL AND SHOULD BE PLACED IN THE "SLOP BUCKET"
	//////////////////////////////////////////////////////////////////////////

	if ( m_VolumeClipList.m_nCount == 0 )
	{
//		if ( apPointCells[0] || apPointCells[1] || apPointCells[2] )
//		{
//			FASSERT_NOW;
//			return KTC_GENERAL_ERROR;
//		}

		// This triangle is not contained within any cell, so put it in the "Slop Volume"
		m_Results.m_nTrisInNoCells++;
		if ( !AddTriToVolume( pTri, FWorld_pWorld->paVolumes, FALSE, nMatID ) )
		{
			return KTC_CALLBACK_ERROR;
		}
		return KTC_NO_ERROR;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// IF WE EXCLUDED ALL BUT ONE VOLUME, THEN WE DO NOT NEED TO CLIP THIS
	// TRIANGLE SINCE IT ACTUALLY ONLY INTERSECTS ONE VOLUME.
	//////////////////////////////////////////////////////////////////////////

	if ( m_VolumeClipList.m_nCount == 1 )
	{
		// Thank the Lord.  This poly is only in one volume so no complex clipping required.
		if ( !m_VolumeClipList.m_pHead )
		{
			FASSERT_NOW;
			return KTC_GENERAL_ERROR;
		}

		m_Results.m_nPostclippedTrisInOneVolume++;
		if ( !AddTriToVolume( pTri, ((CKTC_ClipVolume *)m_VolumeClipList.m_pHead)->m_pVolume, FALSE, nMatID ) )
		{
			DEVPRINTF( "CKongClipper - ERROR - Unable to add tri to volume.\n" );
			return KTC_CALLBACK_ERROR;
		}

		return KTC_NO_ERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	// EVEN IF WE DETERMINED THAT THE TRIANGLE WAS IN MORE THAN ONE VOLUME, WE
	// MIGHT HAVE A CONTAINMENT SITUATION WHERE THE TRIANGLE IS ACTUALLY FULLY
	// IN THE CONTAINED VOLUME.  IN THIS CASE, WE KNOW THAT THE TRIANGLE IS IN
	// ONE VOLUME IF ALL OF THE CLIPPED POINTS WERE CLIPPED AGAINST PLANES THAT
	// ARE SHARED BETWEEN CELLS IN THE SAME VOLUME.  WE IDENTIFY THIS BY
	// DETECTING THAT ALL OF THE CLIPPED POINTS APPEARED IN TWO CELLS, BOTH OF
	// WHICH BELONG TO THE SAME VOLUME.
	//////////////////////////////////////////////////////////////////////////

	// See if all of the verts were clipped to planes shared between cells of the same volume
	BOOL bClippedToOneVolume = TRUE;
	u16 nCommonVolumeID = 0xffff;
	for ( i = 0; i < _nClippedVertCount; i++ )
	{
		if ( _avClippedVertPool[i].bPaired == FALSE || _avClippedVertPool[i].bUnPaired == TRUE )
		{
			// This vert exists in more than one volume, so we need to clip... bummer
			bClippedToOneVolume = FALSE;
			break;
		}
		if ( nCommonVolumeID == 0xffff )
		{
			nCommonVolumeID = _avClippedVertPool[i].nInitialVolumeID;
		}
		else if ( nCommonVolumeID != _avClippedVertPool[i].nInitialVolumeID )
		{
			// This tri contains verts in more than one volume, so we need to clip... bummer
			bClippedToOneVolume = FALSE;
			break;
		}
	}

	if ( bClippedToOneVolume )
	{
		// All of the clipped verts are clipped to planes that are between cells of the
		// same volume, so we don't need to clip this poly
		if ( !m_VolumeClipList.m_pHead )
		{
			FASSERT_NOW;
			return KTC_GENERAL_ERROR;
		}

		m_Results.m_nAllEdgesClippedToOneVolume++;
		if ( !AddTriToVolume( pTri, &FWorld_pWorld->paVolumes[nCommonVolumeID], FALSE, nMatID ) )
		{
			DEVPRINTF( "CKongClipper - ERROR - Unable to add tri to volume.\n" );
			return KTC_CALLBACK_ERROR;
		}

		return KTC_NO_ERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	// THIS TRIANGLE RESIDES IN MORE THAN ONE VOLUME, SO WE HAVE TO PERFORM
	// CLIPPING.  WE COULD HAVE THE FOLLOWING SCENARIOS:
	//		1 - THE TRIANGLE INTERSECTS TWO OR MORE VOLUMES THAT DO NOT
	//			CONTAIN ONE ANOTHER.
	//		2 - THE TRIANGLE IS CONTAINED IN TWO OR MORE VOLUMES AND ONE OR
	//			MORE VOLUMES CONTAIN ONE OR MORE OF THE OTHERS.
	//		3 - BOTH OF THE ABOVE.
	// IN ANY OF THE SCENARIOS, WE COULD STILL HAVE CLIPPING INFORMATION THAT
	// WAS GENERATED ABOVE THAT WE WISH TO DISCARD.  FOR EXAMPLE, IF ONE OF
	// THE VOLUMES CONTAINS MANY CELLS, WE ARE ONLY INTERESTED IN CLIPPING THE
	// VOLUME AGAINST THE CELL BOUNDARIES THAT BORDER OTHER VOLUMES.  OF COURSE
	// WE STILL HAVE TO BE SURE TO DIVIDE ANY CLIPPED POLYS INTO TRIANGLES.
	//////////////////////////////////////////////////////////////////////////

	m_Results.m_nTrisClipped++;
	m_Results.m_nTotalVolumesOfClippedTris += m_VolumeClipList.m_nCount;

	// TEMPORARY - Since we clipped this tri, remove the connectivity info
	// Eventually we need to fixup connectivity and resolve t-verts
	for ( i = 0; i < pTri->nNumEdgeTris; i++ )
	{
		if ( pTri->paEdgeTris[i] )
		{
			FASSERT( pTri->paEdgeTris[i]->nNumEdgeTris > 0 );
			if ( pTri->paEdgeTris[i]->paEdgeTris[0] == pTri )
			{
				pTri->paEdgeTris[i]->paEdgeTris[0] = pTri->paEdgeTris[i]->paEdgeTris[1];
				pTri->paEdgeTris[i]->paEdgeTris[1] = pTri->paEdgeTris[i]->paEdgeTris[2];
				pTri->paEdgeTris[i]->paEdgeTris[2] = NULL;
			}
			else if ( pTri->paEdgeTris[i]->paEdgeTris[1] == pTri )
			{
				pTri->paEdgeTris[i]->paEdgeTris[1] = pTri->paEdgeTris[i]->paEdgeTris[2];
				pTri->paEdgeTris[i]->paEdgeTris[2] = NULL;
			}
			else if ( pTri->paEdgeTris[i]->paEdgeTris[2] == pTri )
			{
				pTri->paEdgeTris[i]->paEdgeTris[2] = NULL;
			}
			else
			{
				FASSERT_NOW;
			}

			pTri->paEdgeTris[i]->nNumEdgeTris--;
			pTri->paEdgeTris[i] = NULL;
		}
	}
	pTri->nNumEdgeTris = 0;

	// Check containment situation
	if ( m_bContainment )
	{
		pClipVol = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
		while ( pClipVol )
		{
			CKTC_ClipVolume *pClipVol2 = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
			while ( pClipVol2 )
			{
				if ( pClipVol2 == pClipVol )
				{
					pClipVol2 = pClipVol2->Next();
					continue;
				}

				// Check all of the cells of the volume for new tris
				CKTC_ClipCell *pClipCell = (CKTC_ClipCell *)pClipVol2->m_ClipCellList.m_pHead;
				while ( pClipCell )
				{
					for ( ii = 0; ii < pClipCell->m_pCell->nPlaneCount; ii++ )
					{
						if ( pClipVol->m_pVolume->IntersectPoint( &pClipCell->m_pCell->paBoundingPlanes[ii].vPoint ) > 0.01f )
						{
							// pClipVol contains a cell of pClipVol2, so stop here
							break;
						}
					}

					if ( ii != pClipCell->m_pCell->nPlaneCount )
					{
						// We found a contained plane, so stop
						break;
					}

					pClipCell = pClipCell->Next();
				}

				if ( pClipCell )
				{
					// We found a contained cell, so stop
					break;
				}

				pClipVol2 = pClipVol2->Next();
			}

			pNext = pClipVol->Next();

			if ( pClipVol2 )
			{
				// pClipVol contains pClipVol2.  For now we will remove pClipVol
				// from the list of volumes to receive polys.  Eventually, we
				// will want to give pClipVol the remainder after clipping the
				// tri to pClipVol2
#if 1
				if ( !AddTriToVolume( pTri, pClipVol->m_pVolume, FALSE, nMatID ) )
				{
					DEVPRINTF( "CKongClipper - ERROR - Unable to add tri to volume.\n" );
					return KTC_CALLBACK_ERROR;
				}
#endif
				m_VolumeClipList.Remove( pClipVol, ReleaseClipVolume );
			}

			pClipVol = pNext;
		}
	}

	pClipVol = (CKTC_ClipVolume *)m_VolumeClipList.m_pHead;
	while ( pClipVol )
	{
		// Add all cells that 
		u16 nClippedPoly[128];
		u16 nClippedPolyVtxCount = 0;

		// First get a cell to start building a poly
		CKTC_ClipCell *pClipCell = (CKTC_ClipCell *)pClipVol->m_ClipCellList.m_pHead;
		while( pClipCell )
		{
			if ( pClipCell->m_nClippedVertIdxCount )
			{
				for ( i = 0; i < pClipCell->m_nClippedVertIdxCount; i++ )
				{
					nClippedPoly[nClippedPolyVtxCount++] = _ClippedVertIndices[(pClipCell->m_nFirstClippedVertIdx + i)];
				}
				break;
			}
			pClipCell = pClipCell->Next();
		}

		if ( nClippedPolyVtxCount == 0 )
		{
			// We have no more tris to add so go to the next volume
			pClipVol = pClipVol->Next();
			continue;
		}
/*
		// See if all of the verts were clipped to planes shared between cells of the same volume
		nCommonVolumeID = 0xffff;
		BOOL bAlreadyAdded = FALSE;
		for ( i = 0; i < nClippedPolyVtxCount; i++ )
		{
			if ( nCommonVolumeID == 0xffff )
			{
				nCommonVolumeID = _avClippedVertPool[i].nInitialVolumeID;
			}

			if (   _avClippedVertPool[nClippedPoly[i]].bPaired == FALSE 
				|| _avClippedVertPool[nClippedPoly[i]].bUnPaired == TRUE 
				|| _avClippedVertPool[i].nInitialVolumeID != nCommonVolumeID )
			{
				// This vert exists in more than one volume, so we should not merge this cell
//				AddPoly( nClippedPoly, nClippedPolyVtxCount, pTri, nMatID, pClipVol );
				pClipCell->m_nClippedVertIdxCount = 0;
				bAlreadyAdded = TRUE;
				break;
			}
		}

		if ( bAlreadyAdded == TRUE )
		{
			continue;
		}
*/
		// All of the verts clip to planes shared among cells of the same volume so
		// see if we can merge this poly with any adjacent cells
		u16 nMergedPoly[128];
		u16 nMergedPolyVtxCount = 0;

		CKTC_ClipCell *pAddCell = (CKTC_ClipCell *)pClipVol->m_ClipCellList.m_pHead;
		BOOL bMergedCells = FALSE;
		while( pAddCell )
		{
			if ( pAddCell != pClipCell && pAddCell->m_nClippedVertIdxCount )
			{
				// We've found a different cell with some indices.  Attempt a merge
				for ( i = 0; i < nClippedPolyVtxCount; i++ )
				{
					if ( bMergedCells )
					{
						nMergedPoly[nMergedPolyVtxCount++] = nClippedPoly[i];
						continue;
					}

					// Calculate the next index in the exiting poly
					u16 nNext = (u16)(i + 1);
					if ( nNext > nClippedPolyVtxCount )
					{
						nNext = 0;
					}

					u16 nNewIndex;
					for ( ii = 0; ii < pAddCell->m_nClippedVertIdxCount; ii++ )
					{
						nNewIndex = (u16)(pAddCell->m_nFirstClippedVertIdx + ii);
						if ( _ClippedVertIndices[nNewIndex] != nClippedPoly[i] )
						{
							continue;
						}

						u16 nOriginalMergedCount = nMergedPolyVtxCount;
						u16 nLastIndex;
						if ( ii == 0 )
						{
							nLastIndex = (u16)(pAddCell->m_nClippedVertIdxCount - 1);
						}
						else
						{
							nLastIndex = (u16)(ii - 1);
						}

						while ( ii != nLastIndex )
						{
							nMergedPoly[nMergedPolyVtxCount++] = _ClippedVertIndices[nNewIndex];
							ii++;
							if ( ii == pAddCell->m_nClippedVertIdxCount )
							{
								ii = 0;
							}
							if ( ii == nLastIndex )
							{
								break;
							}
							nNewIndex = (u16)(pAddCell->m_nFirstClippedVertIdx + ii);
						}

						if ( _ClippedVertIndices[pAddCell->m_nFirstClippedVertIdx + ii] == nClippedPoly[nNext] )
						{
							// The last vert of the attempted merge matches the next vert of the
							// original list so we have a good merge
							i = nNext;
							bMergedCells = TRUE;
						}
						else
						{
							// The last vert of the attempted merge did not match so we have
							// to continue
							nMergedPolyVtxCount = nOriginalMergedCount;
						}

						break;
					}

					if ( i == 0 )
					{
						// If we merged, i might be set to 0
						break;
					}

					nMergedPoly[nMergedPolyVtxCount++] = nClippedPoly[i];
				}

				if ( bMergedCells )
				{
					// Flag verts that are linear
					CFVec3A vDiff1, vDiff2;
					u16 nCurrent = nMergedPolyVtxCount - 1, nPrior = nMergedPolyVtxCount - 2;
					u8 abRemove[KTC_MAX_CLIPPED_VERTS];
					fang_MemZero( abRemove, sizeof( u8 ) * nMergedPolyVtxCount );
					for ( i = 0; i < nMergedPolyVtxCount; i++ )
					{
						vDiff1.Sub( _avClippedVertPool[nMergedPoly[nCurrent]].vPos, _avClippedVertPool[nMergedPoly[nPrior]].vPos );
						vDiff2.Sub( _avClippedVertPool[nMergedPoly[i]].vPos, _avClippedVertPool[nMergedPoly[nCurrent]].vPos );
						vDiff1.Unitize();
						vDiff2.Unitize();
						if ( vDiff1.Dot( vDiff2 ) > 0.999f )
						{
							// Remove
							abRemove[nCurrent] = TRUE;
						}
						nPrior = nCurrent;
						nCurrent = (u16)i;
					}

					// Compact the list
					u16 iOldCount = nMergedPolyVtxCount;
					nMergedPolyVtxCount = 0;
					for ( i = 0; i < iOldCount; i++ )
					{
						if ( !abRemove[i] )
						{
							nMergedPoly[nMergedPolyVtxCount++] = nMergedPoly[i];
						}
					}

					// Make sure this poly is capable of triangulation
					if ( _Triangulate( nMergedPoly, nMergedPolyVtxCount, 0, &pTri->FaceUnitNorm, NULL ) != FMATH_MAX_FLOAT )
					{
						// Successful merge
						if ( _nClippedVertIndexCount + nMergedPolyVtxCount > KTC_MAX_CLIPPED_VERT_INDICES )
						{
							return KTC_EXCEEDED_VERTEX_CLIP_BUFFER;
						}

						// No more indices for the added cell
						m_Results.m_nTrisSavedByMerger += pAddCell->m_nClippedVertIdxCount - 2;
						pAddCell->m_nClippedVertIdxCount = 0;

						pClipCell->m_nFirstClippedVertIdx = _nClippedVertIndexCount;
						pClipCell->m_nClippedVertIdxCount = nMergedPolyVtxCount;
						for ( i = 0; i < nMergedPolyVtxCount; i++ )
						{
							_ClippedVertIndices[_nClippedVertIndexCount++] = nMergedPoly[i];
						}

						break;
					}
					else
					{
						bMergedCells = FALSE;
					}
				}
			}

			if ( bMergedCells )
			{
				break;
			}

			pAddCell = pAddCell->Next();
		}

		if ( !bMergedCells )
		{
			// We couldn't merge any cells with this poly, so just triangulate it
			AddPoly( nClippedPoly, nClippedPolyVtxCount, pTri, nMatID, pClipVol );
			pClipCell->m_nClippedVertIdxCount = 0;
		}
	}

	return KTC_NO_ERROR;
}


//
//
//
BOOL CKongClipper::ClipToPlane( u16 *pnSource, u16 *pnVertCount, CFVec3A *pPlaneNormal, CFVec3A *pRefPoint )
{
	FASSERT( pnVertCount && pnSource && pPlaneNormal && pRefPoint );
	FASSERT( *pnVertCount < KTC_MAX_CLIP_BUFFER_POINTS );
	FASSERT( *pnSource < 2 );

	CFVec3A vDiff;
	float afDistance[KTC_MAX_CLIP_BUFFER_POINTS];

	_ClipperVert *pSourceVerts = _avClipBuffers[ *pnSource ];

	// Determine the distance to the plane
	u32 i, nInsideBits = 0;
	for ( i = 0; i < *pnVertCount; i++ )
	{
		vDiff.Sub( pSourceVerts[i].vPos, *pRefPoint );
		afDistance[i] = vDiff.Dot( *pPlaneNormal );
		
		if ( afDistance[i] >= 0 )
		{
			nInsideBits |= (1 << i);
		}
	}

	// If all of the points are on the negative side of the 
	// plane, no clipping is necessary and we return FALSE.
	if ( nInsideBits == 0 )
	{
		return FALSE;
	}

	// If all of the points are on the positive side of the 
	// plane, no clipping is necessary and we return TRUE.
	if ( nInsideBits == (u32)(1 << (*pnVertCount)) - 1 )
	{
		return TRUE;
	}

	// Swap buffers
	(*pnSource) = (*pnSource + 1) & 0x01;
	_ClipperVert *paNewVerts = _avClipBuffers[*pnSource];

	// Otherwise, the plane separates the points and we need
	// to perform clipping.  Edges created by the verts will
	// be clipped to the positive side of the plane.
	u32 nNewVertCount = 0;
	u32 iPriorVert = *pnVertCount - 1;

	for ( i = 0; i < *pnVertCount; i++ )
	{
		if ( afDistance[i] >= 0 )
		{
			if ( afDistance[iPriorVert] >= 0 )
			{
				// Edge is entirely on positive side so we don't need to clip
				// Add the current vert.
				paNewVerts[nNewVertCount++].Set( &pSourceVerts[i] );
			}
			else
			{
				// We're going from positive to negative side, so we need to clip.
				f32 fLerp = afDistance[i] / (afDistance[i] - afDistance[iPriorVert]);
				paNewVerts[nNewVertCount].ReceivePositionLerp( fLerp, &pSourceVerts[i], &pSourceVerts[iPriorVert] );
				paNewVerts[nNewVertCount++].ReceiveAttributeLerp( fLerp, &pSourceVerts[i], &pSourceVerts[iPriorVert] );

				// Add in the current vert, too, since it is on the positive side
				paNewVerts[nNewVertCount++].Set( &pSourceVerts[i] );
			}
		}
		else if (afDistance[iPriorVert] >= 0 )
		{
			// We're going from negative to positive side, so we need to clip.
			f32 fLerp = afDistance[iPriorVert] / (afDistance[iPriorVert] - afDistance[i]);
			paNewVerts[nNewVertCount].ReceivePositionLerp( fLerp, &pSourceVerts[iPriorVert], &pSourceVerts[i] );
			paNewVerts[nNewVertCount++].ReceiveAttributeLerp( fLerp, &pSourceVerts[iPriorVert], &pSourceVerts[i] );
		}

		// If we fail all of the above tests, the edge is entirely on the 
		// negative side, so we don't need to clip and don't want to use 
		// the current vert.

		iPriorVert = i;
	}

	// Set the new vert count	
	(*pnVertCount) = (u16)nNewVertCount;

	return TRUE;
}	


//
//
//
u32 CKongClipper::AddPoly( u16 *pVertIndices, u16 nIndexCount, KongTri_t *pTri, u32 nMatID, CKTC_ClipVolume *pClipVolume )
{
	FASSERT( pVertIndices && nIndexCount > 2 && pTri && pClipVolume );

	// Make sure we don't exceed the buffer
	if ( _nKongTrisUsed + nIndexCount - 2 >= _MAX_CLIPPED_KONG_TRIS )
	{
		FASSERT_NOW;
		DEVPRINTF( "CKongClipper::AddPoly() - Warning!!!  Exceeded clipper triangle buffer.\n" );
		return KTC_EXCEEDED_VERTEX_CLIP_BUFFER;
	}

	// Get the best organization of the indices for triangulation
	u16 *pIndices = _GetBestTriangulation( pVertIndices, nIndexCount, &pTri->FaceUnitNorm );
	if ( !pIndices )
	{
//		FASSERT_NOW;
		return KTC_ERROR_TRIANGULATING_CLIPPED_POLY;
	}

	u16 nTriCount = 0;
	u16 nVert0, nVert1, nVert2;
	while ( *pIndices != 0xffff )
	{
		nVert0 = pIndices[0];
		nVert1 = pIndices[1];
		nVert2 = pIndices[2];
		pIndices += 3;

		FASSERT( nTriCount < (nIndexCount - 2) * 3 );
		nTriCount++;

		if ( _nKongVertsUsed + 3 > _MAX_CLIPPED_KONG_VERTS )
		{
			DEVPRINTF( "CKongClipper::AddPoly() - Warning!!!  Exceeded clipper vertex buffer.\n" );
			return KTC_EXCEEDED_VERTEX_CLIP_BUFFER;
		}

		// Setup Kong Verts
#if 0
		if ( !_avClippedVertPool[nVert0].pKongPoolVert )
		{
			_ClippedKongVerts[_nKongVertsUsed] = *pTri->apKongVerts[0];
			_avClippedVertPool[nVert0].AssignToKongVert( &_ClippedKongVerts[_nKongVertsUsed] );
			_avClippedVertPool[nVert0].pKongPoolVert = &_ClippedKongVerts[_nKongVertsUsed];
			_nKongVertsUsed++;
		}

		if ( !_avClippedVertPool[nVert1].pKongPoolVert )
		{
			_ClippedKongVerts[_nKongVertsUsed] = *pTri->apKongVerts[1];
			_avClippedVertPool[nVert1].AssignToKongVert( &_ClippedKongVerts[_nKongVertsUsed] );
			_avClippedVertPool[nVert1].pKongPoolVert = &_ClippedKongVerts[_nKongVertsUsed];
			_nKongVertsUsed++;
		}

		if ( !_avClippedVertPool[nVert2].pKongPoolVert )
		{
			_ClippedKongVerts[_nKongVertsUsed] = *pTri->apKongVerts[2];
			_avClippedVertPool[nVert2].AssignToKongVert( &_ClippedKongVerts[_nKongVertsUsed] );
			_avClippedVertPool[nVert2].pKongPoolVert = &_ClippedKongVerts[_nKongVertsUsed];
			_nKongVertsUsed++;
		}

		// Setup Kong Tri
		KongTri_t *pNewTri = &_ClippedKongTris[_nKongTrisUsed];
		(*pNewTri) = *pTri;
		pNewTri->nNumEdgeTris = 0;
		pNewTri->apKongVerts[0] = _avClippedVertPool[nVert0].pKongPoolVert;
		pNewTri->apKongVerts[1] = _avClippedVertPool[nVert1].pKongPoolVert;
		pNewTri->apKongVerts[2] = _avClippedVertPool[nVert2].pKongPoolVert;
#else
		// Setup Kong Tri
		KongTri_t *pNewTri = &_ClippedKongTris[_nKongTrisUsed];
		(*pNewTri) = *pTri;
		pNewTri->nNumEdgeTris = 0;

		_ClippedKongVerts[_nKongVertsUsed] = *pTri->apKongVerts[0];
		_avClippedVertPool[nVert0].AssignToKongVert( &_ClippedKongVerts[_nKongVertsUsed] );
		pNewTri->apKongVerts[0] = &_ClippedKongVerts[_nKongVertsUsed];
		_nKongVertsUsed++;

		_ClippedKongVerts[_nKongVertsUsed] = *pTri->apKongVerts[1];
		_avClippedVertPool[nVert1].AssignToKongVert( &_ClippedKongVerts[_nKongVertsUsed] );
		pNewTri->apKongVerts[1] = &_ClippedKongVerts[_nKongVertsUsed];
		_nKongVertsUsed++;

		_ClippedKongVerts[_nKongVertsUsed] = *pTri->apKongVerts[2];
		_avClippedVertPool[nVert2].AssignToKongVert( &_ClippedKongVerts[_nKongVertsUsed] );
		pNewTri->apKongVerts[2] = &_ClippedKongVerts[_nKongVertsUsed];
		_nKongVertsUsed++;
#endif

		// Make sure we don't add a zero area triangle
		BOOL bSkip = FALSE;
		CFVec3 Vec1, Vec2, Vec3;
		f32 afMag2[3];
		Vec1 = pNewTri->apKongVerts[0]->Pos - pNewTri->apKongVerts[1]->Pos;
		afMag2[0] = Vec1.Mag2();
		Vec2 = pNewTri->apKongVerts[2]->Pos - pNewTri->apKongVerts[1]->Pos;
		afMag2[1] = Vec2.Mag2();
		Vec3 = Vec1.Cross( Vec2 );
		afMag2[2] = Vec3.Mag2();
		if ( afMag2[0] < 0.00000001f ||
			 afMag2[1] < 0.00000001f ||
			 afMag2[2] < 0.00000001f ) 
		{
			bSkip = TRUE;
			m_Results.m_nResultingZeroAreaTris++;
		}

		if ( !bSkip )
		{
			// Build Tri bounding sphere
			utils_ComputeTriBound( pNewTri->BoundSphere, pNewTri->apKongVerts[0]->Pos, pNewTri->apKongVerts[1]->Pos, pNewTri->apKongVerts[2]->Pos );
#if 1
			// Add the triangle to the volume linklist
			if ( !AddTriToVolume( pNewTri, pClipVolume->m_pVolume, TRUE, nMatID ) )
			{
				DEVPRINTF( "CKongClipper - ERROR - Unable to add tri to volume.\n" );
				return KTC_CALLBACK_ERROR;
			}
#endif
			// Advance our counter
			_nKongTrisUsed++;
		}
	}

	return KTC_NO_ERROR;
}


//
//
//
static f32 _GetValue( u32 nV0, u32 nV1, u32 nV2, CFVec3 *pTriNormal )
{
	CFVec3A vDiff01, vDiff02, vNormal;
	vDiff01.Sub( _avClippedVertPool[nV0].vPos, _avClippedVertPool[nV1].vPos );
	vDiff02.Sub( _avClippedVertPool[nV0].vPos, _avClippedVertPool[nV2].vPos );

	// Test against the normal to make sure the winding order is the same
	vNormal.Cross( vDiff02, vDiff01 );
	if ( vNormal.v3.Dot( *pTriNormal ) < 0.001f )
	{
		return -FMATH_MAX_FLOAT;
	}

	// We have a valid normal.  Now calculate the "value"

	CFVec3A vDiff12;
	vDiff12.Sub( _avClippedVertPool[nV1].vPos, _avClippedVertPool[nV2].vPos );

	f32 fDist01 = vDiff01.Mag();
	f32 fDist12 = vDiff12.Mag();
	f32 fDist02 = vDiff02.Mag();

	f32 fValue = 0.f;
	if ( fDist01 > fDist12 && fDist01 > fDist02 )
	{
		return (fDist12 + fDist02) / fDist01;
	}
	else if ( fDist02 > fDist12 && fDist02 > fDist01 )
	{
		return (fDist12 + fDist01) / fDist02;
	}

	return (fDist01 + fDist02) / fDist12;
}


//
//
//
#define _MAX_INDICES_IN_SOLUTION	256
static u16* _GetBestTriangulation( u16 *aVertIndices, u16 nVertCount, CFVec3 *pTriNormal )
{
	FASSERT( aVertIndices && pTriNormal );


	// Make sure we don't exceed our buffer
	if ( (nVertCount - 2) * 3 > _MAX_INDICES_IN_SOLUTION )
	{
		DEVPRINTF( "_GetBestTriangulation() - Not enough indices to triangulate clipped poly.\n" );
		return NULL;
	}

	u32 i;
	f32 fBestValue = -FMATH_MAX_FLOAT;
	s32 nBestValueIndex = -1;
	for ( i = 0; i < nVertCount; i++ )
	{
		f32 fValue = _Triangulate( aVertIndices, nVertCount, (u16)i, pTriNormal, NULL );
		if ( fValue != FMATH_MAX_FLOAT && fValue > fBestValue )
		{
			fBestValue = fValue;
			nBestValueIndex = i;			
		}
	}
	if ( nBestValueIndex < 0 )
	{
		_nNoGoodTriangulationPointCount++;
		return NULL;
	}

	static u16 *pSolution;
	if ( _Triangulate( aVertIndices, nVertCount, (u16)nBestValueIndex, pTriNormal, &pSolution ) == FMATH_MAX_FLOAT )
	{
		FASSERT_NOW;
		DEVPRINTF( "_GetBestTriangulation() - Error triangulating clipped poly.\n" );
		return NULL;
	}

	return pSolution;
}

//
//
//
static f32 _Triangulate( u16 *aVertIndices, u16 nVertCount, u16 nStartVert, CFVec3 *pTriNormal, u16 **pSolution )
{
	FASSERT( aVertIndices && pTriNormal );

	static u16 anSolution[_MAX_INDICES_IN_SOLUTION];
	u8 abRemoved[_MAX_INDICES_IN_SOLUTION];

	// Make sure we don't exceed our buffer
	if ( (nVertCount - 2) * 3 > _MAX_INDICES_IN_SOLUTION )
	{
		return FMATH_MAX_FLOAT;
	}

	// Clear the removed flags
	fang_MemZero( abRemoved, sizeof( u8 ) * nVertCount );

	u16 nSolutionCount = 0;
	u16 nLoopPrevent, nNextVert, nLastGoodStartVert = 0;
	u16 nVertsLeft = nVertCount;
	f32 fValue, fMinValue = FMATH_MAX_FLOAT;
	u16 nV0, nV1, nV2;
	u16 nCounter = 0;
	while ( TRUE )
	{
		nCounter++;
		if ( nCounter == 32 )
		{
			nCounter = 0;
		}
		nLoopPrevent = nStartVert;
		while ( TRUE )
		{
			if ( !abRemoved[nStartVert] )
			{
				nV0 = nStartVert;
				break;
			}
			nStartVert++;
			if ( nStartVert == nVertCount )
			{
				nStartVert = 0;
			}
			if ( nStartVert == nLoopPrevent )
			{
				return FMATH_MAX_FLOAT;
			}
			if ( nStartVert == nLastGoodStartVert )
			{
				// We looped all the way around the verts without finding
				// a valid triangle to add, so this is a bad set of vert indices
				// and we should bail out
				return FMATH_MAX_FLOAT;
			}
		}

		nNextVert = nStartVert;

		nLoopPrevent = nNextVert;
		while ( TRUE )
		{
			if ( nNextVert != nV0 && !abRemoved[nNextVert] )
			{
				nV1 = nNextVert;
				break;
			}
			nNextVert++;
			if ( nNextVert == nVertCount )
			{
				nNextVert = 0;
			}
			if ( nNextVert == nLoopPrevent )
			{
				return FMATH_MAX_FLOAT;
			}
		}

		nLoopPrevent = nNextVert;
		while ( TRUE )
		{
			if ( nNextVert != nV1 && nNextVert != nV0 && !abRemoved[nNextVert] )
			{
				nV2 = nNextVert;
				break;
			}
			nNextVert++;
			if ( nNextVert == nVertCount )
			{
				nNextVert = 0;
			}
			if ( nNextVert == nLoopPrevent )
			{
				return FMATH_MAX_FLOAT;
			}
		}

		fValue = _GetValue( aVertIndices[nV0], aVertIndices[nV1], aVertIndices[nV2], pTriNormal );

		if ( fValue == -FMATH_MAX_FLOAT )
		{
			// Triangle is not valid so try again after changing the start vert
			nStartVert++;
			if ( nStartVert == nVertCount )
			{
				nStartVert = 0;
			}
			if ( nStartVert == nLastGoodStartVert )
			{
				// We looped all the way around the verts without finding
				// a valid triangle to add, so this is a bad set of vert indices
				// and we should bail out
				return FMATH_MAX_FLOAT;
			}

			continue;
		}

		// Keep track of the minimum value
		if ( fValue < fMinValue )
		{
			fMinValue = fValue;
		}

		// Bounds check
		if ( nSolutionCount + 3 > _MAX_INDICES_IN_SOLUTION )
		{
			return FMATH_MAX_FLOAT;
		}

		// Record this try in the "solution"
		anSolution[nSolutionCount++] = aVertIndices[nV0];
		anSolution[nSolutionCount++] = aVertIndices[nV1];
		anSolution[nSolutionCount++] = aVertIndices[nV2];

		// If we only had 3 verts left, these formed the last triangle and we should stop
		if ( --nVertsLeft < 3 )
		{
			break;
		}

		// Flag the middle vert as removed
		abRemoved[nV1] = TRUE;
		nLastGoodStartVert = nStartVert;
	}

	anSolution[nSolutionCount] = 0xffff;

	if ( pSolution )
	{
		*pSolution = anSolution;
	}

	return fMinValue;
}


