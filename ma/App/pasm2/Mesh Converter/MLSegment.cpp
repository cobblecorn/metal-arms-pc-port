//////////////////////////////////////////////////////////////////////////////////////
// MLSegment.cpp - Classes used to submit and manage mesh segments
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
// 11/19/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "fang.h"

#include "MLSegment.h"


// Must be at least this many triangles facing away from the directional light to 
// split the triangle container into two.
#define _DIR_LIGHT_SUBDIVISION_THRESHOLD	4


//
//
//
MLSegment::MLSegment( KongSeg_t *pKongSeg )
{
	m_FMeshSeg.BoundSphere_MS.m_Pos.x = 0.f;
	m_FMeshSeg.BoundSphere_MS.m_Pos.y = 0.f;
	m_FMeshSeg.BoundSphere_MS.m_Pos.z = 0.f;
	m_FMeshSeg.BoundSphere_MS.m_fRadius = 0.f;
	m_FMeshSeg.nBoneMtxCount = (u8)pKongSeg->nNumBonesUsed;

	for ( u32 i = 0; i < FDATA_VW_COUNT_PER_VTX; i++ )
	{
		m_FMeshSeg.anBoneMtxIndex[i] = pKongSeg->anBoneID[i];
	}

	m_nMtxWeightCount = FALSE;
	m_nMatrixIdx = -1;
	if ( m_FMeshSeg.nBoneMtxCount == 1 )
	{
		m_nMatrixIdx = pKongSeg->anBoneID[0];
	}

	m_nMtxWeightCount = m_FMeshSeg.nBoneMtxCount;

	m_pNext = NULL;

	m_nTriContainerCount = 0;
	m_pFirstTriContainer = NULL;
	m_pLastTriContainer = NULL;

	m_nTotalVerts = 0;

	m_nSegmentIdx = 0;

	m_nMaterialCount = 0;

	m_nPosType = -1;
	m_nPosFrac = -1;
}


//
//
//
MLSegment::~MLSegment( void )
{
	MLTriContainer *pTemp = m_pFirstTriContainer;
	MLTriContainer *pNext = NULL;
	while ( pTemp )
	{
		pNext = pTemp->m_pNext;
		delete pTemp;
		pTemp = pNext;
	}

	m_pFirstTriContainer = NULL;
	m_pLastTriContainer = NULL;
}


