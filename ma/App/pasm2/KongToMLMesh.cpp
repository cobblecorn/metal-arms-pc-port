//////////////////////////////////////////////////////////////////////////////////////
// KongToMLMesh.cpp - 
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
// 03/18/02 Starich     Created.
// 11/14/02 Lafleur     Genericized for all platforms from GC file
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "mmsystem.h"
#include "fang.h"
#include "KongToMLMesh.h"
#include "ErrorLog.h"
#include "MLMesh.h"
#include "utils.h"
#include "pasm.h"
#include "pasmdlg.h"
#include "compiledlg.h"
#include "lightmapgen.h"

#define _ERROR_HEADING				"KONG->MLMESH FILE COMPILER " 


//
//
//
CKongToMLMesh::CKongToMLMesh( BOOL bComputeEdgeData/*=TRUE*/ ) 
{
	m_nCompiledBytes = 0;
	m_pCompiledData = NULL;
	m_nNumBones = 0;
	m_paBones = NULL;
	m_nNumChildBones = 0;
	m_paChildIndices = NULL;
	m_nNumUsedBones = 0;
	m_nRootBoneIndex = 0;
	m_pMLMesh = NULL;
	m_nStrippingTime = 0;
	m_nMeshLibTime = 0;
	m_aOldToNewBoneIndex.RemoveAll();
	m_bComputeEdgeData = bComputeEdgeData;
}


//
//
//
CKongToMLMesh::~CKongToMLMesh() 
{
	FreeData();
}


