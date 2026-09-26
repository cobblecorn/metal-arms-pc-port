//////////////////////////////////////////////////////////////////////////////////////
// ApeToKongFormat.cpp - loads and converts an ape file to the kong format
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
// 01/25/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "Pasm.h"
#include "ApeToKongFormat.h"
#include "utils.h"
#include "ErrorLog.h"
#include "fmath.h"
#include "fshaders.h"
#include "CompileDlg.h"

#define _SHOW_DETAILED_TIMINGS					FALSE

#define _ERROR_HEADING							"APE->KONG FORMAT CONVERTER "

#define _REMOVE_DEGENERATIVE_TRIS				FALSE
#define _SMALLEST_POSSIBLE_TRI_AREA2			( 0.000001f * 0.000001f )
#define _GENERATE_BACKFACES_FOR_2SIDED_MATS		TRUE

CApeToKongFormat::CApeToKongFormat() {
	m_bA2KConverted = FALSE;
	m_pKongMesh = NULL;
	m_bSkinned = FALSE;
	m_bVolumeMesh = FALSE;

	m_aPtrArrays.RemoveAll();
}

CApeToKongFormat::~CApeToKongFormat() {
	FreeData();
}

BOOL CApeToKongFormat::LoadApeFile( cchar *pszFilename ) {

	// unload any data that may be loaded
	FreeData();

	// load the ape file
	if( !CApeFileLoader::LoadApeFile( pszFilename ) ) {
		return FALSE;
	}

	return TRUE;
}

BOOL CApeToKongFormat::ConvertApeFile( CCompileDlg *pDlg, cchar *pszFilename, u32 nSigDigits/*=3*/ ) {

	// convert to kong format
	if( !ConvertToKongFormat( pDlg, nSigDigits ) ) {
		return FALSE;
	}
	m_bA2KConverted = TRUE;
	return TRUE;
}

/*
void CApeToKongFormat::ResetTriAddedVar( KongMat_t &Mat ) const 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	KongTri_t *pTri = m_pKongMesh->GetFirstTri( &Mat );
	while ( pTri )
	{
		pTri->bTriAdded = FALSE;
		pTri = m_pKongMesh->GetNextTri( pTri );
	}
}

void CApeToKongFormat::ResetTriDist2Var( KongMat_t &Mat ) const 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	KongTri_t *pTri = m_pKongMesh->GetFirstTri( &Mat );
	while ( pTri )
	{
		pTri->fDist2 = 0.0f;
		pTri = m_pKongMesh->GetNextTri( pTri );
	}
}
*/
void CApeToKongFormat::ComputeMeshBoundingBoxFromKongTris( CFVec3 &vMin, CFVec3 &vMax ) const 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	// compute the mix/max for all of the tris
	FindMinAndMaxPosFromKongTris( vMin, vMax, m_pKongMesh->paTriangles, m_pKongMesh->nTriangleCount );
}

void CApeToKongFormat::ComputeMeshBoundingSphereFromKongTris( CFVec3 &Center, f32 &fRadius, BOOL bVolumeMesh/*=FALSE*/ ) const 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	m_pKongMesh->ComputeBoundingSphere( Center, fRadius, bVolumeMesh );
}

void CApeToKongFormat::ComputeSegBoundingSphereFromKongTris( u32 nSegmentNum, CFVec3 &Center, f32 &fRadius ) const {

	if( !m_pKongMesh ) {
		return;
	}
	if( nSegmentNum >= m_pKongMesh->nSegmentCount ) {
		// invalid segment num
		return;
	}
	KongSeg_t *pSeg = &m_pKongMesh->paSegs[nSegmentNum];
	KongMat_t *pMat;

	f32 fRadiusOld;
	CFVec3 Min, Max, vCenterOld;
	pMat = m_pKongMesh->GetFirstMaterial( pSeg );
	BOOL bFirstCalc = FALSE;
	while ( pMat )
	{
		KongTri_t *pTri = m_pKongMesh->GetFirstTri( pMat );
		while ( pTri )
		{
			FindMinAndMaxPosFromKongTris( Min, Max, pTri, 1, bFirstCalc );
			pTri = m_pKongMesh->GetNextTri( pTri );
			bFirstCalc = TRUE;
		}
		pMat = m_pKongMesh->GetNextMaterial( pMat );
	}
	utils_GetBoundingSphereFromMinMax( vCenterOld, fRadiusOld, Min, Max );

	Center.Zero();
	u32 nTotalVerts = 0;
	pMat = m_pKongMesh->GetFirstMaterial( pSeg );
	while ( pMat )
	{
		KongTri_t *pTri = m_pKongMesh->GetFirstTri( pMat );
		while ( pTri )
		{
			nTotalVerts += 3;
			Center += pTri->apKongVerts[0]->Pos;
			Center += pTri->apKongVerts[1]->Pos;
			Center += pTri->apKongVerts[2]->Pos;
			pTri = m_pKongMesh->GetNextTri( pTri );
		}
		pMat = m_pKongMesh->GetNextMaterial( pMat );
	}
	Center /= (f32)nTotalVerts;
	f32 fTest;
	fRadius = 0.f;
	CFVec3 vTest;
	pMat = m_pKongMesh->GetFirstMaterial( pSeg );
	while ( pMat )
	{
		KongTri_t *pTri = m_pKongMesh->GetFirstTri( pMat );
		while ( pTri )
		{
			vTest = Center - pTri->apKongVerts[0]->Pos;
			fTest = vTest.Mag2();
			if ( fTest > fRadius )
			{
				fRadius = fTest;
			}
			vTest = Center - pTri->apKongVerts[1]->Pos;
			fTest = vTest.Mag2();
			if ( fTest > fRadius )
			{
				fRadius = fTest;
			}
			vTest = Center - pTri->apKongVerts[2]->Pos;
			fTest = vTest.Mag2();
			if ( fTest > fRadius )
			{
				fRadius = fTest;
			}
			pTri = m_pKongMesh->GetNextTri( pTri );
		}
		pMat = m_pKongMesh->GetNextMaterial( pMat );
	}
	fRadius = fmath_AcuSqrt( fRadius ) * UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;

	if ( fRadiusOld < fRadius )
	{
		Center = vCenterOld;
		fRadius = fRadiusOld;
	}
}