//
//
//
BOOL MLSegment::AddMaterialTris( KongMesh_t *pKongMesh, KongMat_t *pKongMat, MLMaterial *pMLMat, CFVec3 *pDirLightNorm )
{
	FASSERT( pKongMesh && pKongMat && pMLMat );

	u32 i, nStartTime = timeGetTime();

	for( i = 0; i < m_nMaterialCount; i++ )
	{
		if ( m_pMaterial[i] == pMLMat )
		{
			break;
		}
	}
	if ( i == m_nMaterialCount )
	{
		if ( m_nMaterialCount == MLSEGMENT_MAX_MATERIALS_PER_SEGMENT )
		{
			FASSERT_NOW;
			return FALSE;
		}

		m_pMaterial[m_nMaterialCount++] = pMLMat;
	}

	u32 nFacingToLightCount = 0, nFacingAwayFromLight = 0;

	// If this is volume geometry we are processing, then we want to setup up to two tri containers:  one
	// that is facing towards the directional light and one away from the directional light.  This will be
	// used to optimize the shadow rendering
	if ( pDirLightNorm )
	{
		// Count up the number of triangles facing the directional light
		KongTri_t *pTri = pKongMesh->GetFirstTri( pKongMat );
		while ( pTri )
		{
//			FASSERT( (u32)m_nVertCount + 3 <= m_nArraySize );

			if ( pTri->FaceUnitNorm.Dot( *pDirLightNorm ) < 0.f )
			{
				nFacingToLightCount++;
			}
			else
			{
				nFacingAwayFromLight++;
			}
			
			pTri = pKongMesh->GetNextTri( pTri );
		}

		if ( nFacingAwayFromLight < _DIR_LIGHT_SUBDIVISION_THRESHOLD )
		{
			// Did not exceed the threshold for subdivision, so put all the triangles into one group
			nFacingAwayFromLight = 0;
			nFacingToLightCount = pKongMat->nNumTris;
			pDirLightNorm = NULL;
		}
	}
	else
	{
		// Mark all triangles as facing the directional light so that there is no division of the polys
		nFacingToLightCount = pKongMat->nNumTris;
	}

	MLTriContainer *pAddTo = m_pFirstTriContainer;
	while ( pAddTo )
	{
		if ( pAddTo->m_pMaterial == pMLMat && pKongMat->pProperties->nLODIndex == pAddTo->m_nLODID )
		{
			if ( nFacingToLightCount && pAddTo->m_bFacingDirectionalLight )
			{
				if ( !pAddTo->AddVerts( pKongMesh, pKongMat, pMLMat, m_FMeshSeg.anBoneMtxIndex, m_FMeshSeg.nBoneMtxCount, nFacingToLightCount, pDirLightNorm ) )
				{
					DEVPRINTF( "MLSegment::AddVerts() - ERROR - Unable to add verts to vert container.\n" );
					MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
					return FALSE;
				}
				nFacingToLightCount = 0;
			}
			else if ( nFacingAwayFromLight && !pAddTo->m_bFacingDirectionalLight )
			{
				if ( !pAddTo->AddVerts( pKongMesh, pKongMat, pMLMat, m_FMeshSeg.anBoneMtxIndex, m_FMeshSeg.nBoneMtxCount, nFacingAwayFromLight, pDirLightNorm ) )
				{
					DEVPRINTF( "MLSegment::AddVerts() - ERROR - Unable to add verts to vert container.\n" );
					MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
					return FALSE;
				}
				nFacingAwayFromLight = 0;
			}

			MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;

			if ( !nFacingToLightCount && !nFacingAwayFromLight )
			{
				return TRUE;
			}
		}

		pAddTo = pAddTo->m_pNext;
	}

	FASSERT( nFacingToLightCount || nFacingAwayFromLight );

	if ( nFacingToLightCount )
	{
		// Allocate a new vertex container
		MLTriContainer *pVC = AllocateVertContainer();
		if ( !pVC )
		{
			MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
			return FALSE;
		}

		pVC->m_nLODID = pKongMat->pProperties->nLODIndex;
		pVC->m_nMatrixIdx = m_nMatrixIdx;
		pVC->m_nSegmentIdx = m_nSegmentIdx;
		pVC->m_bFacingDirectionalLight = TRUE;

		// Create the container data
		if ( !pVC->Create( pKongMesh, pKongMat, pMLMat, m_nMtxWeightCount, m_FMeshSeg.anBoneMtxIndex, m_FMeshSeg.nBoneMtxCount, nFacingToLightCount, pDirLightNorm ) )
		{
			DEVPRINTF( "MLSegment::AddVerts() - ERROR - Creation of Vert Container failed.\n" );
			MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
			return FALSE;
		}
	}

	if ( nFacingAwayFromLight )
	{
		// Allocate a new vertex container
		MLTriContainer *pVC = AllocateVertContainer();
		if ( !pVC )
		{
			MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
			return FALSE;
		}

		pVC->m_nLODID = pKongMat->pProperties->nLODIndex;
		pVC->m_nMatrixIdx = m_nMatrixIdx;
		pVC->m_nSegmentIdx = m_nSegmentIdx;
		pVC->m_bFacingDirectionalLight = FALSE;

		// Create the container data
		if ( !pVC->Create( pKongMesh, pKongMat, pMLMat, m_nMtxWeightCount, m_FMeshSeg.anBoneMtxIndex, m_FMeshSeg.nBoneMtxCount, nFacingAwayFromLight, pDirLightNorm ) )
		{
			DEVPRINTF( "MLSegment::AddVerts() - ERROR - Creation of Vert Container failed.\n" );
			MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
			return FALSE;
		}

	}

	MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
	return TRUE;
}


//
//
//
MLTriContainer* MLSegment::AllocateVertContainer( void )
{
	// Allocate a new vertex container
	MLTriContainer *pVC = new MLTriContainer;
	if ( !pVC )
	{
		return NULL;
	}

	// Link the container to the list
	if ( !m_pFirstTriContainer )
	{
		m_pFirstTriContainer = pVC;
		m_pLastTriContainer = pVC;
	}
	else
	{
		m_pLastTriContainer->m_pNext = pVC;
		m_pLastTriContainer = pVC;
		pVC->m_pNext = NULL;
	}

	// Increment the counter
	m_nTriContainerCount++;

	return pVC;
}


