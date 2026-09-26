//////////////////////////////////////////////////////////////////////////////////////
// KongToChimp.cpp - 
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
// 05/21/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "KongToChimp.h"
#include "utils.h"


#define _CHIMP_ID_INDEX		0
#define _MAT_ID_INDEX		1

u16 CKongToChimp::m_nChimpCount = 0;


CKongToChimp::CKongToChimp() {
	m_nMaxTris = 0;
	m_pSeg = NULL;
	m_bPreserveEdgeData = FALSE;
	m_nChimpID = 0;
}

CKongToChimp::~CKongToChimp() {
	FreeData();
}

BOOL CKongToChimp::CreateEmptyMeshFromTemplate( const CApeToKongFormat &rTemplate, 
											    u32 nMaxTris, 
												BOOL bPreserveEdgeData/*=FALSE*/ ) {
	
	u32 i;
	FreeData();

	if( !rTemplate.m_bA2KConverted ||
		rTemplate.m_bSkinned ||
		!rTemplate.m_pKongMesh->nMaterialCount ||
		!rTemplate.m_pKongMesh->nTriangleCount ||
		rTemplate.m_pKongMesh->nSegmentCount != 1 ||
		!nMaxTris ) {
		// only single segment, non-skinned kong templates are supported
		return FALSE;
	}

	// grab a unique chimp id
	m_nChimpID = m_nChimpCount++;

	// record whether or not the incoming tris will have valid edge data
	m_bPreserveEdgeData = bPreserveEdgeData;

	////////////////////////
	// clone the loaded file
	if( !CloneFromLoadedFile( rTemplate ) ) {
		return FALSE;
	}

	////////////////////////////
	// allocate our working vars
	KongSeg_t *pSrcSeg;
	KongMat_t *pMat;
	
	//////////////////
	// allocate memory
//	m_nKongBytesAllocated = ComputeNeededMemory( rTemplate.m_pKongMesh->nMaterialCount, nMaxTris );
//	m_pKongMemAllocated = new u8[m_nKongBytesAllocated];
//	if( !m_pKongMemAllocated ) {
//		return FALSE;
//	}
//	ZeroMemory( m_pKongMemAllocated, m_nKongBytesAllocated );


	// Allocate Kong Mesh
	if ( m_pKongMesh )
	{
		delete m_pKongMesh;
	}
	m_pKongMesh = new KongMesh_t;
	if ( !m_pKongMesh )
	{
		FreeData();
		return FALSE;
	}
	m_pKongMesh->AllocateArrays( 1, rTemplate.m_pKongMesh->nMaterialCount, 0, nMaxTris, 0 );
	m_pKongMesh->KHeader.AIDLastWriteTime = rTemplate.m_pKongMesh->KHeader.AIDLastWriteTime;
	m_pKongMesh->KHeader.APELastWriteTime = rTemplate.m_pKongMesh->KHeader.APELastWriteTime;
	m_pKongMesh->KHeader.bHasAidFile = rTemplate.m_pKongMesh->KHeader.bHasAidFile;
	m_pKongMesh->KHeader.Header = rTemplate.m_pKongMesh->KHeader.Header;
	m_pKongMesh->paMatProperties = rTemplate.m_pKongMesh->paMatProperties;
	m_pKongMesh->paVertices = rTemplate.m_pKongMesh->paVertices;
	m_pKongMesh->nSegmentCount = 1;
	m_pKongMesh->bWorldFile = TRUE;
	m_pKongMesh->nMatPropertiesCount = rTemplate.m_pKongMesh->nMatPropertiesCount;
	m_pKongMesh->nMatPropertiesAllocated = rTemplate.m_pKongMesh->nMatPropertiesAllocated;
	m_pKongMesh->nVertCount = rTemplate.m_pKongMesh->nVertCount;
	m_pKongMesh->nVertsAllocated = rTemplate.m_pKongMesh->nVertsAllocated;

	/*
	// Allocate memory for Kong triangles
	if ( m_paKongTri )
	{
		delete[] m_paKongTri;
	}
	m_nNumKongTris = 0;
	m_nMaxTris = nMaxTris;
	m_paKongTri = new KongTri_t[m_nMaxTris];
	if ( !m_paKongTri ) 
	{
		FreeData();
		return FALSE;
	}
	ZeroMemory( m_paKongTri, sizeof( KongTri_t ) * m_nMaxTris );
	m_pKongMesh->nTriangleCount = m_nMaxTris;
	m_pKongMesh->paTriangles = m_paKongTri;
	for ( i = 0; i < m_nMaxTris; i++ )
	{
		m_paKongTri[i].nTriIdx = i;
	}

	/////////////
	// fixup ptrs
//	m_pKongMesh = (KongMesh_t *)m_pKongMemAllocated;
//	m_pKongMesh->apSegs = AllocatePtrArray();
//	if( !m_pKongMesh->apSegs ) {
//		return FALSE;
//	}
//	m_pKongWorkMem = (u8 *)&m_pKongMesh[1];
//	m_pKongTri = (KongTri_t *)&m_pKongMemAllocated[m_nKongBytesAllocated];
//	m_pKongTri -= nMaxTris;

	// Allocate Kong Segments
	if ( m_paKongSegments )
	{
		delete[] m_paKongSegments;
	}
	m_nNumKongSegs = 1;
	m_paKongSegments = new KongSeg_t[m_nNumKongSegs];
	if( !m_paKongSegments ) {
		return FALSE;
	}
	m_pKongMesh->paSegs = m_paKongSegments;
	m_pKongMesh->nSegmentCount = 1;

	// Allocate Kong Materials
	if ( m_paKongMaterials )
	{
		delete[] m_paKongMaterials;
	}
	m_nNumKongMats = rTemplate.m_pKongMesh->nMaterialCount;
	m_paKongMaterials = new KongMat_t[m_nNumKongMats];
	if( !m_paKongMaterials ) {
		return FALSE;
	}
	m_pKongMesh->paMaterials = m_paKongMaterials;
	m_pKongMesh->nMaterialCount = 0;
	for ( i = 0; i < m_nNumKongMats; i++ )
	{
		// Set the material indices
		m_paKongMaterials[i].nMatIdx = i;
	}
*/
	/////////////////
	// set our counts
	m_bA2KConverted = FALSE;
	m_bSkinned = FALSE;

	//////////////////////
	// clone the 1 segment
	pSrcSeg = &rTemplate.m_pKongMesh->paSegs[0];
	FASSERT( pSrcSeg->nNumBonesUsed == 0 );
	FASSERT( pSrcSeg->nNumMats > 0 );
	
	m_pSeg = m_pKongMesh->paSegs;

	//////////////////////////////////////////////////////
	// clone the materials, but don't add any tris to them
	for ( i = 0; i < rTemplate.m_pKongMesh->nMaterialCount; i ++ )
	{
		FASSERT( m_pKongMesh->nMaterialCount < m_pKongMesh->nMaterialsAllocated );
		pMat = &m_pKongMesh->paMaterials[m_pKongMesh->nMaterialCount++];
		pMat->SetWithEmptyTris( &rTemplate.m_pKongMesh->paMaterials[i] );
		pMat->nMatIdx = i;
		pMat->nNextMatIdx = KONG_INVALID_MAT_INDEX;
		pMat->nMatFlags = KONGMAT_FLAGS_NONE;

		m_pKongMesh->AddMatToSeg( m_pSeg, pMat );
	}
	// finally record the max number of tris possible
	m_nMaxTris = nMaxTris;

	return TRUE;
}