void CApeToKongFormat::FindMinAndMaxPosFromKongTris( CFVec3 &Min, CFVec3 &Max,
												     KongTri_t *pTri, u32 nNumTris,
												     BOOL bUseMinMax/*=FALSE*/ ) const {
	CFVec3 *pV;
	u32 i, j;

	if( !pTri || !nNumTris ) {
		return;
	}
	if( !bUseMinMax ) {
		// use the first xyz as both the min & max
		pV = (CFVec3 *)&pTri[0].apKongVerts[0]->Pos;
		Min.Set( pV->x, pV->y, pV->z );
		Max = Min;
	}	
	for( i=0; i < nNumTris; i++ ) {
		for( j=0; j < 3; j++ ) {
			pV = (CFVec3 *)&pTri[i].apKongVerts[j]->Pos;
			
			Min.x = FMATH_MIN( pV->x, Min.x );
			Min.y = FMATH_MIN( pV->y, Min.y );
			Min.z = FMATH_MIN( pV->z, Min.z );

			Max.x = FMATH_MAX( pV->x, Max.x );
			Max.y = FMATH_MAX( pV->y, Max.y );
			Max.z = FMATH_MAX( pV->z, Max.z );
		}
	}
}

void CApeToKongFormat::FreeData() {
	m_bA2KConverted = FALSE;
	if( m_pKongMesh ) {
		CPtrArray *pPtrArray;
		for( int i=0; i < m_aPtrArrays.GetSize(); i++ ) {
			pPtrArray = (CPtrArray *)m_aPtrArrays[i];
			pPtrArray->RemoveAll();
			delete pPtrArray;
		}
		m_aPtrArrays.RemoveAll();
	}

	if ( m_pKongMesh )
	{
		delete m_pKongMesh;
	}
	m_pKongMesh = NULL;
	m_bSkinned = FALSE;
	// call base class and unload the file too
	UnloadFile();
}