//
//
//
BOOL MLSegment::AddVerts( MLTriPacket *pPacket )
{
/*
	u32 nStartTime = timeGetTime();

	FASSERT( pPacket );
	u32 i;

	m_nTotalVerts += pPacket->nVertCount;

	for( i = 0; i < m_nMaterialCount; i++ )
	{
		if ( m_pMaterial[i] == pPacket->pMaterial )
		{
			break;
		}
	}
	if ( i == m_nMaterialCount )
	{
		if ( m_nMaterialCount == MLSEGMENT_MAX_MATERIALS_PER_SEGMENT )
		{
			FASSERT_NOW;
			return FALSE;
		}

		m_pMaterial[m_nMaterialCount++] = pPacket->pMaterial;
	}

	MLTriContainer *pAddTo = m_pFirstTriContainer;
	while ( pAddTo )
	{
		if (	pAddTo->m_pMaterial == pPacket->pMaterial
			&&	pAddTo->m_bInBoneSpace == pPacket->bInBoneSpace 
			&&	pAddTo->m_nCollisionMask == pPacket->nCollMask
			&& 	pAddTo->m_nCollisionType == pPacket->nCollType 
			&& 	pAddTo->m_bBumpMapped == (pPacket->pMaterial->m_nBumpMapTexture != -1) )
		{
			if ( !pAddTo->AddVerts( pPacket, m_FMeshSeg.anBoneMtxIndex, m_FMeshSeg.nBoneMtxCount ) )
			{
				DEVPRINTF( "MLSegment::AddVerts() - ERROR - Unable to add verts to vert container.\n" );
				MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
				return FALSE;
			}

			MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
			return TRUE;
		}

		pAddTo = pAddTo->m_pNext;
	}

	// Allocate a new vertex container
	MLTriContainer *pVC = new MLTriContainer;
	if ( !pVC )
	{
		MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
		return FALSE;
	}

	// Create the container data
	if ( !pVC->Create( pPacket, m_nMtxWeightCount, m_FMeshSeg.anBoneMtxIndex, m_FMeshSeg.nBoneMtxCount ) )
	{
		DEVPRINTF( "MLSegment::AddVerts() - ERROR - Creation of Vert Container failed.\n" );
		MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
		return FALSE;
	}

	pVC->m_nMatrixIdx = m_nMatrixIdx;
	pVC->m_nSegmentIdx = m_nSegmentIdx;

	// Link the container to the list
	if ( !m_pFirstTriContainer )
	{
		m_pFirstTriContainer = pVC;
		m_pLastTriContainer = pVC;
	}
	else
	{
		pVC->m_pNext = m_pFirstTriContainer;
		m_pFirstTriContainer = pVC;
	}

	// Increment the counter
	m_nTriContainerCount++;

	MLManager.m_Results.fVertAddTime += (f32)(timeGetTime() - nStartTime) / 1000.f;
*/
	return TRUE;
}


//
// Bounding sphere should be in model space
BOOL MLSegment::SetBoundingSphere( CFSphere *pBoundingSphere )
{
	FASSERT( pBoundingSphere );
	m_FMeshSeg.BoundSphere_MS = *pBoundingSphere;
	return TRUE;
}

//
//
void MLSegment::ColorVerts( u8 nRed, u8 nGreen, u8 nBlue, u8 nAlpha )
{
	MLTriContainer *pVC = m_pFirstTriContainer;
	while ( pVC )
	{
		pVC->ColorVerts( nRed, nGreen, nBlue, nAlpha );

		pVC = pVC->m_pNext;
	}
}