// rTri must have the following fields filled in:
// paApeVerts[3]
// pApeMat
// nNumBoneUsed == 0
// FaceUnitNorm
// BoundSphere
// nNumEdgeTris	(ONLY IF WE ARE PRESERVING THE EDGE DATA)
// paEdgeTris	(ONLY IF WE ARE PRESERVING THE EDGE DATA)
// all fields will be ignored as their validity can't be assured
BOOL CKongToChimp::AddTriToMesh( KongTri_t &rTri, u32 nMatNum ) 
{
	KongMat_t *pMat;
	KongTri_t *pTri;
	u32 i;

	if (m_bA2KConverted ||
		m_pKongMesh->nTriangleCount >= m_nMaxTris ||
		nMatNum >= m_pKongMesh->nMaterialCount ||
		rTri.nNumBonesUsed != 0 ) 
	{
		return FALSE;
	}

	FASSERT( nMatNum < m_pSeg->nNumMats );

	pMat = &m_pKongMesh->paMaterials[nMatNum];
	pTri = &m_pKongMesh->paTriangles[m_pKongMesh->nTriangleCount];
	
	// pTri has already been zero'ed out, so only set valid fields
	pTri->apKongVerts[0] = rTri.apKongVerts[0];
	pTri->apKongVerts[1] = rTri.apKongVerts[1];
	pTri->apKongVerts[2] = rTri.apKongVerts[2];
	pTri->FaceUnitNorm = rTri.FaceUnitNorm;
	pTri->BoundSphere = rTri.BoundSphere;
	pTri->nNextTriIdx = KONG_INVALID_TRI_INDEX;
	pTri->nTriIdx = m_pKongMesh->nTriangleCount;

	if ( m_bPreserveEdgeData ) 
	{
		pTri->nNumEdgeTris = rTri.nNumEdgeTris;

		FASSERT( pTri->nNumEdgeTris <= 3 );
		
		for ( i = 0; i < rTri.nNumEdgeTris; i++ ) 
		{
			// record the current ptr, we'll fix it up to point into our data later
			pTri->paEdgeTris[i] = rTri.paEdgeTris[i];			
		}

		// record some data into the old tri
		rTri.nUser[_CHIMP_ID_INDEX] = m_nChimpID;
		rTri.nUser[_MAT_ID_INDEX] = (u16)nMatNum;
		rTri.pUser = pTri;

		// record the mat id in our user field
		pTri->nUser[_CHIMP_ID_INDEX] = m_nChimpID;
		pTri->nUser[_MAT_ID_INDEX] = (u16)nMatNum;
	}

	// increase the tri count
	m_pKongMesh->nTriangleCount++;	

	// add the tri to the material's tri list
	m_pKongMesh->AddTriToMat( pMat, pTri );

	return TRUE;
}