BOOL IsZeroAreaTri( KongTri_t *pTri )
{
	// Add a tri
	CFVec3 vTemp;
	vTemp = pTri->apKongVerts[0]->Pos - pTri->apKongVerts[1]->Pos;
	if ( vTemp.Mag2() < 0.00000001f )
	{
		return TRUE;
	}
	else
	{
		vTemp = pTri->apKongVerts[1]->Pos - pTri->apKongVerts[2]->Pos;
		if ( vTemp.Mag2() < 0.00000001f )
		{
			return TRUE;
		}
		else
		{
			vTemp = pTri->apKongVerts[2]->Pos - pTri->apKongVerts[0]->Pos;
			if ( vTemp.Mag2() < 0.00000001f )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CApeToKongFormat::ConvertToKongFormat( CCompileDlg *pDlg, u32 nSigDigits ) 
{
	u32 i, j, k, nIndex, nAutoID, nNumTris;
	ApeSegment_t *pApeSeg;
	ApeMaterial_t *pApeMat;
	ApeVert_t *pApeVert;
	ApeVertIndex_t *pApeIndex;
	KongSeg_t *pSeg;
	KongMat_t *pMat;
	KongMatProperties_t KongMatProp;
	KongTri_t *pTri;
	CString sString1, sString2;
	u16 *pauSegMatTriCount = NULL;
	u32 nCurrentKongVertIdx = 0;
	u32 nKongVertCounter = 0;
	u32 nKongTriCounter = 0;
	KongTri_t *pLastTri;
	u32 nZeroArea = 0;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	if ( !GetNumMeshIndices() ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "There are no tris in an ape file???" );
		return FALSE;
	}

	// make sure that there are not too many bones
	if ( GetNumMeshBones() > MAX_TOTAL_BONE_COUNT ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		sString1.Format( "There are %d bones in the ape file, but %d is the max allowed", GetNumMeshBones(), MAX_TOTAL_BONE_COUNT );
		rErrorLog.WriteErrorHeader( sString1 );
		return FALSE;
	}	

	// setup our ptr array to keep track of CPtrArrays
	m_aPtrArrays.RemoveAll();

	// Allocate Kong Mesh
	if ( m_pKongMesh )
	{
		delete m_pKongMesh;
	}
	m_pKongMesh = new KongMesh_t;
	if ( !m_pKongMesh )
	{
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate memory for kong mesh" );
		return FALSE;
	}

	u32 nSegCount, nMatCount, nMatPropertiesCount, nTriCount, nVertCount;

	// Calculate required tri count
	nTriCount = 0;
	for ( i = 0; i < GetNumSegments(); i++ ) 
	{
		pApeSeg = GetSegmentInfo( i );
		if ( !pApeSeg )
		{
			FreeData();
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			sString1.Format( "Bad segment count in APE file.  %d segments indicated but NULL segment returned.", GetNumSegments() );
			rErrorLog.WriteErrorHeader( sString1 );
			return FALSE;
		}
		for ( j = 0; j < pApeSeg->nNumMaterials; j++ ) 
		{
			pApeMat = GetMaterialInfo( i, j );

#if _GENERATE_BACKFACES_FOR_2SIDED_MATS
			if ( pApeMat->aLayer[0].bTwoSided )
			{
				nTriCount += 2 * (pApeMat->nNumIndices / 3);
			}
			else
#endif // _GENERATE_BACKFACES_FOR_2SIDED_MATS
			{
				nTriCount += pApeMat->nNumIndices / 3;
			}
		}
	}

	// Calculate other counts
	nVertCount = nTriCount * 3;
	nMatPropertiesCount = GetNumMeshMaterials();
	if( IsAnyMaterialSkinned() ) 
	{
		nSegCount = GetNumSegments() * GetNumMeshBones() + 10;
		nMatCount = GetNumSegments() * GetNumMeshBones() * GetNumMeshMaterials() + 10;
	}
	else
	{
		nSegCount = 1;
		nMatCount = GetNumMeshMaterials();
	}

	if ( !m_pKongMesh->AllocateArrays( nSegCount, nMatCount, nMatPropertiesCount, nTriCount, nVertCount ) )
	{
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate memory for kong" );
		return FALSE;
	}

	///////////////////////////////////////
	// COPY ALL TRIS INTO OUR KONG TRI LIST
	///////////////////////////////////////
	pauSegMatTriCount = (u16 *)malloc( sizeof( u16) * GetNumSegments() * nMatCount );
	if ( !pauSegMatTriCount )
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate memory for segment material count array" );
		goto _EXITWITHERROR;
	}

	// Clear current triangle counts
	memset( pauSegMatTriCount, 0, sizeof( u16) * GetNumSegments() * nMatCount );

	pTri = m_pKongMesh->paTriangles;
	for ( i=0; i < GetNumSegments(); i++ ) 
	{
		pApeSeg = GetSegmentInfo( i );
		pApeVert = GetVertData( i, 0 );

		if ( !pApeSeg || !pApeVert )
		{
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Unable to retrieve segment or vert data for Kong." );
			goto _EXITWITHERROR;
		}

		for ( j=0; j < pApeSeg->nNumMaterials; j++ ) 
		{
			pApeMat = GetMaterialInfo( i, j );						
			pApeIndex = GetVertIndex( i, j, 0 );

			if ( !pApeMat || !pApeIndex )
			{
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
				rErrorLog.WriteErrorLine( "Unable to retrieve material or index data for Kong segment." );
				goto _EXITWITHERROR;
			}

			nIndex = 0;
			pauSegMatTriCount[(i * nMatCount) + j] = 0;

			nNumTris = pApeMat->nNumIndices / 3;
			for ( k=0; k < nNumTris; k++ ) 
			{
				// add a tri
				FASSERT( nKongVertCounter + 3 <= m_pKongMesh->nVertsAllocated );
				FASSERT( nKongTriCounter < nTriCount );

				m_pKongMesh->paVertices[nKongVertCounter + 0].Set( &pApeVert[ pApeIndex[ nIndex++ ].nVertIndex ] );
				m_pKongMesh->paVertices[nKongVertCounter + 1].Set( &pApeVert[ pApeIndex[ nIndex++ ].nVertIndex ] );
				m_pKongMesh->paVertices[nKongVertCounter + 2].Set( &pApeVert[ pApeIndex[ nIndex++ ].nVertIndex ] );
				pTri->apKongVerts[0] = &m_pKongMesh->paVertices[nKongVertCounter + 0];
				pTri->apKongVerts[1] = &m_pKongMesh->paVertices[nKongVertCounter + 1];
				pTri->apKongVerts[2] = &m_pKongMesh->paVertices[nKongVertCounter + 2];

				if ( IsZeroAreaTri( pTri ) )
				{
					nZeroArea++;
					continue;
				}

				m_pKongMesh->nVertCount += 3;
				nKongVertCounter += 3;
				nKongTriCounter++;

				if ( pApeSeg->bSkinned ) 
				{
					// create a bone map for this tri
					MapTriBoneInfluences( *pTri );
				}
				pTri->pUser = pApeMat;
				utils_ComputeFaceNormal( pTri->FaceUnitNorm,
								   pTri->apKongVerts[0]->Pos, pTri->apKongVerts[1]->Pos, pTri->apKongVerts[2]->Pos,
								   pTri->apKongVerts[0]->Norm, pTri->apKongVerts[1]->Norm, pTri->apKongVerts[2]->Norm );
				utils_ComputeTriBound( pTri->BoundSphere, 
								 pTri->apKongVerts[0]->Pos, pTri->apKongVerts[1]->Pos, pTri->apKongVerts[2]->Pos );
				

				pLastTri = pTri;
				pTri++;
				pauSegMatTriCount[(i * nMatCount) + j]++;
				m_pKongMesh->nTriangleCount++;
			}
		}

		// Increment the verts
		nCurrentKongVertIdx += pApeSeg->nNumVerts;
	}

	if ( nZeroArea )
	{
		DEVPRINTF( "WARNING:  %d Zero Area Triangles detected while loading %s!\n", nZeroArea, m_sLastFileLoaded );
		if ( pDlg )
		{
			pDlg->InfoString( "    WARNING:  Loading %s...", m_sLastFileLoaded );
			pDlg->InfoString( "              %d near zero area triangles detected while loading!", nZeroArea );
		}
	}

	////////////////////////////////////////////////////////////////
	// CREATE KONG SEGMENTS & MATERIALS AND ASSIGN KONG TRIS TO THEM
	////////////////////////////////////////////////////////////////
	// see if we are skinned
	m_pKongMesh->nMatPropertiesCount = 0;
	if( !IsAnyMaterialSkinned() ) 
	{
		m_bSkinned = FALSE;
		//////////////////////////////////////////////////////////
		// NO MATERIAL IS SKINNED, THROW EVERYTHING INTO 1 SEGMENT
		//////////////////////////////////////////////////////////

		m_pKongMesh->nSegmentCount = 1;

		pSeg = m_pKongMesh->paSegs;
		pSeg->nNumBonesUsed = 0;
		pTri = m_pKongMesh->paTriangles;

		// walk the ape segments
		for ( i = 0; i < GetNumSegments(); i++ ) 
		{
			// walk each ape segment's materials
			for ( j = 0; j < GetNumSegmentMaterials( i ); j++ ) 
			{
				KongMatProp.Set( GetMaterialInfo( i, j ) );
				pMat = m_pKongMesh->GetIdenticalMaterial( pSeg, &KongMatProp );

				if ( !pMat )
				{
					// create a material
					if ( m_pKongMesh->nMaterialCount == m_pKongMesh->nMaterialsAllocated ) {
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
						rErrorLog.WriteErrorLine( "Exceeded material count in mesh while dividing triangles" );
						goto _EXITWITHERROR;
					}
					pMat = &m_pKongMesh->paMaterials[m_pKongMesh->nMaterialCount++];
					pMat->pProperties = m_pKongMesh->GetIdenticalProperties( &KongMatProp );
					if ( !pMat->pProperties )
					{
						// Didn't find a matching material property, so create a new one
						FASSERT( m_pKongMesh->nMatPropertiesCount < m_pKongMesh->nMatPropertiesAllocated );
						pMat->pProperties = &m_pKongMesh->paMatProperties[m_pKongMesh->nMatPropertiesCount];
						m_pKongMesh->paMatProperties[m_pKongMesh->nMatPropertiesCount] = KongMatProp;
						m_pKongMesh->nMatPropertiesCount++;
					}
					pMat->nMatFlags |= KONGMAT_FLAGS_ADDED_TO_SEGMENT;
					pMat->pKSeg = pSeg;
					pMat->nNumTris = 0;
					// record this material with pSeg
					m_pKongMesh->AddMatToSeg( pSeg, pMat );
				}

				u32 nNumTris = pauSegMatTriCount[(i * nMatCount) + j];
				for( k=0; k < nNumTris; k++ ) 
				{
					m_pKongMesh->AddTriToMat( pMat, pTri );
					pTri++;
				}
			}
		}
	} 
	else 
	{
		m_bSkinned = FALSE;
		/////////////////////////////////////////////////////////////////
		// WE ARE SKINNED (AT LEAST SOME OF THE SEGMENTS ARE),
		// WE NEED TO CREATE X SEGMENTS (PRESERVE APE SEGMENT BOUNDARIES)
		/////////////////////////////////////////////////////////////////
		u32 nTriStartIndex( 0 ), nNumTris( 0 ), nTriCount;

		// walk the ape segments
		for ( i = 0; i < GetNumSegments(); i++ ) 
		{
			nTriStartIndex += nNumTris;
			nNumTris = 0;
			for ( j = 0; j < GetNumSegmentMaterials( i ); j++ ) 
			{
				nNumTris += pauSegMatTriCount[(i * nMatCount) + j];//GetNumSegmentIndices( i ) / 3;
			}
			
			// SEE IF ANY MATERIAL IN THIS SEGMENT IS SKINNED
			// (IF ONE MATERIAL IS SKINNED, THEY ALL ARE)
			pTri = &m_pKongMesh->paTriangles[nTriStartIndex];
			if ( !pTri->nNumBonesUsed ) 
			{
				// THIS IS A NON SKINNED SEGMENT, THROW ALL MATERIALS INTO 1 SEGMENT
				
				// create a segment
				if ( m_pKongMesh->nSegmentCount == m_pKongMesh->nSegmentsAllocated ) 
				{
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
					rErrorLog.WriteErrorLine( "Exceeded segment count in mesh while dividing triangles" );
					goto _EXITWITHERROR;
				}
				pSeg = &m_pKongMesh->paSegs[m_pKongMesh->nSegmentCount++];

				// walk this ape segment's materials
				for ( j = 0; j < GetNumSegmentMaterials( i ); j++ ) 
				{
					KongMatProp.Set( GetMaterialInfo( i, j ) );
					pMat = m_pKongMesh->GetIdenticalMaterial( pSeg, &KongMatProp );

					if ( !pMat )
					{
						// create a material
						if ( m_pKongMesh->nMaterialCount == m_pKongMesh->nMaterialsAllocated ) {
							rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
							rErrorLog.WriteErrorLine( "Exceeded material count in mesh while dividing triangles" );
							goto _EXITWITHERROR;
						}
						pMat = &m_pKongMesh->paMaterials[m_pKongMesh->nMaterialCount++];
						pMat->pProperties = m_pKongMesh->GetIdenticalProperties( &KongMatProp );
						if ( !pMat->pProperties )
						{
							// Didn't find a matching material property, so create a new one
							FASSERT( m_pKongMesh->nMatPropertiesCount < m_pKongMesh->nMatPropertiesAllocated );
							pMat->pProperties = &m_pKongMesh->paMatProperties[m_pKongMesh->nMatPropertiesCount];
							m_pKongMesh->paMatProperties[m_pKongMesh->nMatPropertiesCount] = KongMatProp;
							m_pKongMesh->nMatPropertiesCount++;
						}
						pMat->nMatFlags |= KONGMAT_FLAGS_ADDED_TO_SEGMENT;
						pMat->pKSeg = pSeg;
						pMat->nNumTris = 0;
						// record this material with pSeg
						m_pKongMesh->AddMatToSeg( pSeg, pMat );
					}

					u32 nNumTris = pauSegMatTriCount[(i * nMatCount) + j];
					for ( k = 0; k < nNumTris; k++ ) 
					{
						m_pKongMesh->AddTriToMat( pMat, pTri );
						pTri++;
					}
				}
			} 
			else 
			{
				// HANDLE SKINNED APE SEGMENTS LIKE THIS:
				nTriCount = 0;
				while( nTriCount < nNumTris ) 
				{
					// create a segment
					if ( m_pKongMesh->nSegmentCount == m_pKongMesh->nSegmentsAllocated ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
						rErrorLog.WriteErrorLine( "Exceeded segment count in mesh while dividing triangles" );
						goto _EXITWITHERROR;
					}
					pSeg = &m_pKongMesh->paSegs[m_pKongMesh->nSegmentCount++];

					// find the best tri to base our segment on
					pTri = FindBestTriToBaseASegment( nTriStartIndex, nNumTris );
					if ( !pTri ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
						rErrorLog.WriteErrorLine( "Unable to establish a triangle to start a segment from" );
						goto _EXITWITHERROR;
					}

					// init the segment from pTri
					pSeg->nNumBonesUsed = pTri->nNumBonesUsed;
					for ( j = 0; j < FDATA_VW_COUNT_PER_VTX; j++ ) 
					{
						pSeg->anBoneID[j] = pTri->anBoneID[j];
					}
					pSeg->nNumMats = 0;

					// add new weights till no more fit in pSeg
#if 1
					while( BiggieSizeSegment( *pSeg, nTriStartIndex, nNumTris ) ) 
					{
					};
#endif
					if ( pSeg->nNumBonesUsed > 1 ) 
					{
						m_bSkinned = TRUE;
					}
#if 0
					DEVPRINTF( "ApeToKongFormat(): Segment %d has %d bones.\n", m_pKongMesh->nSegmentCount, pSeg->nNumBonesUsed );
#endif

					// run through all of the tris in this group looking for tris that can be added
					pApeMat = NULL;
					for ( j = 0; j < nNumTris; j++ ) 
					{
						pTri = &m_pKongMesh->paTriangles[ j + nTriStartIndex ];
						if ( CanTriBeAddedToSeg( *pTri, *pSeg ) ) 
						{
							// pTri can be added to pSeg
							if ( pApeMat != pTri->pUser ) 
							{
								pApeMat = (ApeMaterial_t *)pTri->pUser;// cache this material ptr

								KongMatProp.Set( pApeMat );
								pMat = m_pKongMesh->GetIdenticalMaterial( pSeg, &KongMatProp );

								if ( !pMat )
								{
									// create a material
									if ( m_pKongMesh->nMaterialCount == m_pKongMesh->nMaterialsAllocated ) {
										rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
										rErrorLog.WriteErrorLine( "Exceeded material count in mesh while dividing triangles" );
										goto _EXITWITHERROR;
									}
									pMat = &m_pKongMesh->paMaterials[m_pKongMesh->nMaterialCount++];
									pMat->pProperties = m_pKongMesh->GetIdenticalProperties( &KongMatProp );
									if ( !pMat->pProperties )
									{
										// Didn't find a matching material property, so create a new one
										FASSERT( m_pKongMesh->nMatPropertiesCount < m_pKongMesh->nMatPropertiesAllocated );
										pMat->pProperties = &m_pKongMesh->paMatProperties[m_pKongMesh->nMatPropertiesCount];
										m_pKongMesh->paMatProperties[m_pKongMesh->nMatPropertiesCount] = KongMatProp;
										m_pKongMesh->nMatPropertiesCount++;
									}
									pMat->nMatFlags |= KONGMAT_FLAGS_ADDED_TO_SEGMENT;
									pMat->pKSeg = pSeg;
									// record this material with pSeg
									m_pKongMesh->AddMatToSeg( pSeg, pMat );
								}
							}

							m_pKongMesh->AddTriToMat( pMat, pTri );
							pTri->nTriFlags |= KONGTRI_FLAG_TRI_ADDED_TO_SEGMENT;

							nTriCount++;
						}
					}
				}
			}
		}
	}

#if _GENERATE_BACKFACES_FOR_2SIDED_MATS
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Create backfaces
	//////////////////////////////////////////////////////////////////////////////////////////////////
	pTri = &m_pKongMesh->paTriangles[nKongTriCounter];
	for ( i = 0; i < m_pKongMesh->nSegmentCount; i++ ) 
	{
		pSeg = &m_pKongMesh->paSegs[i];
		
		// walk every Mat in the Mat list
		pMat = m_pKongMesh->GetFirstMaterial( pSeg );
		while ( pMat )
		{
			// Add a duplicate triangle for the backface, if there is one required
			if ( pMat->pProperties->bTwoSided )
			{
				u32 nTriAddedCount = 0, nTriCount = pMat->nNumTris;
				KongTri_t *pTriToDupe = m_pKongMesh->GetFirstTri( pMat );
				while ( nTriAddedCount++ < nTriCount )
				{
					FASSERT( pTriToDupe );
					FASSERT( nKongVertCounter + 3 <= m_pKongMesh->nVertsAllocated );
					FASSERT( nKongTriCounter < m_pKongMesh->nTrianglesAllocated );

					m_pKongMesh->paVertices[nKongVertCounter + 0] = *pTriToDupe->apKongVerts[1];
					m_pKongMesh->paVertices[nKongVertCounter + 0].nMeshVertIdx = nKongVertCounter + 0;
					m_pKongMesh->paVertices[nKongVertCounter + 0].Norm = -m_pKongMesh->paVertices[nKongVertCounter + 0].Norm;
					m_pKongMesh->paVertices[nKongVertCounter + 1] = *pTriToDupe->apKongVerts[0];
					m_pKongMesh->paVertices[nKongVertCounter + 1].nMeshVertIdx = nKongVertCounter + 1;
					m_pKongMesh->paVertices[nKongVertCounter + 1].Norm = -m_pKongMesh->paVertices[nKongVertCounter + 1].Norm;
					m_pKongMesh->paVertices[nKongVertCounter + 2] = *pTriToDupe->apKongVerts[2];
					m_pKongMesh->paVertices[nKongVertCounter + 2].nMeshVertIdx = nKongVertCounter + 2;
					m_pKongMesh->paVertices[nKongVertCounter + 2].Norm = -m_pKongMesh->paVertices[nKongVertCounter + 2].Norm;
					pTri->apKongVerts[0] = &m_pKongMesh->paVertices[nKongVertCounter + 0];
					pTri->apKongVerts[1] = &m_pKongMesh->paVertices[nKongVertCounter + 1];
					pTri->apKongVerts[2] = &m_pKongMesh->paVertices[nKongVertCounter + 2];
					m_pKongMesh->nVertCount += 3;
					nKongVertCounter += 3;
					nKongTriCounter++;

//					if ( pSeg->bSkinned ) 
//					{
						// create a bone map for this tri
//						MapTriBoneInfluences( *pTri );
//					}
					pTri->nTriFlags |= KONGTRI_FLAG_BACKFACE;
					pTri->pUser = pApeMat;
					pTri->FaceUnitNorm = -pTriToDupe->FaceUnitNorm;
					pTri->BoundSphere = pTriToDupe->BoundSphere;
					pTri->nNumBonesUsed = pTriToDupe->nNumBonesUsed;
					memcpy( pTri->anBoneID, pTriToDupe->anBoneID, sizeof(u8) * FDATA_VW_COUNT_PER_VTX );

					m_pKongMesh->nTriangleCount++;
					m_pKongMesh->AddTriToMat( pMat, pTri );

					pTri++;

					pTriToDupe = m_pKongMesh->GetNextTri( pTriToDupe );
				}
			}

			pMat = m_pKongMesh->GetNextMaterial( pMat );
		}
	}
#endif // _GENERATE_BACKFACES_FOR_2SIDED_MATS

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// ASSIGN AUTO-MATERIAL IDS AND SHADER VALUES (THIS MUST BE DONE AFTER MATERIAL COMPRESSION
	//////////////////////////////////////////////////////////////////////////////////////////////////
	nAutoID = 128;
	for ( i = 0; i < m_pKongMesh->nSegmentCount; i++ ) 
	{
		pSeg = &m_pKongMesh->paSegs[i];
		
		// walk every Mat in the Mat list
		pMat = m_pKongMesh->GetFirstMaterial( pSeg );
		while ( pMat )
		{
			// walk the used layers in this material
			for ( k = 0; k < pMat->pProperties->nLayersUsed; k++ ) 
			{
				if ( pMat->pProperties->aLayer[k].nID != 255 && pMat->pProperties->aLayer[k].nID > 127 ) 
				{
					// assign an auto id to this layer
					FASSERT( nAutoID < 255 );
					pMat->pProperties->aLayer[k].nID = (u8)( nAutoID++ );
				}				
			}

			pMat->SetShaderTypes();

			pMat = m_pKongMesh->GetNextMaterial( pMat );
		}
	}

	m_pKongMesh->bWorldFile = GetMeshInfo()->bWorldFile;

	free( pauSegMatTriCount );

	return TRUE;

_EXITWITHERROR:
	FreeData();
	if ( pauSegMatTriCount )
	{
		free( pauSegMatTriCount );
	}
	return FALSE;
}


CPtrArray *CApeToKongFormat::AllocatePtrArray() {
	CPtrArray *pPtrArray = new CPtrArray;
	if( !pPtrArray ) {
		return NULL;
	}
	pPtrArray->RemoveAll();
	m_aPtrArrays.Add( pPtrArray );	

	return pPtrArray;
}

void CApeToKongFormat::MapTriBoneInfluences( KongTri_t &Tri ) 
{
	u32 i, j, k, nVertBCount;
	u8 nBoneId;

	Tri.nNumBonesUsed = 0;
	for( i=0; i < 3; i++ ) 
	{
		// for each of the 3 verts
		nVertBCount = (u32)Tri.apKongVerts[i]->nNumWeights;
		for( j=0; j < nVertBCount; j++ ) 
		{
			// for each of this vert's weights
			nBoneId = (u8)Tri.apKongVerts[i]->nWeightBoneIdx[j];
			// see if this id is already in this list
			for( k=0; k < Tri.nNumBonesUsed; k++ ) 
			{
				if( Tri.anBoneID[k] == nBoneId ) 
				{
					break;
				}
			}
			if( k == Tri.nNumBonesUsed ) 
			{
				// id was not found, add it
				FASSERT( nBoneId < 255 );// 255 is a special bone id and can't be used
				Tri.anBoneID[ Tri.nNumBonesUsed ] = nBoneId;
				Tri.nNumBonesUsed++;
				FASSERT( Tri.nNumBonesUsed <= FDATA_VW_COUNT_PER_VTX );
			}
		}
	}
	// double check that all bone ids are in range
	for( i=0; i < Tri.nNumBonesUsed; i++ ) 
	{
		FASSERT( Tri.anBoneID[i] < CApeFileLoader::GetNumMeshBones() );
	}
}

// search the m_pKongTri array (within the specified range) returning the 
// tri with the most weights with bTriAdded==FALSE
KongTri_t *CApeToKongFormat::FindBestTriToBaseASegment( u32 nStartIndex, u32 nNumTris ) 
{
	u32 i, nNumWeightsUsed, nLastIndex = nStartIndex + nNumTris;
	KongTri_t *pTri, *pBest;

	// make sure that the last index doesn't extend too far
	if( nLastIndex > m_pKongMesh->nTriangleCount ) 
	{
		nLastIndex = m_pKongMesh->nTriangleCount;
	}
	pBest = NULL;
	nNumWeightsUsed=0;
	for( i=nStartIndex; i < nLastIndex; i++ ) 
	{
		pTri = &m_pKongMesh->paTriangles[i];
		if( !(pTri->nTriFlags & KONGTRI_FLAG_TRI_ADDED_TO_SEGMENT) ) 
		{
			if( pTri->nNumBonesUsed > nNumWeightsUsed ) 
			{
				nNumWeightsUsed = pTri->nNumBonesUsed;
				pBest = pTri;
				if( nNumWeightsUsed == FDATA_VW_COUNT_PER_VTX ) 
				{
					// the tri uses the max weights, we can use this tri
					return pBest;
				}
			}
		}
	}
	return pBest;
}

// checks to see if Tri can be added to Seg
BOOL CApeToKongFormat::CanTriBeAddedToSeg( KongTri_t &Tri, KongSeg_t &Seg ) {

	if( (Tri.nTriFlags & KONGTRI_FLAG_TRI_ADDED_TO_SEGMENT) || Tri.nNumBonesUsed > Seg.nNumBonesUsed ) 
	{
		// tri already added or this tri has too many bones to be added
		return FALSE;
	}

	// make sure that Tri doesn't have any bone ids that Seg doesn't have
	u32 i, j, nNumSegBones = Seg.nNumBonesUsed, nTriBones = Tri.nNumBonesUsed;
	u8 *panBoneId = Tri.anBoneID, *panSegBoneId = Seg.anBoneID;
	for( i=0; i < nTriBones; i++ ) 
	{
		for( j=0; j < nNumSegBones; j++ ) 
		{
			if( panSegBoneId[j] == *panBoneId ) 
			{
				break;
			}
		}
		if( j == nNumSegBones ) 
		{
			// Tri has a bone that is not contained in Seg
			return FALSE;
		}
		panBoneId++;
	}

	return TRUE;
}

// tries to find weights to fill up Seg.  Assumes that Seg
// has already been filled up with the weights from the best
// tri returned from FindBestTriToBaseASegment()
// returns TRUE if a weight was added to Seg, FALSE if not
BOOL CApeToKongFormat::BiggieSizeSegment( KongSeg_t &Seg, u32 nStartIndex, u32 nNumTris ) 
{
	u32 i, j, nLastIndex = nStartIndex + nNumTris;
	u32 nOrigBoneCount, nOrigTrisIn;
	u32 nCurTriCount, nBestTriCount;
	u8 nBestBoneId;
		
	if( Seg.nNumBonesUsed >= FDATA_VW_COUNT_PER_VTX ) 
	{
		// no room to add another weights
		return FALSE;
	}
	// make sure that the last index doesn't extend too far
	if( nLastIndex > m_pKongMesh->nTriangleCount ) 
	{
		nLastIndex = m_pKongMesh->nTriangleCount;
	}
	// get a count of how many tris would be in with the current weight set
	nOrigTrisIn = HowManyTrisFitIntoSeg( Seg, nStartIndex, nLastIndex );
	// copy the orig weight count
	nOrigBoneCount = Seg.nNumBonesUsed;
	// run through the tris and try to add bones
	nBestTriCount = nOrigTrisIn;
	
	u32 nTotalBoneCount = CApeFileLoader::GetNumMeshBones();
	for( i=0; i < nTotalBoneCount; i++ ) 
	{
		// see if i is already in this segment
		for( j=0; j < nOrigBoneCount; j++ ) 
		{
			if( i == Seg.anBoneID[j] ) 
			{
				break;
			}
		}
		if( j < nOrigBoneCount ) 
		{
			continue;
		}
		Seg.nNumBonesUsed = nOrigBoneCount+1;
		Seg.anBoneID[nOrigBoneCount] = (u8)i;

		nCurTriCount = HowManyTrisFitIntoSeg( Seg, nStartIndex, nLastIndex );
		if( nCurTriCount > nBestTriCount ) 
		{
			// this weight combo is our best yet, save the new bone id
			nBestTriCount = nCurTriCount;
			nBestBoneId = (u8)i;
		}
		// remove the bone that we just added
		Seg.nNumBonesUsed = nOrigBoneCount;
		Seg.anBoneID[nOrigBoneCount] = 0;
	}

	// see if we should add the new bone to Seg for good
	if( nBestTriCount > nOrigTrisIn ) 
	{
		Seg.nNumBonesUsed = nOrigBoneCount+1;
		Seg.anBoneID[nOrigBoneCount] = nBestBoneId;
		return TRUE;
	}
	return FALSE;
}

u32 CApeToKongFormat::HowManyTrisFitIntoSeg( KongSeg_t &Seg, u32 nStartIndex, u32 nLastIndex ) {
	KongTri_t *pTri;

	u32 nTriCount = 0;
	for( u32 i=nStartIndex; i < nLastIndex; i++ ) {
		pTri = &m_pKongMesh->paTriangles[i];
		if( CanTriBeAddedToSeg( *pTri, Seg ) ) {
			nTriCount++;
		}
	}
	return nTriCount;
}

// reset all kong verts edge info
void CApeToKongFormat::ResetEdgeInfo() 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	u32 i;
	KongTri_t *pTri = m_pKongMesh->paTriangles;
	for( i=0; i < m_pKongMesh->nTriangleCount; i++ ) {
		pTri->nNumEdgeTris = 0;
		pTri->paEdgeTris[0] = 
			pTri->paEdgeTris[1] = 
			pTri->paEdgeTris[2] = NULL;

		pTri++;
	}
}