//
//
void MLSegment::ProcessVerts( FMeshBone_t *pBoneArray )
{
	MLTriContainer *pVC;

	if ( !pBoneArray )
	{
		pVC = m_pFirstTriContainer;
		while ( pVC )
		{
			pVC->CalculateAveragePosition();
			pVC = pVC->m_pNext;
		}
		return;
	}

	// If this segment is not skinned, we want to bring the verts into bone space.
	// This makes animation faster and quantizing more accurate:
	if ( m_nMtxWeightCount > 1 || m_nMatrixIdx == -1 )
	{
		pVC = m_pFirstTriContainer;
		while ( pVC )
		{
			pVC->CalculateAveragePosition();
			pVC = pVC->m_pNext;
		}
		return;
	}

	CFMtx43A mtxNormalTrans;
	mtxNormalTrans.Set( pBoneArray[ m_nMatrixIdx ].AtRestBoneToModelMtx );
	mtxNormalTrans.m_vRight.v3.Unitize();
	mtxNormalTrans.m_vUp.Unitize();
	mtxNormalTrans.m_vFront.Unitize();
	mtxNormalTrans.Transpose33();
	pVC = m_pFirstTriContainer;
	while ( pVC )
	{
		pVC->TransformVerts( &pBoneArray[ m_nMatrixIdx ].AtRestModelToBoneMtx, &mtxNormalTrans );
		pVC->m_nPartID = pBoneArray[ m_nMatrixIdx ].nPartID;
		pVC->CalculateAveragePosition();
		pVC = pVC->m_pNext;
	}
}


//
//
//
void MLSegment::Quantize( u32 nBias, u32 nUsedBoneCount )
{
	FASSERT( nBias >= 0 && nBias <= 10 );

	MLTriContainer *pVC = m_pFirstTriContainer;
	while ( pVC )
	{
		pVC->Quantize( nBias );

		// Verify that we have a valid matrix ID
		if ( pVC->m_nMatrixIdx == -1 )
		{
			// There are two instances where the matrix idx will be -1:  for skinned
			// meshes and for models with no bones.  In the instance of skinned meshes,
			// we will store the view matrix at the end of the bone list for use to transform
			// the skinned verts.  For models with no bones, we would store the matrix
			// in slot 0 since there are no other matrices stored.
			FASSERT( !nUsedBoneCount || m_nMtxWeightCount > 1 );

			// For both skinned and boneless segments, we want the matrix IDX to be at the end

			pVC->m_nMatrixIdx = nUsedBoneCount;
		}

		// If we don't already have quantization settings, use these
		if ( m_nPosType == -1 )
		{
			m_nPosType = pVC->m_nPosType;
			m_nPosFrac = pVC->m_nPosFrac;
		}
		else
		{
			// If we already have settings, we want to make sure that
			// all of the tri containers are using the same quantization (this
			// ensures that the verts don't separate):
			u32 nSign = 0;
			u32 nCurrPosFrac = m_nPosFrac;
			u32 nCurrPosBits = GetBits( m_nPosType );
			u32 nCurrNonFrac = nCurrPosBits - nCurrPosFrac;
			if ( m_nPosType == GX_S8 || m_nPosType == GX_S16 )
			{
				nSign = 1;
				nCurrNonFrac -= 1;
			}

			u32 nVCPosBits = GetBits( pVC->m_nPosType );
			u32 nVCNonFrac = nVCPosBits - pVC->m_nPosFrac;
			if ( pVC->m_nPosType == GX_S8 || pVC->m_nPosType == GX_S16 )
			{
				nSign = 1;
				nVCNonFrac -= 1;
			}

			if ( nCurrPosBits == 32 || nVCPosBits == 32 )
			{
				nCurrPosBits = 32;
				nCurrPosFrac = 0;
			}
			else
			{
				if ( nVCNonFrac > nCurrNonFrac )
					nCurrNonFrac = nVCNonFrac;

				if ( nVCPosBits > nCurrPosBits )
					nCurrPosBits = nVCPosBits;

				if ( nCurrPosBits - nSign - nCurrNonFrac < MIN_FRAC_COMPONENT )
				{
					// We need to raise the bits
					if ( nCurrPosBits == 8 )
						nCurrPosBits = 16;
					else
						nCurrPosBits = 32;
				}

				if ( nCurrPosBits == 32 )
					nCurrPosFrac = 0;
				else
					nCurrPosFrac = nCurrPosBits - nSign - nCurrNonFrac;
			}

			// Sanity Checks
			FASSERT( nCurrPosFrac == 0 || nCurrPosFrac >= MIN_FRAC_COMPONENT );
			FASSERT( nCurrPosBits >= nCurrPosFrac );
			FASSERT(	nCurrPosFrac == 0 
					|| nSign + nCurrNonFrac + nCurrPosFrac == 32
					||	nSign + nCurrNonFrac + nCurrPosFrac == 16
					||	nSign + nCurrNonFrac + nCurrPosFrac == 8 );

			m_nPosType = GetGCType( nCurrPosBits, nSign );
			m_nPosFrac = nCurrPosFrac;
		}

		pVC = pVC->m_pNext;
	}

	// Propogate the settings
	pVC = m_pFirstTriContainer;
	while ( pVC )
	{
		pVC->m_nPosType = (u16)m_nPosType;
		pVC->m_nPosFrac = (u16)m_nPosFrac;
		pVC = pVC->m_pNext;
	}
}