//
//
//
BOOL CKongToMLMesh::Convert( CCompileDlg *pDlg, CApeToKongFormat &Kong, cchar *pszFilename, CAidFile *pAidFile, TargetPlatform_e nPlatform, 
							BOOL bGenerateStrips, BOOL bWorld, BOOL bVertexRadiosity, CFVec3 *pDirLight ) 
{
	u32 i, k;

	FreeData();

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	MLManager.InitForPlatform( pDlg, nPlatform, bGenerateStrips );

	if ( !Kong.m_bA2KConverted ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Kong format is not valid" );
		return FALSE;
	}

	// set our BEFORE stats
	m_nNumUVsBefore = Kong.m_pKongMesh->nTriangleCount * MAX_LAYERS_PER_MAT * 3;
	m_nNumXYZsBefore = 
		m_nNumRGBsBefore = 
		m_nNumTanBefore =
		m_nNumBiNBefore = 
		m_nNumNormsBefore = Kong.m_pKongMesh->nTriangleCount * 3;
	// zero our AFTER stats
	m_nNumUVsAfter = 
		m_nNumXYZsAfter = 
		m_nNumRGBsAfter = 
		m_nNumTanAfter = 
		m_nNumBiNAfter = 
		m_nNumNormsAfter = 0;
	
	// create our bone array
	if ( !CreateBones( &Kong ) ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Unable to create bone array for our mesh" );
		return FALSE;
	}

	MLManager.m_pDlg->WriteToLog( "Convert - Checking AID file for part settings.\n" );

	// check the aid file for user supplied settings
	if ( pAidFile && pAidFile->m_bDataValid ) 
	{
		u32 iPart, ii;
		char szError[256];
		if ( pAidFile->m_aPartStrings[0].bValueWasFound )
		{
			// Aid file is attempting to add bones to partlist 0, which is not allowed (that is the default partlist)
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
			sprintf( szError, "Attempting to add bones to partlist 0 is not allowed. (Partlist 0 is the default partlist)." );
			rErrorLog.WriteErrorLine( szError );
			return FALSE;
		}
		for ( iPart = 1; iPart < FDATA_MAX_MESH_PARTS; iPart++ )
		{
			// Check for an aid file specification for this mesh part
			if ( !pAidFile->m_aPartStrings[iPart].bValueWasFound )
			{
				continue;
			}
			s32 iIdx = 0;
			while ( iIdx != -1 && pAidFile->m_aPartStrings[iPart].sValue.GetLength() )
			{
				CString csTemp;

				// Extract the next bone name in the string
				pAidFile->m_aPartStrings[iPart].sValue.TrimLeft();
				pAidFile->m_aPartStrings[iPart].sValue.TrimRight();
				iIdx = pAidFile->m_aPartStrings[iPart].sValue.Find( "," );
				if ( iIdx == -1 )
				{
					// This is the last bone name in the list
					csTemp = pAidFile->m_aPartStrings[iPart].sValue;
				}
				else
				{
					// There is a comma after this bone name, so we need
					// to get the substring and chop it from the main string
					csTemp = pAidFile->m_aPartStrings[iPart].sValue.Left( iIdx );
					pAidFile->m_aPartStrings[iPart].sValue = pAidFile->m_aPartStrings[iPart].sValue.Right( pAidFile->m_aPartStrings[iPart].sValue.GetLength() - (iIdx + 1) );
				}

				// Find the bone that matches the specified bone name
				for ( ii = 0; ii < m_nNumBones; ii++ )
				{
					if ( csTemp.CompareNoCase( m_paBones[ii].szName ) == 0 )
					{
						if ( m_paBones[ii].nPartID != 0 )
						{
							// Aid file is attempting to add a bone to more than one part
							rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
							sprintf( szError, "Attempting to add bone %s to more than one mesh partlist in aid file.", csTemp.GetBuffer(0) );
							rErrorLog.WriteErrorLine( szError );
							return FALSE;
						}

						// Flag this bone as belonging to the specified part
						m_paBones[ii].nPartID = (u8)iPart;
						break;
					}
				}

				if ( ii == m_nNumBones )
				{
					// We could not find the bone name in the bone list
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
					sprintf( szError, "Unable to find bone name %s for partlist %d specified in aid file.", csTemp.GetBuffer(0), iPart );
					rErrorLog.WriteErrorLine( szError );
					return FALSE;
				}
			}
		}
	}

	// Remap our vertex weight bone indices
	if ( m_nNumBones && (m_aOldToNewBoneIndex.GetSize() > 0) ) 
	{
		// This mesh has bones, we need to fixup every vertex weight to index the proper bones.
		u32 nNumWeights, nBoneIndex;
		for ( i = 0; i < Kong.m_pKongMesh->nVertCount; i++ )
		{
			nNumWeights = (u32)Kong.m_pKongMesh->paVertices[i].nNumWeights;
			for( k = 0; k < nNumWeights; k++ ) 
			{
				nBoneIndex = (u32)Kong.m_pKongMesh->paVertices[i].nWeightBoneIdx[k];
				Kong.m_pKongMesh->paVertices[i].nWeightBoneIdx[k] = ( m_aOldToNewBoneIndex[ nBoneIndex ] );
			}
		}
	}

	m_nMeshLibTime = timeGetTime();
	// create our mesh class
	m_pMLMesh = MLManager.AllocateMLMesh( (char *)pszFilename, bWorld );
	if ( !m_pMLMesh ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Unable to allocate a ML mesh class" );
		MLManager.RemoveFromExportList( m_pMLMesh );
		return FALSE;
	}

	MLManager.m_pDlg->WriteToLog( "Convert - Checking AID file for LODs.\n" );

	// See if the aid file has LOD info
	if ( pAidFile && pAidFile->m_bDataValid ) 
	{
		char *pEnd;
		if ( pAidFile->m_aStrings[AID_FILE_STRINGS_LOD_SWITCHES].bValueWasFound )
		{
			CString sText = pAidFile->m_aStrings[AID_FILE_STRINGS_LOD_SWITCHES].sValue;

			u32 nLOD = 1;
			while ( sText.GetLength() )
			{
				f32 fDist = (f32)strtod( sText, &pEnd );
				if ( !m_pMLMesh->SetLODSwitchDistance( nLOD++, fDist ) )
				{
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
					rErrorLog.WriteErrorLine( "Error in LOD switch definition in AID file." );
					MLManager.RemoveFromExportList( m_pMLMesh );
					return FALSE;
				}
				if ( !pEnd[0] )
				{
					break;
				}
				sText = sText.Right( strlen( pEnd ) - 1 );
			}
		}

		if ( pAidFile->m_aNonStrings[AID_FILE_FLOATS_COLLISION_LOD].bValueWasFound )
		{
			if ( !m_pMLMesh->SetCollisionLOD( pAidFile->m_aNonStrings[AID_FILE_FLOATS_COLLISION_LOD].nValue ) )
			{
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
				rErrorLog.WriteErrorLine( "Unable to set Collision LOD for mesh." );
				MLManager.RemoveFromExportList( m_pMLMesh );
				return FALSE;
			}
		}

		if ( pAidFile->m_aNonStrings[AID_FILE_FLOATS_SHADOW_LOD_BIAS].bValueWasFound )
		{
			if ( !m_pMLMesh->SetShadowLODBias( pAidFile->m_aNonStrings[AID_FILE_FLOATS_SHADOW_LOD_BIAS].nValue ) )
			{
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
				rErrorLog.WriteErrorLine( "Unable to set Collision LOD for mesh." );
				MLManager.RemoveFromExportList( m_pMLMesh );
				return FALSE;
			}
		}
	}

	if ( bVertexRadiosity )
	{
		CString csRemapFilename = pszFilename;
		csRemapFilename.Replace( ".ape", "" );
		MLManager.m_bVertexRadiosityLit = TRUE;
		m_pMLMesh->GenerateVertexRemap( csRemapFilename, Kong.m_pKongMesh->nVertCount, &Kong.m_pKongMesh->KHeader.APELastWriteTime );
	}

	// Flag it as a Volume mesh, if applicable
	if ( Kong.m_bVolumeMesh ) 
	{
		m_pMLMesh->AddFlags( FMESH_FLAGS_VOLUME_MESH );
	}

	KongSeg_t *pKSeg;
	KongMat_t *pKongMat;
	MLSegment *pMLSeg;

	CFSphere Sphere;

	MLManager.m_pDlg->WriteToLog( "Convert - Adding segments and materials to MLMesh.\n" );

	// walk the segments
	for ( i = 0; i < Kong.m_pKongMesh->nSegmentCount; i++ ) 
	{
		// grab the kong segment
		pKSeg = &Kong.m_pKongMesh->paSegs[i];
		
		// allocate a ML segment
		pMLSeg = m_pMLMesh->AllocateMLSegment( pKSeg );
		if ( !pMLSeg ) 
		{
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Unable to allocate a ML segment class" );
			MLManager.RemoveFromExportList( m_pMLMesh );
			return FALSE;
		}

		// set the segment bounding sphere
		Kong.ComputeSegBoundingSphereFromKongTris( i, Sphere.m_Pos, Sphere.m_fRadius );
		pMLSeg->SetBoundingSphere( &Sphere );

		// walk the materials	
		pKongMat = Kong.m_pKongMesh->GetFirstMaterial( pKSeg );
		while ( pKongMat )
		{
			if ( pKongMat->nNumTris )
			{
				// Allocate an MLMat
				pKongMat->nMLMeshMatIndex = m_pMLMesh->AllocateMLMaterial( pKongMat );
				if ( pKongMat->nMLMeshMatIndex == 0xffff ) 
				{
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
					rErrorLog.WriteErrorLine( "Unable to allocate a ML material class" );
					MLManager.RemoveFromExportList( m_pMLMesh );
					return FALSE;
				}

				MLMaterial *pMLMat = m_pMLMesh->GetMaterialAtIdx( pKongMat->nMLMeshMatIndex );

				if ( !pMLSeg->AddMaterialTris( Kong.m_pKongMesh, pKongMat, pMLMat, pDirLight ) )
				{
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
					rErrorLog.WriteErrorLine( "Unable to add a material triangles to a segment" );
					MLManager.RemoveFromExportList( m_pMLMesh );
					return FALSE;	
				}
			}

			pKongMat = Kong.m_pKongMesh->GetNextMaterial( pKongMat );
		}
	}

	// add our bone array to the gc mesh
	if ( m_nNumBones ) 
	{
		if ( !m_pMLMesh->SetBoneArray( m_paBones, m_paChildIndices, m_nNumBones, m_nNumChildBones, m_nRootBoneIndex, m_nNumUsedBones ) ) 
		{
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Unable to add a bone array to a mesh" );
			MLManager.RemoveFromExportList( m_pMLMesh );
			return FALSE;	
		}
	}

	// Add the lights attached to this mesh
	if ( !Kong.m_bVolumeMesh ) 
	{
		ApeLight_t *pApeLight;
		u32 nLightCount = Kong.GetNumLights();
		for ( i = 0; i < nLightCount; i++ ) 
		{
			pApeLight = Kong.GetLightInfo( i );
			m_pMLMesh->AddLight( pApeLight );
		}
	}

	CFVec3 vMin, vMax;
	Kong.ComputeMeshBoundingBoxFromKongTris( vMin, vMax );
	m_pMLMesh->SetBoundingBox( &vMin, &vMax );

	Kong.ComputeMeshBoundingSphereFromKongTris( Sphere.m_Pos, Sphere.m_fRadius, Kong.m_bVolumeMesh );
	m_pMLMesh->SetBoundingSphere( &Sphere );

	// finally, generate our exportable data
	MLResults *pLibResults;
	pLibResults = MLManager.GenerateExportData();
	if ( !pLibResults->nExportDataSize ) 
	{
		MLManager.m_pDlg->WriteToLog( "Convert - Mesh lib error!!!\n" );

		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Kong.m_sLastFileLoaded );
		switch ( pLibResults->nErrorCode ) 
		{
			case ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER:
				rErrorLog.WriteErrorLine( "Mesh lib error: Vertex data exceeded lib buffer" );
				break;
			case ML_UNABLE_TO_RESOLVE_VERT_NORMAL:
				rErrorLog.WriteErrorLine( "Mesh lib error: Unable to resolve vert normal" );
				break;
			case ML_OUT_OF_MEMORY:
				rErrorLog.WriteErrorLine( "Mesh lib error: Unable to allocate memory" );
				break;
			case ML_LOD_COUNT_IN_AID_DOES_NOT_MATCH_MESH:
				rErrorLog.WriteErrorLine( "Mesh lib error: AID file LOD switch count must be at least # of mesh LODs." );
				break;
			default:
				rErrorLog.WriteErrorLine( "Mesh lib error: Unable to generate mesh export data" );
		}
		MLManager.RemoveFromExportList( m_pMLMesh );
		return FALSE;
	}

	// copy the results
	m_LibResults = *pLibResults;

	m_nCompiledBytes = m_LibResults.nExportDataSize;
	m_pCompiledData = m_LibResults.pExportData;
	
	// update our stats
	m_nNumUVsBefore = m_LibResults.nOrigSTCount;
	m_nNumXYZsBefore = m_LibResults.nOrigPosCount;
	m_nNumRGBsBefore = m_LibResults.nOrigColorCount;
	m_nNumNormsBefore = m_LibResults.nOrigNormalCount;
	m_nNumTanBefore = m_LibResults.nOrigTangCount;
	m_nNumBiNBefore = m_LibResults.nOrigBinormCount;

	m_nNumUVsAfter = m_LibResults.nST16Count;
	m_nNumXYZsAfter = m_LibResults.nPos16Count + m_LibResults.nPos32Count + m_LibResults.nPos8Count;
	m_nNumRGBsAfter = m_LibResults.nColorCount;
	m_nNumNormsAfter = m_LibResults.nNormal16Count;
	m_nNumTanAfter = m_LibResults.nTang16Count;
	m_nNumBiNAfter = m_LibResults.nBinorm16Count;

	m_nStripCount = m_LibResults.nStripCount;
	m_nStripTriCount = m_LibResults.nStripTriCount;
	m_nListTriCount = m_LibResults.nListTriCount;
	m_nStrippingTime = (u32)(m_LibResults.fStrippingTime * 1000);

	if ( m_paBones ) 
	{
		fang_Free( m_paBones );
		m_paBones = NULL;
	}

	if ( m_paChildIndices ) 
	{
		fang_Free( m_paChildIndices );
		m_paChildIndices = NULL;
	}

	if ( m_pMLMesh ) 
	{
		m_pMLMesh = NULL;
	}
	
	m_aOldToNewBoneIndex.RemoveAll();

	m_nMeshLibTime = timeGetTime() - m_nMeshLibTime;

	return TRUE;
}