BOOL CKongToChimp::CloseMesh( BOOL bRemoveUnusedMats/*=TRUE*/ ) {
	u32 i, j, nOrigEdgeCount;
	KongTri_t *pNew, *pOld;

	if ( !m_pKongMesh )
	{
		return FALSE;
	}

	if( m_bA2KConverted ) {
		// already closed the mesh
		return TRUE;
	}

	#if FANG_DEBUG_BUILD
		m_pKongMesh->VerifySegMatList( m_pKongMesh->paSegs );
	#endif

	// fixup the edge ptr of our tris
	if( m_bPreserveEdgeData ) 
	{
		// walk the tris
		pNew = m_pKongMesh->paTriangles;
		for( i=0; i < m_pKongMesh->nTriangleCount; i++ ) 
		{
			
			// see if the tri has any connected edges
			if( pNew->nNumEdgeTris ) 
			{

				// walk the known edges, seeing if the connected tri is the same material and is part of this chimp
				nOrigEdgeCount = pNew->nNumEdgeTris;
				for( j=0; j < nOrigEdgeCount; j++ ) 
				{
					pOld = pNew->paEdgeTris[j];
					
					if( pOld->nUser[_CHIMP_ID_INDEX] == pNew->nUser[_CHIMP_ID_INDEX] &&
						pOld->nUser[_MAT_ID_INDEX] == pNew->nUser[_MAT_ID_INDEX] &&
						utils_DoTheseTrisShareAnEdge( *pOld, *pNew ) ) 
					{
						// pOld tri is part of our chimp material, update our edge ptr
						pNew->paEdgeTris[j] = (KongTri_t *)pOld->pUser;

						FASSERT( pNew->paEdgeTris[j]->nUser[_MAT_ID_INDEX] == pNew->nUser[_MAT_ID_INDEX] );

					} 
					else 
					{
						// pOld tri is not part of our chimp material, remove it from our edge list
						pNew->paEdgeTris[j] = NULL;
						pNew->nNumEdgeTris--;
					}
				}
				if( nOrigEdgeCount != pNew->nNumEdgeTris ) 
				{
					
					switch( pNew->nNumEdgeTris ) 
					{
						case 0:
						case 3:
							// no shifting needed
							break;

						case 1:
							// since only 1 edge is known about, it should be in the first spot
							if( pNew->paEdgeTris[1] ) {
								pNew->paEdgeTris[0] = pNew->paEdgeTris[1];
								pNew->paEdgeTris[1] = NULL;
							} else if( pNew->paEdgeTris[2] ) {
								pNew->paEdgeTris[0] = pNew->paEdgeTris[2];
								pNew->paEdgeTris[2] = NULL;
							}
							break;
						case 2:
							// since 2 edges are known about, it should be in the first 2 spots
							if( pNew->paEdgeTris[2] ) {
								// this needs to be in spot [0] or [1], whichever is empty
								if( pNew->paEdgeTris[0] ) {
									pNew->paEdgeTris[1] = pNew->paEdgeTris[2];								
								} else {
									// swap the order so that stripping produces the same results
									pNew->paEdgeTris[0] = pNew->paEdgeTris[1];
									pNew->paEdgeTris[1] = pNew->paEdgeTris[2];
								}
								pNew->paEdgeTris[2] = NULL;
							}
							break;
					}
				}
			}

			// move to the next new tri
			pNew++;
		}
	}

	#if FANG_DEBUG_BUILD
		m_pKongMesh->VerifySegMatList( m_pKongMesh->paSegs );
	#endif

	if ( bRemoveUnusedMats ) 
	{
		m_pKongMesh->RemoveEmptyMaterials();
	}

	m_bA2KConverted = TRUE;

	return TRUE;
}

void CKongToChimp::FreeData() 
{
	m_nMaxTris = 0;

	// call the base class's freedata too
	CApeToKongFormat::FreeData();
}