//
//
//
BOOL MLTriContainer::ProcessVert( KongVert_t *pVert, u8 *anSegmentBoneIdx, u16 nBoneCount, u8 nBaseUVCount, u8 nLightMapUVCount )
{
	u32 ii, iii;
#if _DEBUG
	// Verify that we have a valid normal
	f32 fNormalMag = pVert->Norm.Mag();
	if ( fNormalMag < 0.99f || fNormalMag > 1.01f )
	{
		FASSERT_NOW;
		return FALSE;
	}

	if ( m_bBumpMapped )
	{
		fNormalMag = pVert->Binorm.Mag();
		if ( fNormalMag < 0.99f || fNormalMag > 1.01f )
		{
			FASSERT_NOW;
			return FALSE;
		}
		fNormalMag = pVert->Tangent.Mag();
		if ( fNormalMag < 0.99f || fNormalMag > 1.01f )
		{
			FASSERT_NOW;
			return FALSE;
		}
	}

	for ( ii = 0; ii < (u32)(nBaseUVCount + nLightMapUVCount); ii++ )
	{
		FMATH_DEBUG_FCHECK( pVert->aUV[ii].x );
		FMATH_DEBUG_FCHECK( pVert->aUV[ii].y );
	}
#endif
	pVert->nBaseUVCount = nBaseUVCount;
	pVert->nLightMapUVCount = nLightMapUVCount;

	if ( pVert->nNumWeights > nBoneCount )
	{
		FASSERT_NOW;
		return FALSE;
	}

	// If this vert has only one bone, then set its weights to 0
	if ( nBoneCount == 1 )
	{
		pVert->nNumWeights = 0;
	}

	if ( nBoneCount > 1 
		&& (MLManager.GetPlatformType() == PASM_TARGET_PC 
		|| MLManager.GetPlatformType() == PASM_TARGET_XB) )
	{
		// Convert the matrix indices to be segment relative
		for ( ii = 0; ii < pVert->nNumWeights; ii++ )
		{
			for ( iii = 0; iii < nBoneCount; iii++ )
			{
				if ( pVert->nWeightBoneIdx[ii] == (u8)anSegmentBoneIdx[iii] )
				{
					pVert->nWeightBoneIdx[ii] = (u8)iii;
					break;
				}
			}

			if ( iii == nBoneCount )
			{
				// We didn't find the bone
				FASSERT_NOW;
				return FALSE;
			}
		}
	}

	return TRUE;
}