//
//
//
u32 CKongToMLMesh::GetSizeOfConvertedFile( void ) 
{
	if ( !m_pCompiledData ) 
	{
		return 0;
	}
	
	return m_nCompiledBytes;
}


//
//
//
u32 CKongToMLMesh::GetCRC( u32 nCurrentCRC ) 
{
	if ( !m_pCompiledData ) 
	{
		return nCurrentCRC;
	}

	u32 nReturnCRC = fmath_Crc32( nCurrentCRC, (u8 *)m_pCompiledData, m_nCompiledBytes );

	return nReturnCRC;
}


//
//
//
BOOL CKongToMLMesh::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) 
{

	if ( !m_pCompiledData ) 
	{
		return FALSE;
	}

	BOOL bCloseFile = FALSE;
	if ( !pFileStream ) 
	{
		if ( !pszFilename ) 
		{
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if ( !pFileStream ) 
		{
			return FALSE;
		}
		bCloseFile = TRUE;
	}
	
	// write out our file
	fwrite( m_pCompiledData, m_nCompiledBytes, 1, pFileStream );
	
	// close our file
	if ( bCloseFile ) 
	{
		fclose( pFileStream );
	}

	return TRUE;
}


//
//
//
void CKongToMLMesh::FreeData( void ) 
{

	if ( m_pCompiledData ) 
	{
		free( m_pCompiledData );
		m_pCompiledData = NULL;
	}

	if ( m_paBones ) 
	{
		fang_Free( m_paBones );
		m_paBones = NULL;
	}

	if ( m_paChildIndices ) 
	{
		fang_Free( m_paChildIndices );
		m_paChildIndices = NULL;
	}

	m_aOldToNewBoneIndex.RemoveAll();

	m_nNumChildBones = 0;
	m_nNumBones = 0;
	m_nNumUsedBones = 0;
	m_nRootBoneIndex = 0;
	m_nCompiledBytes = 0;
}


//
//
//
BOOL CKongToMLMesh::CreateBones( CApeToKongFormat *pKong ) 
{
	u32 i, j, k, nStartIndex = 0, nBoneUsage, nVoidBoneCount = 0;
	FMeshBone_t *pBone, *pTempBone, SwapBone;
	ApeBone_t *pApeBone, *pParentBone;
	CFMtx43 Parent2Bone, Model2Bone;
	KongSeg_t *pKSeg, *pKSegWithSphere;
	BOOL bSkinned;
	f32 fBoneScale, fRadius;
	CPtrArray aBonePtrs;
	u32 nIndex;
	CFVec3 Center;
	CFVec3A Vec3A, Vec3AResult;
	
	// gather our counts
	m_nNumBones = pKong->GetNumMeshBones();
	m_nNumChildBones = pKong->GetNumBoneChildren();
	m_paBones = NULL;
	m_paChildIndices = NULL;
	m_nNumUsedBones = 0;
	m_nRootBoneIndex = 255;
	m_aOldToNewBoneIndex.RemoveAll();

	if ( !m_nNumBones ) 
	{
		return TRUE;
	}

	// allocate memory
	m_paBones = (FMeshBone_t *)fang_MallocAndZero( sizeof( FMeshBone_t ) * m_nNumBones, 16 );
	if ( !m_paBones ) 
	{
		return FALSE;
	}
	m_paChildIndices = (u8 *)fang_MallocAndZero( m_nNumChildBones, 0 );
	if ( !m_paBones ) 
	{
		return FALSE;
	}
	
	////////////////////////
	// create the bone array
	for ( i = 0; i < m_nNumBones; i++ ) 
	{
		// grab a mesh bone
		pBone = &m_paBones[i];
				
		// grab our ape bone
		pApeBone = pKong->GetMeshBone( i );

		pBone->nPartID = 0;

		// fill in the mesh bone
		strncpy( pBone->szName, pApeBone->szBoneName, FDATA_BONENAME_LEN );
		if ( pApeBone->nParentIndex < 0 ) 
		{
			pBone->Skeleton.nParentBoneIndex = 255;
			m_nRootBoneIndex = pApeBone->nBoneIndex;// since this bone has no parent it must be our root index

			Parent2Bone.Identity();			
		} 
		else 
		{
			pBone->Skeleton.nParentBoneIndex = (u8)pApeBone->nParentIndex;
			
			pParentBone = pKong->GetMeshBone( pBone->Skeleton.nParentBoneIndex );
			Parent2Bone = pApeBone->AtRestModelToBoneMtx * pParentBone->AtRestModelToBoneMtx.GetInverse();			
		}
		Model2Bone = pApeBone->AtRestModelToBoneMtx;

		pBone->Skeleton.nChildBoneCount = (u8)pApeBone->nNumChildren;
		pBone->Skeleton.nChildArrayStartIndex = (u8)nStartIndex;
		for ( j = 0; j < pBone->Skeleton.nChildBoneCount; j++ ) 
		{
			m_paChildIndices[nStartIndex + j] = pApeBone->auChildIndices[j];
		}
		nStartIndex += pBone->Skeleton.nChildBoneCount; 
		FASSERT( nStartIndex <= m_nNumChildBones );

		// fill in the bone mtxs
		pBone->AtRestParentToBoneMtx.Set( Parent2Bone );
		pBone->AtRestBoneToParentMtx.Set( Parent2Bone.GetInverse() );
		pBone->AtRestModelToBoneMtx.Set( Model2Bone );
		pBone->AtRestBoneToModelMtx.Set( Model2Bone.GetInverse() );
		
		if ( pApeBone->nFlags & APE_BONE_FLAG_NOT_USED ) 
		{
			pBone->nFlags = FMESH_BONEFLAG_VOIDBONE;
		}
	}

	/////////////////////////////////////////////
	// make sure that bone's get flagged properly

	// for every bone
	for ( i = 0; i < m_nNumBones; i++ ) 
	{
		// for every segment
		bSkinned = FALSE;
		nBoneUsage = 0;
		for ( j = 0; j < pKong->m_pKongMesh->nSegmentCount; j++ ) 
		{
			pKSeg = &pKong->m_pKongMesh->paSegs[j];
			if ( pKSeg->nNumBonesUsed ) 
			{
				// see if the i'th bone is used by this segment
				for( k = 0; k < pKSeg->nNumBonesUsed; k++ ) 
				{
					if ( pKSeg->anBoneID[k] == i ) 
					{
						// bone is used by the j'th segment
						nBoneUsage++;
						pKSegWithSphere = pKSeg;
						nIndex = j;
						if ( pKSeg->nNumBonesUsed > 1 ) 
						{
							bSkinned = TRUE;
						}
					}
				}
			}
		}

		pBone = &m_paBones[i];
		if ( pBone->nFlags & FMESH_BONEFLAG_VOIDBONE ) 
		{
			// this bone was flagged as not used, it should not appear in any segment!!!
			FASSERT( !nBoneUsage );
		}
		switch ( nBoneUsage ) 
		{
			case 0:
				// the i'th bone is not referenced by any segment
				pBone->nFlags |= FMESH_BONEFLAG_VOIDBONE;
				nVoidBoneCount++;
				break;
			case 1:
				if ( bSkinned ) 
				{
					// the i'th bone is used in a segment with more than 1 bone
					pBone->nFlags |= FMESH_BONEFLAG_SKINNEDBONE;
				} 
				else 
				{
					// the i'th bone is used in only 1 segment and that segment only references the i'th bone
					fBoneScale = pBone->AtRestModelToBoneMtx.m_vRight.Mag();
					pKong->ComputeSegBoundingSphereFromKongTris( nIndex, Center, fRadius );
					pBone->SegmentedBoundSphere_BS.m_fRadius = fRadius * fBoneScale;

					Vec3A.Set( Center );
					pBone->AtRestModelToBoneMtx.MulPoint( Vec3AResult, Vec3A );
					pBone->SegmentedBoundSphere_BS.m_Pos = Vec3AResult.v3;
				}
				break;
			default:
				// the i'th bone only appears in more than 1 segment
				pBone->nFlags |= FMESH_BONEFLAG_SKINNEDBONE;
				break;
		}
	}

	//////////////////////////////////////////////////////
	// finally, move all void bones to the end of the list
	if ( nVoidBoneCount ) 
	{
		if ( nVoidBoneCount == m_nNumBones ) 
		{
			// they are all void bones, mark it as such and move on
			m_nNumUsedBones = 0;
		} 
		else 
		{
			// create a bone ptr array
			aBonePtrs.SetSize( m_nNumBones );
			m_aOldToNewBoneIndex.SetSize( m_nNumBones );
			for ( i = 0; i < m_nNumBones; i++ ) 
			{
				aBonePtrs[i] = &m_paBones[i];
				m_aOldToNewBoneIndex[i] = i;
			}

			// sort the bone ptr array
			for ( i = 0; i < m_nNumBones; i++ ) 
			{
				pBone = (FMeshBone_t *)aBonePtrs[i];
				if ( pBone->nFlags & FMESH_BONEFLAG_VOIDBONE ) 
				{
					// find a slot at the end of the list for pBone
					for ( j = (m_nNumBones-1); j > i; j-- ) 
					{
						pTempBone = (FMeshBone_t *)aBonePtrs[j];
						if ( (pTempBone->nFlags & FMESH_BONEFLAG_VOIDBONE) == 0 ) 
						{
							// swap i and j
							aBonePtrs[i] = pTempBone;
							aBonePtrs[j] = pBone;

							m_aOldToNewBoneIndex[i] = j;
							m_aOldToNewBoneIndex[j] = i;
							break;
						}
					}
					if ( i == j ) 
					{
						// no need to continue
						break;
					}
				}
			}

			// fix up the mesh
			if ( m_nRootBoneIndex != 255 ) 
			{
				m_nRootBoneIndex = m_aOldToNewBoneIndex[m_nRootBoneIndex];
			}
			// fix up the segments
			for ( j = 0; j < pKong->m_pKongMesh->nSegmentCount; j++ ) 
			{
				pKSeg = pKSeg = &pKong->m_pKongMesh->paSegs[j];
				if ( pKSeg->nNumBonesUsed ) 
				{
					// see if the i'th bone is used by this segment
					for ( k = 0; k < pKSeg->nNumBonesUsed; k++ ) 
					{
						nIndex = pKSeg->anBoneID[k];
						if ( nIndex != 255 ) 
						{
							pKSeg->anBoneID[k] = m_aOldToNewBoneIndex[ nIndex ];
						}
					}
				}
			}
			// fix up the bone array
			for ( i = 0; i < m_nNumBones; i++ ) 
			{
				pBone = (FMeshBone_t *)aBonePtrs[i];
				
				nIndex = pBone->Skeleton.nParentBoneIndex;
				if ( nIndex != 255 ) 
				{
					pBone->Skeleton.nParentBoneIndex = m_aOldToNewBoneIndex[ nIndex ];
				}

				for ( j = 0; j < pBone->Skeleton.nChildBoneCount; j++ ) 
				{
					nIndex = pBone->Skeleton.nChildArrayStartIndex + j;

					m_paChildIndices[nIndex] = m_aOldToNewBoneIndex[ m_paChildIndices[nIndex] ];
				}
			}
			for ( i = 0; i < m_nNumBones; i++ ) 
			{
				if ( m_aOldToNewBoneIndex[i] > i ) 
				{
					// swap
					pBone = (FMeshBone_t *)aBonePtrs[i];
					pTempBone = (FMeshBone_t *)aBonePtrs[ m_aOldToNewBoneIndex[i] ];
					SwapBone = *pBone;
					*pBone = *pTempBone;
					*pTempBone = SwapBone;					
				}
			}

			m_nNumUsedBones = m_nNumBones - nVoidBoneCount;
		}
	}
	else
	{
		m_nNumUsedBones = m_nNumBones;
	}

//	if ( m_nNumBones ) 
//	{
//		DEVPRINTF( "KongToMLMesh(): Bones Used %d of %d total.\n", m_nNumUsedBones, m_nNumBones );
//	}
	return TRUE;
}


//
//
//
BOOL CKongToMLMesh::GenerateLMSTs( CCompileDlg *pDlg, KongMesh_t *pKongMesh, cchar *pszFilename ) 
{
	u32 j, nStartTime;
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

//	pDlg->InfoString( "    Creating lightmap STs for %s.", pszFilename );
	nStartTime = timeGetTime();

	// Set the parameters and build the lightmap
	CLightMapGen m_LightMapGen;
	m_LightMapGen.SetLocalDir( CPasmDlg::m_pStaticLocalDir->GetString() );
	m_LightMapGen.SetParam( 4, 0.5f, 0, "NULL", TRUE );
	m_LightMapGen.InitWithKongData( pKongMesh, pDlg );
	m_LightMapGen.Build( NULL, TRUE );

	if ( pDlg->m_bAbortCompile )
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + *pszFilename );
		rErrorLog.WriteErrorLine( "Compiling aborted by user." );
		return FALSE;
	}

	// Now add the lightmap data into the mesh data
	u32 nMtlVtxOffset = 0;
	CLightingSolution::LN_MaterialExport_t *pMtl;

	if ( !pKongMesh->paSegs )
	{
		return FALSE;
	}

	// Walk the materials	
	j = 0;
	KongMat_t *pKMat = pKongMesh->GetFirstMaterial( pKongMesh->paSegs );
	while ( pKMat )
	{
		if ( !LMG_StaticallyLightMaterial( pKMat ) )
		{
			pKMat = pKongMesh->GetNextMaterial( pKMat );
			continue;
		}

		pMtl = m_LightMapGen.ExportMaterial( 0, j );
		if ( pMtl && !(pKMat->pProperties->nFlags & APE_MAT_FLAGS_DO_NOT_LM) )
		{
			u32 nMtlLMIdx;
			u32 nLMVtxCount = 0;
			pKMat->nLightMapCount = 0;
			u32 nBaseSTSets = FShaders_aShaderRegs[pKMat->pProperties->nSurfaceShaderID].nUVCount;
			for ( nMtlLMIdx = 0; nMtlLMIdx < pMtl->nNumLightMaps; nMtlLMIdx++ )
			{
				// Copy temp lightmap texture name into kong material
				fclib_strcpy( pKMat->szLightMapTexname[pKMat->nLightMapCount], "black" );
				pKMat->anLightMapMotifID[pKMat->nLightMapCount] = pMtl->anMotifList[nMtlLMIdx];

				u32 nMtlVtxIdx = 0;
				while ( nMtlVtxIdx < pMtl->nVtx && pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].nLMIndex == pMtl->anLightMapIdx[nMtlLMIdx] )
				{
					u32 nTriIdx = (pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].nVtxIndex - nMtlVtxOffset) / 3;
					u32 nVertIdx = (pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].nVtxIndex - nMtlVtxOffset) % 3;
					
					FASSERT( nTriIdx < pKMat->nNumTris );
					KongTri_t *pTri = pKongMesh->GetTriAtIdx( pKMat, nTriIdx );
					if ( !pTri ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + *pszFilename );
						rErrorLog.WriteErrorLine( "Error matching triangles to lightmap ST's." );
						return FALSE;
					}

					FASSERT( pTri );

					// Add the vertex info
					pTri->apKongVerts[nVertIdx]->aUV[nBaseSTSets + pKMat->nLightMapCount].a[0] = pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].fU;
					pTri->apKongVerts[nVertIdx]->aUV[nBaseSTSets + pKMat->nLightMapCount].a[1] = pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].fV;

					nMtlVtxIdx++;
				}

				pKMat->nLightMapCount++;
			}
		}

		if ( pMtl )
		{
			m_LightMapGen.FreeMaterial( pMtl );
		}

		// Increment the material vertex offset so that future materials do not index past their end
		nMtlVtxOffset += (pKMat->nNumTris * 3);

		pKMat = pKongMesh->GetNextMaterial( pKMat );
		j++;
	}

	pKongMesh->KHeader.bHasLightmapsSTs = TRUE;

	return TRUE;
}