/////////////////////////////////////////////////////////////
// BUILD EDGE CONNECTIVITY INFORMATION
// ONLY COMPUTE EDGE TRIS PER KONG MATERIAL, PER KONG SEGMENT
/////////////////////////////////////////////////////////////
void CApeToKongFormat::BuildEdgeDataPerMaterial() {
	if ( !m_pKongMesh )
	{
		return;
	}

	ResetEdgeInfo();

#if 0	
	u32 i, j;
	KongSeg_t *pSeg;
	KongMat_t *pMat, **ppMatData;

	// for every segment
	for( i=0; i < m_pKongMesh->nNumSegs; i++ ) { 
		pSeg = (KongSeg_t *)(*m_pKongMesh->apSegs)[i];

		// for every material
		ppMatData = (KongMat_t **)pSeg->apMats->GetData();
		for( j=0; j < pSeg->nNumMats; j++ ) {
			pMat = ppMatData[j];
						
			ComputeEdgeInfo( *pMat->apTris, pMat->nNumTris, 0 );						
		}
	}
#endif
}

void CApeToKongFormat::ComputeEdgeInfo( CPtrArray &rKTris, u32 uNumTris, u32 uStartingIndex ) {
	u32 k, nTri, nOuterIterations, nInnerIterations;
	KongTri_t **ppTriData, *pTri, *pTri2;

	// for each tri
	ppTriData = (KongTri_t **)rKTris.GetData();
	nInnerIterations = uStartingIndex + uNumTris;
	nOuterIterations = nInnerIterations - 1;
	
	for( k=uStartingIndex; k < nOuterIterations; k++ ) {
		pTri = ppTriData[k];
				
		// if pTri doesn't have 3 known edges
		if( pTri->nNumEdgeTris < 3 ) {
			// walk the rest of the tri list 
			for( nTri=k+1; nTri < nInnerIterations; nTri++ ) {
				pTri2 = ppTriData[nTri];
								
				// if pTri2 doesn't already have 3 known edges
				if( pTri2->nNumEdgeTris < 3 ) {
					
					if( utils_DoTheseTrisShareAnEdge( *pTri, *pTri2 ) ) {
						// create a link
						pTri->paEdgeTris[ pTri->nNumEdgeTris++ ] = pTri2;
						pTri2->paEdgeTris[ pTri2->nNumEdgeTris++ ] = pTri;
						if( pTri->nNumEdgeTris >= 3 ) {
							// no more edges to find
							break;
						}
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////
// BUILD EDGE CONNECTIVITY INFORMATION ACROSS THE ENTIRE MESH
/////////////////////////////////////////////////////////////

typedef struct _TriEdge_s _TriEdge_t;
struct _TriEdge_s {
	KongTri_t *pTri;
	_TriEdge_t *apFellowEdges[2];// the other 2 points that make up this tri
	FLink_t BucketLink;

	FLinkRoot_t *pBucket;
};

#define _BUCKETS_X						16
#define _BUCKETS_Y						16
#define _BUCKETS_Z						16
#define _BUCKET_COUNT					( _BUCKETS_X * _BUCKETS_Y * _BUCKETS_Z )
#define _CLOSE_ENOUGH_TO_SHARE_AN_EDGE	(0.001f * 0.001f)

void CApeToKongFormat::BuildEdgeDataAcrossEntireKong( CFVec3 &Min, CFVec3 &Max, CCompileDlg *pDlg/*=NULL*/ ) 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	static FLinkRoot_t _aaaBuckets[_BUCKETS_X][_BUCKETS_Y][_BUCKETS_Z];
	u32 nNumEdges, i, j, k, nX, nY, nZ, nPtsToAdd, nNotFullyConnected, nTicks, nMatTime, nSetupTime, nConnectingTime;
	_TriEdge_t *paEdges, *pEdge, *pNext, *pSearch, *pNextSearch, *pCurrent;
	KongTri_t *pKTri;
	CFVec3 Temp;
	f32 fX, fY, fZ, fOOX, fOOY, fOOZ;
	FLinkRoot_t *pBucket;

	// first build the material edge data 
	// (THIS CALL WILL START BY RESETING THE EDGE DATA)
	nTicks = GetTickCount();
	BuildEdgeDataPerMaterial();
	nMatTime = GetTickCount() - nTicks;

	// allocate enough edges to represent all of the tris in the mesh
	nNumEdges = m_pKongMesh->nTriangleCount * 3;
	paEdges = (_TriEdge_t *)fang_Malloc( nNumEdges * sizeof( _TriEdge_t ), 4 );
	if( !paEdges ) {
		// couldn't allocate memory, we are done
		FASSERT_NOW;
		return;
	}

	// init the link lists
	pBucket = &_aaaBuckets[0][0][0];
	for( i=0; i < _BUCKET_COUNT; i++ ) {
		flinklist_InitRoot( pBucket, (s32)FANG_OFFSETOF( _TriEdge_t, BucketLink ) );
		pBucket++;
	}

	// build a min/max box
	fX = (Max.x - Min.x) * (1.01f/_BUCKETS_X);
	fOOX = 1.0f/fX;
	fY = (Max.y - Min.y) * (1.01f/_BUCKETS_Y);
	fOOY = 1.0f/fY;
	fZ = (Max.z - Min.z) * (1.01f/_BUCKETS_Z);
	fOOZ = 1.0f/fZ;

	// walk the tri list, setting up our EdgePts and putting them in the proper buckets
	pKTri = m_pKongMesh->paTriangles;
	nNumEdges = 0;
	pEdge = &paEdges[0];
	nNotFullyConnected = 0;
	for( i=0; i < m_pKongMesh->nTriangleCount; i++ ) {

		if( pKTri->nNumEdgeTris < 3 ) {

			// create upto 3 edges for the tri
			nPtsToAdd = 0;
			pEdge = &paEdges[nNumEdges];
			for( j=0; j < 3; j++ ) {
				pCurrent = &pEdge[nPtsToAdd];
				
				// fill in the tri edge
				pCurrent->pTri = pKTri;

				// compute which bucket to put this pt into
				Temp = pKTri->apKongVerts[j]->Pos - Min;
				
				nX = (u32)( Temp.x * fOOX );
				FMATH_CLAMPMAX( nX, (_BUCKETS_X-1) );
				
				nY = (u32)( Temp.y * fOOY );
				FMATH_CLAMPMAX( nY, (_BUCKETS_Y-1) );
				
				nZ = (u32)( Temp.z * fOOZ );
				FMATH_CLAMPMAX( nZ, (_BUCKETS_Z-1) );

				pCurrent->pBucket = &_aaaBuckets[nX][nY][nZ];

				// make sure that this is a unique bucket from the other pts
				for( k=0; k < nPtsToAdd; k++ ) {
					if( pEdge[k].pBucket == pCurrent->pBucket ) {
						// don't add another bucket, we already have this one
						break;
					}
				}
				if( k == nPtsToAdd ) {
					flinklist_AddTail( pCurrent->pBucket, pCurrent );
					nPtsToAdd++;
				}
			}
			// assign pts to each other
			switch( nPtsToAdd ) {
			case 1:
				pEdge[0].apFellowEdges[0] = NULL;
				pEdge[0].apFellowEdges[1] = NULL;
				nNotFullyConnected++;
				break;
				
			case 2:
				pEdge[0].apFellowEdges[0] = &pEdge[1];
				pEdge[0].apFellowEdges[1] = NULL;
				
				pEdge[1].apFellowEdges[0] = &pEdge[0];
				pEdge[1].apFellowEdges[1] = NULL;
				nNotFullyConnected++;
				break;

			case 3:
				pEdge[0].apFellowEdges[0] = &pEdge[1];
				pEdge[0].apFellowEdges[1] = &pEdge[2];
				
				pEdge[1].apFellowEdges[0] = &pEdge[0];
				pEdge[1].apFellowEdges[1] = &pEdge[2];
				
				pEdge[2].apFellowEdges[0] = &pEdge[0];
				pEdge[2].apFellowEdges[1] = &pEdge[1];
				nNotFullyConnected++;
				break;
			}

			nNumEdges += nPtsToAdd;
		}
		pKTri++;
	}

	nSetupTime = GetTickCount() - (nTicks + nMatTime);
#if _SHOW_DETAILED_TIMINGS
	DEVPRINTF( "BuildEdgeDataAcrossEntireKong(): out of %d orig tris, %d remain after connecting each material's edges.\n", m_nNumKongTris, nNotFullyConnected );
#endif

	if ( pDlg )
	{
		pDlg->ResetSubOp( "Building Triangle Connectivity...", _BUCKET_COUNT );
		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	// walk each bucket
	pBucket = &_aaaBuckets[0][0][0];
	for( i=0; i < _BUCKET_COUNT; i++ ) {
		
		if ( pDlg )
		{
			pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );

			if ( pDlg->m_bAbortCompile )
			{
				return;
			}
		}

		// walk the edges in the bucket
		pEdge = (_TriEdge_t *)flinklist_GetHead( pBucket );
		while( pEdge ) {
			pNext = (_TriEdge_t *)flinklist_GetNext( pBucket, pEdge );

			if( pEdge->pTri->nNumEdgeTris < 3 ) {

				// find any other tris close to this one
				pSearch = pNext;
				while( pSearch ) {
					pNextSearch = (_TriEdge_t *)flinklist_GetNext( pBucket, pSearch );

					if( pSearch->pTri->nNumEdgeTris < 3 ) {
						// make sure that pEdge and pSearch don't already share an edge
						if( !utils_DoTheseTrisAlreadyShareAnEdge( *pEdge->pTri, *pSearch->pTri ) ) {

							// do the tri's bounding spheres intersect
							if( pEdge->pTri->BoundSphere.IsIntersecting( pSearch->pTri->BoundSphere ) ) {

								// do the tris share an edge
								if( utils_AreTheseTrisAdjacentToEachOther( *pEdge->pTri, *pSearch->pTri, _CLOSE_ENOUGH_TO_SHARE_AN_EDGE ) ) {
									// link these 2 tris
									pEdge->pTri->paEdgeTris[ pEdge->pTri->nNumEdgeTris++ ] = pSearch->pTri;
									pSearch->pTri->paEdgeTris[ pSearch->pTri->nNumEdgeTris++ ] = pEdge->pTri;

									if( pEdge->pTri->nNumEdgeTris == 3 ) {
										break;
									}
								}
							}
						}
					}

					// move to the next edge
					pSearch = pNextSearch;
				}
			}

			// see if pEdge should be removed
			if( pEdge->pTri->nNumEdgeTris == 3 ) {
				nNotFullyConnected--;

				// remove pEdge from its bucket
				flinklist_Remove( pEdge->pBucket, pEdge );

				// remove any of pEdge's fellow edges from their buckets & the main list
				if( pEdge->apFellowEdges[0] ) {
					flinklist_Remove( pEdge->apFellowEdges[0]->pBucket, pEdge->apFellowEdges[0] );
					
					if( pEdge->apFellowEdges[1] ) {
						flinklist_Remove( pEdge->apFellowEdges[1]->pBucket, pEdge->apFellowEdges[1] );
					}
				}				
			}

			// move to the next edge
			pEdge = pNext;
		}

		// advance to the next bucket
		pBucket++;
	}

	nConnectingTime = GetTickCount() - (nTicks + nSetupTime);
#if _SHOW_DETAILED_TIMINGS
	DEVPRINTF( "BuildEdgeDataAcrossEntireKong(): %d tris were not able to find all 3 edge connections.\n"
			   "\t%d ms to connect same material edges.\n"
			   "\t%d ms to setup the %d buckets.\n"
			   "\t%d ms to connect different material edges.\n",
			   nNotFullyConnected, nMatTime, nSetupTime, _BUCKET_COUNT, nConnectingTime );
#endif
	// free our memory
	fang_Free( paEdges );
}