//
//
//
BOOL MLTriContainer::Create( KongMesh_t *pKongMesh, KongMat_t *pKongMat, MLMaterial *pMLMat, u32 nMtxWeightCount, u8 *anSegmentBoneIdx, u16 nBoneCount, u32 nRelevantTriCount, CFVec3 *pDirLightNorm )
{
	m_nBaseSTSets = (u8)FShaders_aShaderRegs[pKongMat->pProperties->nSurfaceShaderID].nUVCount;
	m_nLightMapSTSets = (u8)pKongMat->nLightMapCount;

	m_pMaterial = pMLMat;
	m_nCollisionMask = pKongMat->pProperties->nCollMask;
	if ( (pKongMat->pProperties->nCollID == 1 || pKongMat->pProperties->nCollID == 2) && pKongMat->pProperties->nReactType == 0 )
	{
		pKongMat->pProperties->nReactType = 1;
	}
	m_nCollisionType = (pKongMat->pProperties->nSurfaceType << 3) + pKongMat->pProperties->nReactType;
	m_bInBoneSpace = FALSE;

	m_nArraySize = nRelevantTriCount * 3;
	m_paKongVerts = new KongVert_t[m_nArraySize];
	if ( !m_paKongVerts )
	{
		return FALSE;
	}

	m_bBumpMapped = (pMLMat->m_nBumpMapTexture != -1);

	m_nVertCount = 0;
	KongTri_t *pTri = pKongMesh->GetFirstTri( pKongMat );
	while ( pTri )
	{
		if ( pDirLightNorm )
		{
			if ( m_bFacingDirectionalLight )
			{
				// Only add polys that are facing the directional light
				if ( pDirLightNorm->Dot( pTri->FaceUnitNorm ) >= 0.f )
				{
					pTri = pKongMesh->GetNextTri( pTri );
					continue;
				}
			}
			else
			{
				// Only add polys that are facing away from the directional light
				if ( pDirLightNorm->Dot( pTri->FaceUnitNorm ) < 0.f )
				{
					pTri = pKongMesh->GetNextTri( pTri );
					continue;
				}
			}
		}

		FASSERT( (u32)m_nVertCount + 3 <= m_nArraySize );

		fang_MemCopy( &m_paKongVerts[m_nVertCount], pTri->apKongVerts[0], sizeof( KongVert_t ) );
		if ( !ProcessVert( &m_paKongVerts[m_nVertCount++], anSegmentBoneIdx, nBoneCount, m_nBaseSTSets, m_nLightMapSTSets ) )
		{
			return FALSE;
		}
		fang_MemCopy( &m_paKongVerts[m_nVertCount], pTri->apKongVerts[1], sizeof( KongVert_t ) );
		if ( !ProcessVert( &m_paKongVerts[m_nVertCount++], anSegmentBoneIdx, nBoneCount, m_nBaseSTSets, m_nLightMapSTSets ) )
		{
			return FALSE;
		}
		fang_MemCopy( &m_paKongVerts[m_nVertCount], pTri->apKongVerts[2], sizeof( KongVert_t ) );
		if ( !ProcessVert( &m_paKongVerts[m_nVertCount++], anSegmentBoneIdx, nBoneCount, m_nBaseSTSets, m_nLightMapSTSets ) )
		{
			return FALSE;
		}
		
		pTri = pKongMesh->GetNextTri( pTri );
	}

	FASSERT( (u32)m_nVertCount == m_nArraySize );

	// Set the matrix weight count (>1 == skinned)
	m_nMtxWeightCount = (u16)nMtxWeightCount;

	return TRUE;
}


//
//
//
BOOL MLTriContainer::AddVerts( KongMesh_t *pKongMesh, KongMat_t *pKongMat, MLMaterial *pMLMat, u8 *anSegmentBoneIdx, u16 nBoneCount, u32 nRelevantTriCount, CFVec3 *pDirLightNorm )
{
	u32 nBaseSTSets = (u8)FShaders_aShaderRegs[pKongMat->pProperties->nSurfaceShaderID].nUVCount;
	u32 nLightMapSTSets = (u8)pKongMat->nLightMapCount;

	if ( nLightMapSTSets + nBaseSTSets != m_nBaseSTSets + m_nLightMapSTSets )
	{
		FASSERT_NOW;
		return FALSE;
	}

	u32 nVertCount = nRelevantTriCount * 3;
	if ( (u32)(m_nArraySize - m_nVertCount) < nVertCount )
	{
		m_nArraySize = m_nVertCount + nVertCount;

		// Allocate a new array of kong verts
		KongVert_t *pNewKongVerts = new KongVert_t[m_nArraySize];
		if ( !pNewKongVerts )
		{
			return FALSE;
		}

		// Copy in the existing kong verts and free the old array
		fang_MemCopy( pNewKongVerts, m_paKongVerts, m_nVertCount * sizeof( KongVert_t ) );
		FreeVerts();

		// Reassign the kong vert pointer to the new array
		m_paKongVerts = pNewKongVerts;
	}

	KongTri_t *pTri = pKongMesh->GetFirstTri( pKongMat );
	while ( pTri )
	{
		if ( pDirLightNorm )
		{
			if ( m_bFacingDirectionalLight )
			{
				// Only add polys that are facing the directional light
				if ( pDirLightNorm->Dot( pTri->FaceUnitNorm ) >= 0.f )
				{
					pTri = pKongMesh->GetNextTri( pTri );
					continue;
				}
			}
			else
			{
				// Only add polys that are facing away from the directional light
				if ( pDirLightNorm->Dot( pTri->FaceUnitNorm ) < 0.f )
				{
					pTri = pKongMesh->GetNextTri( pTri );
					continue;
				}
			}
		}

		FASSERT( (u32)m_nVertCount + 3 <= m_nArraySize );

		fang_MemCopy( &m_paKongVerts[m_nVertCount], pTri->apKongVerts[0], sizeof( KongVert_t ) );
		if ( !ProcessVert( &m_paKongVerts[m_nVertCount++], anSegmentBoneIdx, nBoneCount, m_nBaseSTSets, m_nLightMapSTSets ) )
		{
			return FALSE;
		}
		fang_MemCopy( &m_paKongVerts[m_nVertCount], pTri->apKongVerts[1], sizeof( KongVert_t ) );
		if ( !ProcessVert( &m_paKongVerts[m_nVertCount++], anSegmentBoneIdx, nBoneCount, m_nBaseSTSets, m_nLightMapSTSets ) )
		{
			return FALSE;
		}
		fang_MemCopy( &m_paKongVerts[m_nVertCount], pTri->apKongVerts[2], sizeof( KongVert_t ) );
		if ( !ProcessVert( &m_paKongVerts[m_nVertCount++], anSegmentBoneIdx, nBoneCount, m_nBaseSTSets, m_nLightMapSTSets ) )
		{
			return FALSE;
		}
		
		pTri = pKongMesh->GetNextTri( pTri );
	}

	FASSERT( (u32)m_nVertCount == m_nArraySize );

	return TRUE;
}


//
//
//
void MLTriContainer::Quantize( u32 nBias )
{
	u32 i;

	// record the color of the first vert in order to test for constant color
	m_ConstantColor.fRed = m_paKongVerts[0].Color.fRed;
	m_ConstantColor.fGreen = m_paKongVerts[0].Color.fGreen;
	m_ConstantColor.fBlue = m_paKongVerts[0].Color.fBlue;
	m_ConstantColor.fAlpha = m_paKongVerts[0].Color.fAlpha;
	m_bConstantColor = TRUE;

#if !OPTIMIZE_MEM_FOR_CONSTANT_COLOR
	m_bConstantColor = FALSE;
#endif

	if ( MLManager.m_bVertexRadiosityLit )
	{
		// If we're vertex radiosity lit, there cannot be constant color
		m_bConstantColor = FALSE;
	}

	// Allocate memory for the vertex abstractions
//	m_pGCVertAbstr = (GCVertAbstr_t *)malloc( m_nVertCount * sizeof( GCVertAbstr_t ) );
//	FASSERT( m_pVertAbstr );

	// Circulate through the verts getting the highest and lowest range
	f32 fHighest = -FMATH_MAX_FLOAT, fLowest = FMATH_MAX_FLOAT;
	for ( i = 0; i < m_nVertCount; i++ )
	{
		// Initially, the indices are all the same and linear
//		m_pVertAbstr[i].nPosIdx		 = (u16)i;
//		m_pVertAbstr[i].nNormIdx	 = (u16)i;
//		m_pVertAbstr[i].nDiffuseIdx  = (u16)i;
//		m_pVertAbstr[i].nSpecularIdx = (u16)i;
//		for ( u32 ii = 0; ii < FGCVB_MAX_ST_SETS; ii++ )
//		{
//			m_pVertAbstr[i].nSTIdx[ii]	= (u16)i;
//		}

		if ( m_paKongVerts[i].Pos.x > fHighest )
		{
			fHighest = m_paKongVerts[i].Pos.x;
		}
		if ( m_paKongVerts[i].Pos.y > fHighest )
		{
			fHighest = m_paKongVerts[i].Pos.y;
		}
		if ( m_paKongVerts[i].Pos.z > fHighest )
		{
			fHighest = m_paKongVerts[i].Pos.z;
		}

		if ( m_paKongVerts[i].Pos.x < fLowest )
		{
			fLowest = m_paKongVerts[i].Pos.x;
		}
		if ( m_paKongVerts[i].Pos.y < fLowest )
		{
			fLowest = m_paKongVerts[i].Pos.y;
		}
		if ( m_paKongVerts[i].Pos.z < fLowest )
		{
			fLowest = m_paKongVerts[i].Pos.z;
		}

		if ( m_bConstantColor && m_ConstantColor != m_paKongVerts[i].Color )
		{
			m_bConstantColor = FALSE;
		}
	}

	fHighest = (f32)fabs( fHighest );
	fLowest = (f32)fabs( fLowest );

	// Determine the largest non fractional component
	u32 nLargest = (u32)fHighest;
	if ( fHighest < fLowest )
	{
		nLargest = (u32)fLowest;
	}

	// Apply bias
	nLargest <<= nBias;

	s32 nNonFracBits = BitsToRepresent( nLargest );

	// We need to add a bit to the non-fractional component for the sign
	nNonFracBits++;

	// If the mesh is skinned, then we need to allocate 16 bit pos/normal pairs
	if ( m_nMtxWeightCount > 1 )
	{
		FASSERT( nNonFracBits <= 10 );
		m_nPosFrac = 6;
		m_nPosType = GX_S16;
	}

	// If the fractional component in an 8-bit representation would be greater 
	// than the minimum fractional component, we'll compress the verts to 8-bit.
	else if ( 8 - nNonFracBits >= MIN_FRAC_COMPONENT && !MLMesh_bVolumeMesh )
	{
		m_nPosFrac = (u16)(8 - nNonFracBits);
		m_nPosType = GX_S8;
	}

	// If the fractional component in an 16-bit representation would be greater 
	// than the minimum fractional component, we'll compress the verts to 16-bit.
	else if ( 16 - nNonFracBits >= MIN_FRAC_COMPONENT )
	{
		if ( MLMesh_bVolumeMesh )
		{
			m_nPosFrac = MIN_FRAC_COMPONENT;
		}
		else
		{
			m_nPosFrac = (u16)(16 - nNonFracBits);
		}
		m_nPosType = GX_S16;
	}
	// Otherwise, no compression will be performed
	else
	{
		m_nPosFrac = 0;
		m_nPosType = GX_F32;
	}
}

//
// This function determines a VB key which is used to
// determine whether a VB switch would be necessary
// between geometry groups using the same material
//
u32 MLTriContainer::CalculateVBKey( TargetPlatform_e nPlatform )
{
	if ( nPlatform == PASM_TARGET_PC )
	{
		u32 nWeightCount = 0;
		if ( m_nMtxWeightCount > 1 )
		{
			nWeightCount = 3;
		}

		m_nVBKey = fdx8vb_FindFormat( FALSE, 1, nWeightCount, 1, m_nBaseSTSets );
		FASSERT( m_nVBKey != 0xffffffff );
	}
	else if ( nPlatform == PASM_TARGET_XB )
	{
		FASSERT_NOW;
	}
	else if ( nPlatform == PASM_TARGET_GC )
	{
		// Determine Position component
		m_nVBKey = m_nPosType << 24;

		// Set a flag for skinning
		if ( m_nMtxWeightCount > 1 )
		{
			m_nVBKey |= 0x80000000;
		}

		// Determine the pos fractional component
		m_nVBKey |= (m_nPosFrac << 16);

		m_nVBKey |= (m_bBumpMapped * 0x40000000 );

		// Determine UV component
//		m_nVBKey |= m_nSTSets;
	}
#ifdef _MMI_TARGET_PS2
    else if ( nPlatform == PASM_TARGET_PS2 )               // KJ Begin
	{
		u32 nWeightCount = 0;
		if ( m_nMtxWeightCount > 1 )
		{
			nWeightCount = 3;
		}

//		fsh_GetVertexComponentCounts( m_pMaterial->m_nShaderID, &nNormalCount, &nTCCount, &nColorCount );

        m_nVBKey = fdx8vb_FindFormat( FALSE, 1, nWeightCount, 1, m_nBaseSTSets );
		FASSERT( m_nVBKey != 0xffffffff );
	}															// KJ End
#endif

	return m_nVBKey;
}



