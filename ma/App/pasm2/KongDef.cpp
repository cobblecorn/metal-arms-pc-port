//////////////////////////////////////////////////////////////////////////////////////
// KongDef.cpp - 
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
// 12/10/01 Lafleur     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fang.h"
#include "pasm.h"
#include "pasmdlg.h"
#include "kongdef.h"
#include "fshaders.h"
#include "utils.h"


//
//
//
BOOL KongMesh_t::AllocateArrays( u32 nSegCount, u32 nMatCount, u32 nMatPropCount, u32 nTriCount, u32 nVertexCount )
{
	u32 i;

	if ( nSegCount )
	{
		// Allocate memory for Kong Segments
		nSegmentsAllocated = nSegCount;
		paSegs = new KongSeg_t[nSegmentsAllocated];
		if ( !paSegs ) 
		{
			FreeMemory();
			return FALSE;
		}
		nSegmentCount = 0;
		bSegmentsLocallyAllocated = TRUE;
	}


	if ( nMatCount )
	{
		// Allocate memory for Kong Materials
		nMaterialsAllocated = nMatCount;
		paMaterials = new KongMat_t[nMaterialsAllocated];
		if ( !paMaterials ) 
		{
			FreeMemory();
			return FALSE;
		}
		for ( i = 0; i < nMaterialsAllocated ; i++ )
		{
			// Set the material indices
			paMaterials[i].nMatIdx = i;
		}
		nMaterialCount = 0;
		bMaterialsLocallyAllocated = TRUE;
	}

	if ( nMatPropCount )
	{
		// Allocate memory for Kong Material Properties
		nMatPropertiesAllocated = nMatPropCount;
		paMatProperties = new KongMatProperties_t[nMatPropertiesAllocated];
		if( !paMatProperties ) 
		{
			FreeMemory();
			return FALSE;
		}
		ZeroMemory( paMatProperties, sizeof( KongMatProperties_t ) * nMatPropertiesAllocated );
		nMatPropertiesCount = 0;
		bMatPropertiesLocallyAllocated = TRUE;
	}

	if ( nTriCount )
	{
		// Allocate memory for Kong triangles
		nTrianglesAllocated = nTriCount;
		paTriangles = new KongTri_t[nTrianglesAllocated];
		if ( !paTriangles ) 
		{
			FreeMemory();
			return FALSE;
		}
		ZeroMemory( paTriangles, sizeof( KongTri_t ) * nTrianglesAllocated );
		for ( i = 0; i < nTrianglesAllocated; i++ )
		{
			paTriangles[i].nTriIdx = i;
		}
		nTriangleCount = 0;
		bTrianglesLocallyAllocated = TRUE;
	}

	if ( nVertexCount )
	{
		// Allocate memory for Kong verts
		nVertsAllocated = nVertexCount;
		paVertices = new KongVert_t[nVertsAllocated];
		if ( !paVertices ) 
		{
			FreeMemory();
			return FALSE;
		}
		ZeroMemory( paVertices, sizeof( KongVert_t ) * nVertsAllocated );
		for ( i = 0; i < nVertsAllocated; i++ )
		{
			paVertices[i].nMeshVertIdx = i;
		}
		nVertCount = 0;
		bVertsLocallyAllocated = TRUE;
	}

	return TRUE;
}


//
//
//
BOOL KongMesh_t::FreeMemory( void )
{
	if ( paSegs && bSegmentsLocallyAllocated )
	{
		delete[] paSegs;
	}
	paSegs = NULL;
	nSegmentCount = 0;
	nSegmentsAllocated = 0;
	bSegmentsLocallyAllocated = FALSE;

	if ( paMaterials && bMaterialsLocallyAllocated )
	{
		delete[] paMaterials;
	}
	paMaterials = NULL;
	nMaterialCount = 0;
	nMaterialsAllocated = 0;
	bMaterialsLocallyAllocated = FALSE;

	if ( paMatProperties && bMatPropertiesLocallyAllocated )
	{
		delete[] paMatProperties;
	}
	paMatProperties = NULL;
	nMatPropertiesCount = 0;
	nMatPropertiesAllocated = 0;
	bMatPropertiesLocallyAllocated = FALSE;

	if ( paTriangles && bTrianglesLocallyAllocated )
	{
		delete[] paTriangles;
	}
	paTriangles = NULL;
	nTriangleCount = 0;
	nTrianglesAllocated = 0;
	bTrianglesLocallyAllocated = FALSE;

	if ( paVertices && bVertsLocallyAllocated )
	{
		delete[] paVertices;
	}
	paVertices = NULL;
	nVertCount = 0;
	nVertsAllocated = 0;
	bVertsLocallyAllocated = FALSE;

	return TRUE;
}


//
//
//
BOOL ReadKongHeader( cchar *pszKongFilePath, KongFileHeader_t *pKongHeader )
{
	FASSERT( pszKongFilePath && pKongHeader );

	HANDLE hFile = CreateFile( pszKongFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	u32 nBytesRead;
	ReadFile( hFile, pKongHeader, sizeof(KongFileHeader_t), (LPDWORD)&nBytesRead, NULL );
	CloseHandle( hFile );

	if ( nBytesRead != sizeof( KongFileHeader_t ) )
	{
		// KNG file is corrupt
		return FALSE;
	}

	if ( pKongHeader->Header.nSignature != FVERSION_FILE_SIGNATURE || pKongHeader->Header.nVersion != KONG_FILE_VERSION )
	{
		// KNG file is corrupt
		return FALSE;
	}

	return TRUE;
}


//
//
//
u32 KongMesh_t::CreateFileAndSaveKong( cchar *pszFilename )
{
	u32 nTotalBytesWritten;
	CString csName;
	FASSERT( pszFilename );

	// Create/Overwrite the file for this kong
	csName = CPasmDlg::m_csPASMTempDirectory + pszFilename + ".kng";
	csName.MakeLower();
	HANDLE hFile = CreateFile( csName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	nTotalBytesWritten = SaveKong( hFile );

	CloseHandle( hFile );

	return nTotalBytesWritten;
}


//
//
//
u32 KongMesh_t::OpenFileAndLoadKong( cchar *pszFilePath )
{
	u32 nTotalBytesRead;
	FASSERT( pszFilePath );

	HANDLE hFile = CreateFile( pszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	nTotalBytesRead = LoadKong( hFile );

	CloseHandle( hFile );

	return nTotalBytesRead;
}


//
//
//
u32 KongMesh_t::SaveKong( HANDLE hFile )
{
	u32 i, nBytesWritten, ADR_Current;
	CString csName;

	FASSERT( hFile != INVALID_HANDLE_VALUE );

	ADR_Current = sizeof( KongMesh_t );

	// Determine file offsets
	u32 ADR_Segs			= ADR_Current;
	u32 MEM_Segs			= sizeof(KongSeg_t) * nSegmentCount;
	ADR_Current				= ADR_Segs + MEM_Segs;

	u32 ADR_Materials		= ADR_Current;
	u32 MEM_Materials		= sizeof(KongMat_t) * nMaterialCount;
	ADR_Current				= ADR_Materials + MEM_Materials;

	u32 ADR_MatProperties	= ADR_Current;
	u32 MEM_MatProperties	= sizeof(KongMatProperties_t) * nMatPropertiesCount;
	ADR_Current				= ADR_MatProperties + MEM_MatProperties;

	u32 ADR_Triangles		= ADR_Current;
	u32 MEM_Triangles		= sizeof(KongTri_t) * nTriangleCount;
	ADR_Current				= ADR_Triangles + MEM_Triangles;

	u32 nMainBufferSize = ADR_Current;

	u32 ADR_Vertices		= ADR_Current;
	u32 MEM_Vertices		= sizeof(KongVert_t) * nVertCount;
	ADR_Current				= ADR_Vertices + MEM_Vertices;

	void *pMainBuffer = malloc( nMainBufferSize + MEM_Vertices );
	if ( !pMainBuffer )
	{
		return FALSE;
	}
	memset( pMainBuffer, 0, nMainBufferSize + MEM_Vertices );

	u8 *pCurrentMain = (u8 *)pMainBuffer;
	u8 *pCurrentVert = pCurrentMain + nMainBufferSize;

	// Fill the buffers
	KongMesh_t *pSaveMesh = (KongMesh_t *)pCurrentMain;
	memcpy( pSaveMesh, this, sizeof( KongMesh_t ) );
	pSaveMesh->paSegs = (KongSeg_t *)ADR_Segs;
	pSaveMesh->paMaterials = (KongMat_t *)ADR_Materials;
	pSaveMesh->paMatProperties = (KongMatProperties_t *)ADR_MatProperties;
	pSaveMesh->paTriangles = (KongTri_t *)ADR_Triangles;
	pSaveMesh->paVertices = (KongVert_t *)ADR_Vertices;
	pSaveMesh->nSegmentsAllocated = pSaveMesh->nSegmentCount;
	pSaveMesh->bSegmentsLocallyAllocated = TRUE;
	pSaveMesh->nMaterialsAllocated = pSaveMesh->nMaterialCount;
	pSaveMesh->bMaterialsLocallyAllocated = TRUE;
	pSaveMesh->nMatPropertiesAllocated = pSaveMesh->nMatPropertiesCount;
	pSaveMesh->bMatPropertiesLocallyAllocated = TRUE;
	pSaveMesh->nTrianglesAllocated = pSaveMesh->nTriangleCount;
	pSaveMesh->bTrianglesLocallyAllocated = TRUE;
	pSaveMesh->nVertsAllocated = pSaveMesh->nVertCount;
	pSaveMesh->bVertsLocallyAllocated = TRUE;
	pCurrentMain += sizeof(KongMesh_t);

	FASSERT( (u32)pCurrentMain == (u32)pMainBuffer + (u32)ADR_Segs );
	for ( i = 0; i < nSegmentCount; i++ )
	{
		KongSeg_t *pSeg = (KongSeg_t *)pCurrentMain;
		memcpy( pSeg, &paSegs[i], sizeof( KongSeg_t) );
		pCurrentMain += sizeof( KongSeg_t );
	}
	
	FASSERT( (u32)pCurrentMain == (u32)pMainBuffer + (u32)ADR_Materials );
	for ( i = 0; i < nMaterialCount; i++ )
	{
		KongMat_t *pMat = (KongMat_t *)pCurrentMain;
		memcpy( pMat, &paMaterials[i], sizeof( KongMat_t) );
		pMat->pKSeg = (KongSeg_t *)((u32)pMat->pKSeg - (u32)paSegs);
		pMat->pProperties = (KongMatProperties_t *)((u32)pMat->pProperties - (u32)paMatProperties);
		pCurrentMain += sizeof( KongMat_t );
	}

	FASSERT( (u32)pCurrentMain == (u32)pMainBuffer + (u32)ADR_MatProperties );
	for ( i = 0; i < nMatPropertiesCount; i++ )
	{
		KongMatProperties_t *pMatProp = (KongMatProperties_t *)pCurrentMain;
		memcpy( pMatProp, &paMatProperties[i], sizeof( KongMatProperties_t) );
		pCurrentMain += sizeof( KongMatProperties_t );
	}

	FASSERT( (u32)pCurrentMain == (u32)pMainBuffer + (u32)ADR_Triangles );
	for ( i = 0; i < nTriangleCount; i++ )
	{
		KongTri_t *pTri = (KongTri_t *)pCurrentMain;
		memcpy( pTri, &paTriangles[i], sizeof( KongTri_t) );
		pTri->paEdgeTris[0] = (KongTri_t *)((u32)pTri->paEdgeTris[0] - (u32)paTriangles);
		pTri->paEdgeTris[1] = (KongTri_t *)((u32)pTri->paEdgeTris[1] - (u32)paTriangles);
		pTri->paEdgeTris[2] = (KongTri_t *)((u32)pTri->paEdgeTris[2] - (u32)paTriangles);
		pTri->nUser[0] = 0;
		pTri->nUser[1] = 0;
		pTri->pUser = NULL;
		pCurrentMain += sizeof( KongTri_t );
	}


	if ( bWorldFile )
	{
		// If this is a world file mesh, we are going to compress the verts
		pSaveMesh->nVertCount = 0;
	}

	u32 nTriIdx = 0, ii, iii;
	FASSERT( (u32)pCurrentMain == (u32)pMainBuffer + (u32)ADR_Vertices );
	KongMat_t *pDestMats = (KongMat_t *)((u32)pMainBuffer + ADR_Materials);
	KongTri_t *pDestTris = (KongTri_t *)((u32)pMainBuffer + ADR_Triangles);
	for ( i = 0; i < nMaterialCount; i++ )
	{
		if ( pDestMats[i].nFirstTriIdx == KONG_INVALID_TRI_INDEX )
		{
			continue;
		}

		KongTri_t *pTri = &pDestTris[pDestMats[i].nFirstTriIdx];
		while ( pTri )
		{
			nTriIdx++;
			for ( ii = 0; ii < 3; ii++ )
			{
				KongVert_t *pVert = pTri->apKongVerts[ii];

				if ( bWorldFile )
				{
					memcpy( pCurrentVert, &pSaveMesh->nVertCount, sizeof( u32 ) );
				}
				else
				{
					memcpy( pCurrentVert, &pVert->nMeshVertIdx, sizeof( u32 ) );
				}
				pCurrentVert += sizeof( u32 );

				memcpy( pCurrentVert, &pVert->Pos, sizeof( CFVec3 ) );
				pCurrentVert += sizeof( CFVec3 );
				memcpy( pCurrentVert, &pVert->Norm, sizeof( CFVec3 ) );
				pCurrentVert += sizeof( CFVec3 );
				if ( paMaterials[i].pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_BUMP][0] != 0 )
				{
					memcpy( pCurrentVert, &pVert->Binorm, sizeof( CFVec3 ) );
					pCurrentVert += sizeof( CFVec3 );
					memcpy( pCurrentVert, &pVert->Tangent, sizeof( CFVec3 ) );
					pCurrentVert += sizeof( CFVec3 );
				}
				memcpy( pCurrentVert++, &pVert->nBaseUVCount, sizeof( u8 ) );
				memcpy( pCurrentVert++, &pVert->nLightMapUVCount, sizeof( u8 ) );
				memcpy( pCurrentVert++, &pVert->nNumWeights, sizeof( u8 ) );
				memcpy( pCurrentVert++, &pVert->__PAD, sizeof( u8 ) );

				memcpy( pCurrentVert, &pVert->Color, sizeof( CFColorRGBA ) );
				pCurrentVert += sizeof( CFColorRGBA );

				// Only add in the uvs that are used
				for ( iii = 0; iii < FShaders_aShaderRegs[paMaterials[i].pProperties->nSurfaceShaderID].nUVCount + paMaterials[i].nLightMapCount; iii++ )
				{
					memcpy( pCurrentVert, &pVert->aUV[iii], sizeof( CFVec2 ) );
					pCurrentVert += sizeof( CFVec2 );
				}

				if ( paMaterials[i].pKSeg->nNumBonesUsed > 1 )
				{
					memcpy( pCurrentVert, pVert->nWeightBoneIdx, sizeof( s8 ) * MAX_WEIGHTS_PER_VERT );
					pCurrentVert += sizeof( s8 ) * MAX_WEIGHTS_PER_VERT;
					memcpy( pCurrentVert, pVert->fWeight, sizeof( f32 ) * MAX_WEIGHTS_PER_VERT );
					pCurrentVert += sizeof( f32 ) * MAX_WEIGHTS_PER_VERT;
				}

				if ( bWorldFile )
				{
					pTri->apKongVerts[ii] = (KongVert_t *)(pSaveMesh->nVertCount);
					pSaveMesh->nVertCount++;
				}
				else
				{
					pTri->apKongVerts[ii] = (KongVert_t *)(pVert->nMeshVertIdx);
				}
			}

			if ( pTri->nNextTriIdx == KONG_INVALID_TRI_INDEX )
			{
				pTri = NULL;
			}
			else
			{
				pTri = &pDestTris[pTri->nNextTriIdx];
			}
		}
	}

	pSaveMesh->nVertsAllocated = pSaveMesh->nVertCount;

	FASSERT( nTriIdx == nTriangleCount );

	u32 nBytesToWrite = (u32)pCurrentVert - (u32)pMainBuffer;
	pSaveMesh->KHeader.nFileDataSize = nBytesToWrite - sizeof(KongFileHeader_t);
    WriteFile( hFile, pMainBuffer, nBytesToWrite, (LPDWORD)&nBytesWritten, NULL );

	free( pMainBuffer );

	if ( nBytesWritten != nBytesToWrite )
	{
		return 0;
	}

	return nBytesWritten;
}


//
//
//
u32 KongMesh_t::LoadKong( HANDLE hFile )
{
	u32 i, ii, iii, nBytesRead, nTotalBytesRead = 0;

	FASSERT( hFile != INVALID_HANDLE_VALUE );

	#define __FIXUP_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pNewMesh + (u32)pointer)

	ReadFile( hFile, &KHeader, sizeof(KongFileHeader_t), (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != sizeof( KongFileHeader_t ) )
	{
		return 0;
	}
	nTotalBytesRead += nBytesRead;

	// Make sure we have a valid signature
	if ( KHeader.Header.nSignature != FVERSION_FILE_SIGNATURE )
	{
		return 0;
	}

	// Allocate temp memory to read in the Kong
	void *pFile = malloc( KHeader.nFileDataSize );
	if ( !pFile )
	{
		return 0;
	}
	u8 *pCurrent = (u8 *)pFile;

	ReadFile( hFile, pFile, KHeader.nFileDataSize, (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != KHeader.nFileDataSize )
	{
		return 0;
	}
	nTotalBytesRead += nBytesRead;

	// Copy over the member vars
	memcpy( (void *)((u32)this + sizeof(KongFileHeader_t)), pFile, sizeof(KongMesh_t) - sizeof(KongFileHeader_t) );
	pCurrent += sizeof(KongMesh_t) - sizeof(KongFileHeader_t);

	// Allocate memory to hold the data
	AllocateArrays( nSegmentCount, nMaterialCount, nMatPropertiesCount, nTriangleCount, nVertCount );
	nSegmentCount = nSegmentsAllocated;
	nMaterialCount = nMaterialsAllocated;
	nMatPropertiesCount = nMatPropertiesAllocated;
	nTriangleCount = nTrianglesAllocated;
	nVertCount = nVertsAllocated;

	// Read in the segments
	memcpy( paSegs, pCurrent, sizeof(KongSeg_t) * nSegmentCount );
	pCurrent += sizeof(KongSeg_t) * nSegmentCount;

	// Read in the materials
	memcpy( paMaterials, pCurrent, sizeof(KongMat_t) * nMaterialCount );
	pCurrent += sizeof(KongMat_t) * nMaterialCount;

	// Fixup the materials
	for ( i = 0; i < nMaterialCount; i++ )
	{
		paMaterials[i].pKSeg = (KongSeg_t *)((u32)paSegs + (u32)paMaterials[i].pKSeg);
		paMaterials[i].pProperties = (KongMatProperties_t *)((u32)paMatProperties + (u32)paMaterials[i].pProperties);
	}

	// Read in the material properties
	memcpy( paMatProperties, pCurrent, sizeof(KongMatProperties_t) * nMatPropertiesCount );
	pCurrent += sizeof(KongMatProperties_t) * nMatPropertiesCount;

	// Read in the triangles
	memcpy( paTriangles, pCurrent, sizeof(KongTri_t) * nTriangleCount );
	pCurrent += sizeof(KongTri_t) * nTriangleCount;

	// Fixup the triangles
	for ( i = 0; i < nTriangleCount; i++ )
	{
		paTriangles[i].paEdgeTris[0] = (KongTri_t *)((u32)paTriangles + (u32)paTriangles[i].paEdgeTris[0]);
		paTriangles[i].paEdgeTris[1] = (KongTri_t *)((u32)paTriangles + (u32)paTriangles[i].paEdgeTris[1]);
		paTriangles[i].paEdgeTris[2] = (KongTri_t *)((u32)paTriangles + (u32)paTriangles[i].paEdgeTris[2]);
		paTriangles[i].apKongVerts[0] = &paVertices[ (u32)paTriangles[i].apKongVerts[0] ];
		paTriangles[i].apKongVerts[1] = &paVertices[ (u32)paTriangles[i].apKongVerts[1] ];
		paTriangles[i].apKongVerts[2] = &paVertices[ (u32)paTriangles[i].apKongVerts[2] ];
	}

	// Read in the vertices
	u32 nTriIdx = 0;//, nVertexCount = 0;
	for ( i = 0; i < nMaterialCount; i++ )
	{
		KongTri_t *pTri = GetFirstTri( &paMaterials[i] );
		while ( pTri )
		{
			nTriIdx++;

			for ( ii = 0; ii < 3; ii++ )
			{
				u32 nVertIdx = *((u32 *)pCurrent);

				FASSERT( pTri->apKongVerts[ii] == &paVertices[nVertIdx] );

				memcpy( &paVertices[nVertIdx].nMeshVertIdx, pCurrent, sizeof(u32) );
				pCurrent += sizeof(u32);
				memcpy( &paVertices[nVertIdx].Pos, pCurrent, sizeof(CFVec3) );
				pCurrent += sizeof(CFVec3);
				memcpy( &paVertices[nVertIdx].Norm, pCurrent, sizeof(CFVec3) );
				pCurrent += sizeof(CFVec3);
				if ( paMaterials[i].pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_BUMP][0] != 0 )
				{
					memcpy( &paVertices[nVertIdx].Binorm, pCurrent, sizeof(CFVec3) );
					pCurrent += sizeof(CFVec3);
					memcpy( &paVertices[nVertIdx].Tangent, pCurrent, sizeof(CFVec3) );
					pCurrent += sizeof(CFVec3);
				}
				memcpy( &paVertices[nVertIdx].nBaseUVCount, pCurrent, sizeof(u8) );
				pCurrent += sizeof(u8);
				memcpy( &paVertices[nVertIdx].nLightMapUVCount, pCurrent, sizeof(u8) );
				pCurrent += sizeof(u8);
				memcpy( &paVertices[nVertIdx].nNumWeights, pCurrent, sizeof(u8) );
				pCurrent += sizeof(u8);
				memcpy( &paVertices[nVertIdx].__PAD, pCurrent, sizeof(u8) );
				pCurrent += sizeof(u8);

				memcpy( &paVertices[nVertIdx].Color, pCurrent, sizeof(CFColorRGBA) );
				pCurrent += sizeof(CFColorRGBA);

				// We only saved the uvs that are used
				for ( iii = 0; iii < FShaders_aShaderRegs[paMaterials[i].pProperties->nSurfaceShaderID].nUVCount + paMaterials[i].nLightMapCount; iii++ )
				{
					memcpy( &paVertices[nVertIdx].aUV[iii], pCurrent, sizeof(CFVec2) );
					pCurrent += sizeof(CFVec2);
				}

				if ( paMaterials[i].pKSeg->nNumBonesUsed > 1 )
				{
					memcpy( &paVertices[nVertIdx].nWeightBoneIdx, pCurrent, sizeof(s8) * MAX_WEIGHTS_PER_VERT );
					pCurrent += sizeof(s8) * MAX_WEIGHTS_PER_VERT;
					memcpy( &paVertices[nVertIdx].fWeight, pCurrent, sizeof(f32) * MAX_WEIGHTS_PER_VERT );
					pCurrent += sizeof(f32) * MAX_WEIGHTS_PER_VERT;
				}

				FASSERT( pCurrent <= (u8 *)pFile + KHeader.nFileDataSize );
			}

			pTri = GetNextTri( pTri );
		}
	}

	FASSERT( nTriIdx == nTriangleCount );

	free( pFile );

	return nTotalBytesRead;
}


//
//
//
void KongMesh_t::AddMatToSeg( KongSeg_t *pSeg, KongMat_t *pMat )
{
	pMat->pKSeg = pSeg;
	FASSERT( pMat && pMat->pProperties );
	if ( pSeg->nNumMats == 0 )
	{
		FASSERT( pSeg->nFirstMatIdx == KONG_INVALID_MAT_INDEX && pSeg->nLastMatIdx == KONG_INVALID_MAT_INDEX );
		pSeg->nFirstMatIdx = pMat->nMatIdx;
		pSeg->nLastMatIdx = pMat->nMatIdx;
		pMat->nNextMatIdx = KONG_INVALID_MAT_INDEX;
		pSeg->nNumMats++;
	}
	else
	{
		FASSERT( pSeg->nFirstMatIdx != KONG_INVALID_MAT_INDEX && pSeg->nLastMatIdx != KONG_INVALID_MAT_INDEX );
		KongMat_t *pLastMat = NULL;
		KongMat_t *pTestMat = GetFirstMaterial( pSeg );
		if ( pMat->pProperties->bTransparent )
		{
			// Put transparent materials last
			while ( pTestMat )
			{
				FASSERT( pTestMat->pProperties );
				if ( pTestMat->pProperties->bTransparent && pMat->pProperties->nOrderNum > pTestMat->pProperties->nOrderNum )
				{
					if ( pLastMat )
					{
						pMat->nNextMatIdx = pTestMat->nMatIdx;
						pLastMat->nNextMatIdx = pMat->nMatIdx;
						pSeg->nNumMats++;
						#if FANG_DEBUG_BUILD
							VerifySegMatList( pSeg );
						#endif
						return;
					}
					else
					{
						pMat->nNextMatIdx = pTestMat->nMatIdx;
						pSeg->nFirstMatIdx = pMat->nMatIdx;
						pSeg->nNumMats++;
						#if FANG_DEBUG_BUILD
							VerifySegMatList( pSeg );
						#endif
						return;
					}
				}
				pLastMat = pTestMat;
				pTestMat = GetNextMaterial( pTestMat );
			}
		}
		else
		{
			// Put opaque materials before transparent
			while ( pTestMat )
			{
				FASSERT( pTestMat->pProperties );
				if ( pTestMat->pProperties->bTransparent || pMat->pProperties->nOrderNum > pTestMat->pProperties->nOrderNum )
				{
					if ( pLastMat )
					{
						pMat->nNextMatIdx = pTestMat->nMatIdx;
						pLastMat->nNextMatIdx = pMat->nMatIdx;
						pSeg->nNumMats++;
						#if FANG_DEBUG_BUILD
							VerifySegMatList( pSeg );
						#endif
						return;
					}
					else
					{
						pMat->nNextMatIdx = pTestMat->nMatIdx;
						pSeg->nFirstMatIdx = pMat->nMatIdx;
						pSeg->nNumMats++;
						#if FANG_DEBUG_BUILD
							VerifySegMatList( pSeg );
						#endif
						return;
					}
				}
				pLastMat = pTestMat;
				pTestMat = GetNextMaterial( pTestMat );
			}
		}

		FASSERT( paMaterials[pSeg->nLastMatIdx].nNextMatIdx == KONG_INVALID_MAT_INDEX );
		paMaterials[pSeg->nLastMatIdx].nNextMatIdx = pMat->nMatIdx;
		pSeg->nLastMatIdx = pMat->nMatIdx;
		pMat->nNextMatIdx = KONG_INVALID_MAT_INDEX;
		pSeg->nNumMats++;
		#if FANG_DEBUG_BUILD
			VerifySegMatList( pSeg );
		#endif
	}
}


//
//
//
BOOL KongMesh_t::VerifySegMatList( KongSeg_t *pSeg )
{
	FASSERT( pSeg );
	KongMat_t *pMat = GetFirstMaterial( pSeg );
	u32 nMatCount = 0;
	while ( pMat )
	{
		nMatCount++;
		pMat = GetNextMaterial( pMat );
	}

	FASSERT( nMatCount == pSeg->nNumMats );
	return (nMatCount == pSeg->nNumMats);
}


//
//
//
BOOL KongMesh_t::RemoveMatFromSeg( KongSeg_t *pSeg, KongMat_t *pMat )
{
	u32 nLastIdx = KONG_INVALID_MAT_INDEX;
	u32 nCurrentIdx = pSeg->nFirstMatIdx;

	// Next, remove the materials that have 0 tris
	while ( nCurrentIdx != KONG_INVALID_MAT_INDEX )
	{
		if ( &paMaterials[nCurrentIdx] == pMat )
		{
			if ( nLastIdx != KONG_INVALID_MAT_INDEX )
			{
				paMaterials[nLastIdx].nNextMatIdx = pMat->nNextMatIdx;
			}
			else
			{
				pSeg->nFirstMatIdx = pMat->nNextMatIdx;
			}

			if ( pMat->nNextMatIdx == KONG_INVALID_MAT_INDEX )
			{
				pSeg->nLastMatIdx = nLastIdx;
			}

			pMat->nNextMatIdx = KONG_INVALID_MAT_INDEX;
			pSeg->nNumMats--;

			FASSERT( pSeg->nNumMats != 0 || (pSeg->nFirstMatIdx == KONG_INVALID_MAT_INDEX && pSeg->nLastMatIdx == KONG_INVALID_MAT_INDEX) );

			#if FANG_DEBUG_BUILD
				VerifySegMatList( pSeg );
			#endif

			return TRUE;
		}

		nLastIdx = nCurrentIdx;
		nCurrentIdx = paMaterials[nCurrentIdx].nNextMatIdx;
	}

	return FALSE;
}


//
//
//
KongMat_t* KongMesh_t::GetIdenticalMaterial( KongSeg_t *pSeg, KongMatProperties_t *pProperties )
{
	FASSERT( pSeg && pProperties );

	if ( pSeg->nFirstMatIdx == KONG_INVALID_MAT_INDEX )
	{
		return NULL;
	}

	FASSERT( pSeg->nFirstMatIdx < (s32)nMaterialCount );
	KongMat_t *pMat = &paMaterials[pSeg->nFirstMatIdx];

	while ( pMat )
	{
		if ( pMat->pProperties->Equals( pProperties, FALSE ) )
		{
			return pMat;
		}
		if ( pMat->nNextMatIdx != KONG_INVALID_MAT_INDEX )
		{
			FASSERT( pMat->nNextMatIdx < (s32)nMaterialCount );
			pMat = &paMaterials[pMat->nNextMatIdx];
		}
		else
		{
			pMat = NULL;
		}
	}

	return NULL;
}


//
//
//
void KongMesh_t::AddTriToMat( KongMat_t *pMat, KongTri_t *pTri )
{
	FASSERT( pMat && pTri );

	pTri->nTriFlags |= KONGTRI_FLAG_TRI_ADDED_TO_MATERIAL;

	if ( pMat->nNumTris == 0 )
	{
		FASSERT( pMat->nFirstTriIdx == KONG_INVALID_TRI_INDEX && pMat->nLastTriIdx == KONG_INVALID_TRI_INDEX );
		pMat->nFirstTriIdx = pTri->nTriIdx;
		pMat->nLastTriIdx = pTri->nTriIdx;
		pTri->nNextTriIdx = KONG_INVALID_TRI_INDEX;
		pMat->nNumTris++;
	}
	else
	{
		FASSERT( pMat->nFirstTriIdx != KONG_INVALID_TRI_INDEX && pMat->nLastTriIdx != KONG_INVALID_TRI_INDEX );

		FASSERT( paTriangles[pMat->nLastTriIdx].nNextTriIdx == KONG_INVALID_TRI_INDEX );
		paTriangles[pMat->nLastTriIdx].nNextTriIdx = pTri->nTriIdx;
		pMat->nLastTriIdx = pTri->nTriIdx;
		pTri->nNextTriIdx = KONG_INVALID_TRI_INDEX;
		pMat->nNumTris++;
		#if FANG_DEBUG_BUILD
			VerifyMatTriList( pMat );
		#endif
	}
}


//
//
//
BOOL KongMesh_t::VerifyMatTriList( KongMat_t *pMat )
{
	FASSERT( pMat );
	KongTri_t *pTri = GetFirstTri( pMat );
	u32 nTriCount = 0;
	while ( pTri )
	{
		nTriCount++;
		pTri = GetNextTri( pTri );
	}

	FASSERT( nTriCount == pMat->nNumTris );
	return ( nTriCount == pMat->nNumTris );
}


//
//
//
KongMatProperties_t* KongMesh_t::GetIdenticalProperties( KongMatProperties_t *pProperties )
{
	FASSERT( pProperties );

	u32 nMatPropIdx;
	for ( nMatPropIdx = 0; nMatPropIdx < nMatPropertiesCount; nMatPropIdx++ )
	{
		if ( pProperties->Equals( &paMatProperties[nMatPropIdx], FALSE ) )
		{
			return &paMatProperties[nMatPropIdx];
		}
	}

	return NULL;
}


//
//
//
void KongMesh_t::RemoveEmptyMaterials( void )
{
	static s32 nRemap[KONG_MAX_MATERIALS];
	u16 nValidCount = 0;
	KongMat_t SwapMat, *pMat;
	u32 i, ii;

	#if FANG_DEBUG_BUILD
	for ( i = 0; i < nSegmentCount; i++ )
	{
		VerifySegMatList( &paSegs[i] );
	}
	#endif

	for ( i = 0; i < nMaterialCount; i++ )
	{
		nRemap[i] = -1;
	}

	u32 nLastBad = nMaterialCount - 1;
	for ( i = 0; i < nMaterialCount; i++ )
	{
		if ( nRemap[i] != -1 )
		{
			continue;
		}

		if ( paMaterials[i].nNumTris == 0 )
		{
			u32 nValidIn = nValidCount;
			for ( ii = nLastBad; ii > i; ii-- )
			{
				if ( paMaterials[ii].nNumTris > 0 )
				{
					SwapMat = paMaterials[i];
					paMaterials[i] = paMaterials[ii];
					paMaterials[ii] = SwapMat;
					nRemap[ii] = i;
					nRemap[i] = ii;
					nValidCount++;
					nLastBad = ii - 1;
					break;
				}
			}

			if ( nValidIn == nValidCount )
			{
				// We didn't find a match, so the remainder of the materials must be empty
				nRemap[i] = i;
			}

			continue;
		}

		nValidCount++;
		nRemap[i] = i;
	}

	// Remap the material indices
	for ( i = 0; i < nSegmentCount; i++ )
	{
		KongSeg_t *pSeg = &paSegs[i];

		if ( pSeg->nNumMats == 0 )
		{
			continue;
		}

		pSeg->nFirstMatIdx = nRemap[pSeg->nFirstMatIdx];
		pSeg->nLastMatIdx = nRemap[pSeg->nLastMatIdx];

		// First, remap all of the materials
		pMat = GetFirstMaterial( pSeg );
		while ( pMat )
		{
			pMat->nMatIdx = nRemap[pMat->nMatIdx];
			if ( pMat->nNextMatIdx != KONG_INVALID_MAT_INDEX )
			{
				pMat->nNextMatIdx = nRemap[pMat->nNextMatIdx];
			}

			pMat = GetNextMaterial( pMat );
		}

		#if FANG_DEBUG_BUILD
			VerifySegMatList( pSeg );
		#endif

		// Next, remove the materials that have 0 tris
		u32 nNext, nCurr = pSeg->nFirstMatIdx;
		while ( nCurr != KONG_INVALID_MAT_INDEX )
		{
			nNext = paMaterials[nCurr].nNextMatIdx;
			if ( paMaterials[nCurr].nMatIdx >= nValidCount )
			{
				RemoveMatFromSeg( pSeg, &paMaterials[nCurr] );
			}

			nCurr = nNext;
		}

		#if FANG_DEBUG_BUILD
			VerifySegMatList( pSeg );
		#endif
	}

	nMaterialCount = nValidCount;
}


//
//
//
void KongMesh_t::ComputeBoundingSphere( CFVec3 &Center, f32 &fRadius, BOOL bVolumeMesh/*=FALSE*/ ) const 
{
	// compute the mix/max for all of the tris
	f32 fRadiusOld;
	CFVec3 Min, Max, vCenterOld;

	CFVec3 *pV;
	u32 i, j;

	// Use the first xyz as both the min & max
	pV = (CFVec3 *)&paTriangles[0].apKongVerts[0]->Pos;
	Min.Set( pV->x, pV->y, pV->z );
	Max = Min;

	for( i=0; i < nTriangleCount; i++ ) 
	{
		for( j=0; j < 3; j++ ) 
		{
			pV = (CFVec3 *)&paTriangles[i].apKongVerts[j]->Pos;
			
			Min.x = FMATH_MIN( pV->x, Min.x );
			Min.y = FMATH_MIN( pV->y, Min.y );
			Min.z = FMATH_MIN( pV->z, Min.z );

			Max.x = FMATH_MAX( pV->x, Max.x );
			Max.y = FMATH_MAX( pV->y, Max.y );
			Max.z = FMATH_MAX( pV->z, Max.z );
		}
	}

	utils_GetBoundingSphereFromMinMax( vCenterOld, fRadiusOld, Min, Max );

	if ( bVolumeMesh )
	{
		Center = vCenterOld;
		fRadius = fRadiusOld;
		return;
	}

	Center.Zero();
	u32 nSeg;
	KongMat_t *pMat;
	u32 nTotalVerts = 0;
	for ( nSeg = 0; nSeg < nSegmentCount; nSeg++ )
	{
		KongSeg_t *pSeg = &paSegs[nSeg];

		pMat = GetFirstMaterial( pSeg );
		while ( pMat )
		{
			KongTri_t *pTri = GetFirstTri( pMat );
			while ( pTri )
			{
				nTotalVerts += 3;
				Center += pTri->apKongVerts[0]->Pos;
				Center += pTri->apKongVerts[1]->Pos;
				Center += pTri->apKongVerts[2]->Pos;
				pTri = GetNextTri( pTri );
			}
			pMat = GetNextMaterial( pMat );
		}
	}

	Center = ( Min + Max ) / 2.f;
	f32 fTest;
	fRadius = 0.f;
	CFVec3 vTest;
	for ( nSeg = 0; nSeg < nSegmentCount; nSeg++ )
	{
		KongSeg_t *pSeg = &paSegs[nSeg];

		pMat = GetFirstMaterial( pSeg );
		while ( pMat )
		{
			KongTri_t *pTri = GetFirstTri( pMat );
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
				pTri = GetNextTri( pTri );
			}
			pMat = GetNextMaterial( pMat );
		}
	}
	fRadius = fmath_AcuSqrt( fRadius ) * UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;

#if 0
	if ( fRadiusOld < fRadius )
	{
		Center = vCenterOld;
		fRadius = fRadiusOld;
	}
#endif
}


//
//
//
static BOOL _IsTranslucent( KongMatProperties_t *pProperties )
{
	FASSERT( pProperties );

	// Now override the defaults with actual shader entries
	switch( pProperties->nShaderNum ) 
	{
		//////////////////
		// Translucent Shaders
		//////////////////
		case APE_SHADER_TYPE_tBASE:
		case APE_SHADER_TYPE_etbsBASE:
		case APE_SHADER_TYPE_ADD_BASE:
		case APE_SHADER_TYPE_pBASE:
		case APE_SHADER_TYPE_epbsBASE:
		case APE_SHADER_TYPE_ADD_vBASE:
		case APE_SHADER_TYPE_tBASE_LERP_tLAYER:
		case APE_SHADER_TYPE_etBASE_ADD_rbENV:
		case APE_SHADER_TYPE_tBASE_vALPHA:
		case APE_SHADER_TYPE_LIQUID_ENV:
		case APE_SHADER_TYPE_LIQUID_LAYER_ENV:
		case APE_SHADER_TYPE_LIQUID_TEXTURE:
			return TRUE;
			break;
	} 

	return FALSE;
}


//
//
//
void KongMatProperties_t::Set( ApeMaterial_t *pApeMat )
{
	nFlags = pApeMat->StarCommands.nFlags | pApeMat->nFlags;
	fUnitAlphaMultiplier = pApeMat->aLayer[0].fUnitAlphaMultiplier;
	bDrawAsWire = pApeMat->aLayer[0].bDrawAsWire;
	bTwoSided = pApeMat->aLayer[0].bTwoSided;
	bSort = pApeMat->StarCommands.bSort;
	nOrderNum = pApeMat->StarCommands.nOrderNum;
	nShaderNum = pApeMat->StarCommands.nShaderNum;
	nAffectAngle = (u8)pApeMat->nAffectAngle;
	if ( nAffectAngle != 0 )
	{
		nAffectAngle = nAffectAngle;
	}

	nEmissiveMotifID = pApeMat->aLayer[0].StarCommands.nEmissiveMotifID;
	bUseEmissiveColor = pApeMat->aLayer[0].StarCommands.bUseEmissiveColor;
	EmissiveRGB = pApeMat->aLayer[0].SelfIllumRGB;

	nSpecularMotifID = pApeMat->aLayer[0].StarCommands.nSpecularMotifID;
	bUseSpecularColor = pApeMat->aLayer[0].StarCommands.bUseSpecularColor;
	SpecularRGB = pApeMat->aLayer[0].SpecularRGB;

	nDiffuseMotifID = pApeMat->aLayer[0].StarCommands.nDiffuseMotifID;
	bUseDiffuseColor = pApeMat->aLayer[0].StarCommands.bUseDiffuseColor;
	DiffuseRGB = pApeMat->aLayer[0].DiffuseRGB;

	fShininess = pApeMat->aLayer[0].fShininess;
	fShinStr = pApeMat->aLayer[0].fShinStr;

	nZTugValue = pApeMat->aLayer[0].StarCommands.nZTugValue;

	if ( pApeMat->aLayer[0].StarCommands.fBumpMapTileFactor == 0.f )
	{
        fBumpMapTileFactor = 1.f;  // Default bump map tile factor is 1.f
	}
	else
	{
        fBumpMapTileFactor = pApeMat->aLayer[0].StarCommands.fBumpMapTileFactor;
	}

	if ( pApeMat->aLayer[0].StarCommands.fDetailMapTileFactor == 0.f )
	{
		fDetailMapTileFactor = 4.f;  // Default detail map tile factor is 4.f
	}
	else
	{
		fDetailMapTileFactor = pApeMat->aLayer[0].StarCommands.fDetailMapTileFactor;
	}

	bNoColl = pApeMat->StarCommands.bNoColl;
	nCollID = pApeMat->StarCommands.nCollID;

	nCollMask = pApeMat->StarCommands.nCollMask;
	nReactType = pApeMat->StarCommands.nReactType;
	nSurfaceType = pApeMat->StarCommands.nSurfaceType;

	TintRGB = pApeMat->StarCommands.TintRGB;
	LightRGBI = pApeMat->StarCommands.LightRGBI;

	nLODIndex = (u8)pApeMat->nLODIndex;

	FASSERT( pApeMat->nLayersUsed <= KONG_MAX_LAYERS_PER_MAT );
	memset( aLayer, 0, sizeof( KongLayer_t ) * KONG_MAX_LAYERS_PER_MAT );
	nLayersUsed = pApeMat->nLayersUsed;
	u32 i;
	for ( i = 0; i < pApeMat->nLayersUsed; i++ )
	{
		aLayer[i].abTile[0] = pApeMat->aLayer[i].abTile[0];
		aLayer[i].abTile[1] = pApeMat->aLayer[i].abTile[1];
		aLayer[i].fUVRotation = pApeMat->aLayer[i].StarCommands.fDeltaUVRotationPerSec;
		aLayer[i].vUVRotAnchor = pApeMat->aLayer[i].StarCommands.vRotateUVAround;
		aLayer[i].fDeltaUPerSec = pApeMat->aLayer[i].StarCommands.fDeltaUPerSec;
		aLayer[i].fDeltaVPerSec = pApeMat->aLayer[i].StarCommands.fDeltaVPerSec;
		aLayer[i].fFramesPerSecs = pApeMat->aLayer[i].StarCommands.fFramesPerSecs;
		aLayer[i].nID = pApeMat->aLayer[i].StarCommands.nID;
		aLayer[i].nNumTexFrames = pApeMat->aLayer[i].StarCommands.nNumTexFrames;
		memcpy( aLayer[i].szTexnames, pApeMat->aLayer[i].szTexnames, APE_LAYER_TEXTURE_MAX * TEXTURE_NAME_LEN );
	}

	bTransparent = _IsTranslucent( this );
}


//
//
//
BOOL KongMatProperties_t::Equals( KongMatProperties_t *pProperties, BOOL bMLCompare )
{
	if ( bMLCompare )
	{
		u32 nLocalFlags = nFlags & ~(APE_MAT_FLAGS_DO_NOT_LM|APE_MAT_FLAGS_NO_LM_USE|APE_MAT_FLAGS_DO_NOT_BLOCK_LM|APE_MAT_FLAGS_VERT_RADIOSITY);
		u32 nCompareFlags = pProperties->nFlags & ~(APE_MAT_FLAGS_DO_NOT_LM|APE_MAT_FLAGS_NO_LM_USE|APE_MAT_FLAGS_DO_NOT_BLOCK_LM|APE_MAT_FLAGS_VERT_RADIOSITY);
		if ( nLocalFlags != nCompareFlags )
		{
			return FALSE;
		}
	}
	else
	{
		if ( nFlags != pProperties->nFlags )
		{
			return FALSE;
		}
	}

	if (   fUnitAlphaMultiplier	!= pProperties->fUnitAlphaMultiplier 
		|| bDrawAsWire			!= pProperties->bDrawAsWire 
		|| bTwoSided			!= pProperties->bTwoSided 
		|| bSort				!= pProperties->bSort 
		|| nOrderNum			!= pProperties->nOrderNum 
		|| nShaderNum			!= pProperties->nShaderNum 
		|| fShininess			!= pProperties->fShininess 
		|| fShinStr				!= pProperties->fShinStr 
		|| nZTugValue			!= pProperties->nZTugValue 
		|| bNoColl				!= pProperties->bNoColl 
		|| nCollID				!= pProperties->nCollID 
		|| nCollMask			!= pProperties->nCollMask 
		|| nReactType			!= pProperties->nReactType 
		|| nSurfaceType			!= pProperties->nSurfaceType 
		|| TintRGB				!= pProperties->TintRGB 
		|| LightRGBI			!= pProperties->LightRGBI 
		|| bUseEmissiveColor	!= pProperties->bUseEmissiveColor 
		|| bUseSpecularColor	!= pProperties->bUseSpecularColor 
		|| bUseDiffuseColor		!= pProperties->bUseDiffuseColor
		|| nLayersUsed			!= pProperties->nLayersUsed
		|| fDetailMapTileFactor	!= pProperties->fDetailMapTileFactor
		|| fBumpMapTileFactor	!= pProperties->fBumpMapTileFactor
		|| nAffectAngle			!= pProperties->nAffectAngle )
	{
		return FALSE;
	}

	if ( bUseEmissiveColor && ( nEmissiveMotifID != pProperties->nEmissiveMotifID 
		|| EmissiveRGB != pProperties->EmissiveRGB ) )
	{
		return FALSE;
	}
	if ( bUseSpecularColor && ( nSpecularMotifID != pProperties->nSpecularMotifID 
		|| SpecularRGB != pProperties->SpecularRGB ) )
	{
		return FALSE;
	}
	if ( bUseDiffuseColor && ( nDiffuseMotifID != pProperties->nDiffuseMotifID 
		|| DiffuseRGB != pProperties->DiffuseRGB ) )
	{
		return FALSE;
	}

	if (   bEmissiveOn		!= pProperties->bEmissiveOn
		|| bSpecularOn		!= pProperties->bSpecularOn 
		|| bEnviroMotifOn	!= pProperties->bEnviroMotifOn 
		|| nEnvironmentMap	!= pProperties->nEnvironmentMap 
		|| nTransparencyMask!= pProperties->nTransparencyMask 
		|| nSpecularMask	!= pProperties->nSpecularMask 
		|| nEmissiveMask	!= pProperties->nEmissiveMask 
		|| nSurfaceShaderID	!= pProperties->nSurfaceShaderID 
		|| nLightShaderID	!= pProperties->nLightShaderID 
		|| nSpecularShaderID!= pProperties->nSpecularShaderID )
	{
		return FALSE;
	}

	u32 i;
	for ( i = 0; i < nLayersUsed; i++ )
	{
		if (   aLayer[i].abTile[0]		!= pProperties->aLayer[i].abTile[0] 
			|| aLayer[i].abTile[1]		!= pProperties->aLayer[i].abTile[1] 
			|| aLayer[i].fDeltaUPerSec	!= pProperties->aLayer[i].fDeltaUPerSec 
			|| aLayer[i].fDeltaVPerSec	!= pProperties->aLayer[i].fDeltaVPerSec 
			|| aLayer[i].fFramesPerSecs	!= pProperties->aLayer[i].fFramesPerSecs 
			|| aLayer[i].nNumTexFrames	!= pProperties->aLayer[i].nNumTexFrames
			|| aLayer[i].fUVRotation	!= pProperties->aLayer[i].fUVRotation
			|| aLayer[i].vUVRotAnchor	!= pProperties->aLayer[i].vUVRotAnchor )
		{
			return FALSE;
		}

		// See if we should check ids
		BOOL bCheckID = FALSE;
		if ( aLayer[i].nID < 128 || pProperties->aLayer[i].nID  < 128 ) 
		{
			// One of these layers has an assigned ID, we need to check the ids
			bCheckID = TRUE;
		} 
		else if ( aLayer[i].nID  == 255 || pProperties->aLayer[i].nID == 255 ) 
		{
			// One or both of these don't have an id, we wouldn't want to compress if one did and the other didn't
			bCheckID = TRUE;
		}
		if ( bCheckID ) 
		{
			if( aLayer[i].nID != pProperties->aLayer[i].nID ) 
			{
				return FALSE;
			}
		}

		u32 ii;
		for ( ii = 0; ii < APE_LAYER_TEXTURE_MAX; ii++ )
		{
			if ( stricmp( aLayer[i].szTexnames[ii], pProperties->aLayer[i].szTexnames[ii] ) != 0 )
			{
				return FALSE;
			}
		}
	}

	if ( bMLCompare )
	{
		return TRUE;
	}

	// Compare the items that are relevant outside of the ML Library
	if ( nLODIndex != pProperties->nLODIndex )
	{
		return FALSE;
	}

	return TRUE;
}


//
//
//
void KongMat_t::SetShaderTypes( void )
{
	FASSERT( pProperties );
//	ApeMaterial_t *pApeMat = pApeMat;
//	FASSERT( pApeMat );

	// Set Default Values:
	pProperties->bEmissiveOn = FALSE;
	pProperties->bSpecularOn = FALSE;
	pProperties->nEnvironmentMap = -1;
	pProperties->nTransparencyMask = -1;
	pProperties->nSpecularMask = -1;
	pProperties->nEmissiveMask = -1;
	pProperties->nSurfaceShaderID = FSH_INVALID_SURFACE_SHADER;
	pProperties->nLightShaderID = FSH_INVALID_LIGHT_SHADER;
	pProperties->nSpecularShaderID = FSH_INVALID_SPECULAR_SHADER;

	// determine if specular and emissive are "turned on" from the base layer
	f32 fTotalEmissive = 0;
	if ( pProperties->bUseEmissiveColor ) 
	{
		fTotalEmissive = pProperties->EmissiveRGB.fRed + pProperties->EmissiveRGB.fGreen + pProperties->EmissiveRGB.fBlue;

		/*if ( pProperties->EmissiveRGB.fRed > 0.9f && pProperties->EmissiveRGB.fGreen > 0.9f && pProperties->EmissiveRGB.fBlue > 0.9f )
		{
			// Highly emissive materials will not accept lightmaps
			pProperties->nFlags |= APE_MAT_FLAGS_DO_NOT_LM;
		}*/
	}
	pProperties->bEmissiveOn = ( fTotalEmissive > 0.f );
	pProperties->bSpecularOn = ( pProperties->fShininess > 0.0f );


	// Set a default shader type
	if ( pProperties->nLayersUsed == 1 ) 
	{
		// Default 1 layer shader
		// APE_SHADER_TYPE_cBASE:
		pProperties->nSurfaceShaderID = FSHADERS_cBASE;
		pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC_CO;
	}
	else if ( pProperties->nLayersUsed == 2 ) 
	{
		// Default 2 layer shader
		// APE_SHADER_TYPE_oBASE_LERP_pLAYER:
		pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_pLAYER;
		pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
	}
	else
	{
		// We shouldn't get textures with more than one layer
		FASSERT_NOW;
		pProperties->nSurfaceShaderID = FSHADERS_cBASE;
		pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC_CO;
	}

	//look at blend shaders, if envmap exists then switch to appropriate shader.
	if ( pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT][0] )
	{
		if ( pProperties->nShaderNum == APE_SHADER_TYPE_oBASE_LERP_pLAYER )
		{
			pProperties->nShaderNum = APE_SHADER_TYPE_oBASE_LERP_pLAYER_ADD_rbENV;
		}
		else if ( pProperties->nShaderNum == APE_SHADER_TYPE_oBASE_LERP_vLAYER )
		{
			pProperties->nShaderNum = APE_SHADER_TYPE_oBASE_LERP_vLAYER_ADD_rbENV;
		}
		else if ( pProperties->nShaderNum == APE_SHADER_TYPE_oBASE_LERP_tLAYER )
		{
			pProperties->nShaderNum = APE_SHADER_TYPE_oBASE_LERP_tLAYER_ADD_rbENV;
		}

	}

	// Now override the defaults with actual shader entries
	switch( pProperties->nShaderNum ) 
	{
		//////////////////
		// 1 layer shaders
		//////////////////
		case APE_SHADER_TYPE_oBASE:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			break;

		case APE_SHADER_TYPE_cBASE:
			pProperties->nTransparencyMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_cBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC_CO;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;

		case APE_SHADER_TYPE_tBASE:
			pProperties->nTransparencyMask = 0;
			if ( pProperties->bEmissiveOn && pProperties->bSpecularOn ) 
			{
				pProperties->nSurfaceShaderID = FSHADERS_etBASE_ADD_SPEC;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			else if ( pProperties->bEmissiveOn ) 
			{
				pProperties->nSurfaceShaderID = FSHADERS_etBASE;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			} 
			else if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSurfaceShaderID = FSHADERS_tBASE_ADD_SPEC;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			else 
			{
				pProperties->nSurfaceShaderID = FSHADERS_tBASE;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			}
			break;

		case APE_SHADER_TYPE_pBASE:
			pProperties->nTransparencyMask = 0;
			if ( pProperties->bEmissiveOn && pProperties->bSpecularOn ) 
			{
				pProperties->nSurfaceShaderID = FSHADERS_epBASE_ADD_SPEC;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			else if ( pProperties->bEmissiveOn ) 
			{
				pProperties->nSurfaceShaderID = FSHADERS_epBASE;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			} 
			else if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSurfaceShaderID = FSHADERS_pBASE_ADD_SPEC;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			else 
			{
				pProperties->nSurfaceShaderID = FSHADERS_pBASE;
				pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			}
			break;

		case APE_SHADER_TYPE_obsBASE:
			pProperties->nSpecularMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_oBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			pProperties->nSpecularShaderID = FSHADERS_VSPEC_MASK;
			break;
			
		case APE_SHADER_TYPE_beoBASE:
			pProperties->nEmissiveMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_oBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_MASK_EMISSIVE;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			break;

		case APE_SHADER_TYPE_etbsBASE:
			pProperties->nTransparencyMask = 0;
			pProperties->nSpecularMask = FSHADERS_VSPEC_BASIC_CO;
			pProperties->nSurfaceShaderID = FSHADERS_etBASE_ADD_bSPEC;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;

		case APE_SHADER_TYPE_epbsBASE:
			pProperties->nTransparencyMask = 0;
			pProperties->nSpecularMask = FSHADERS_VSPEC_BASIC_CO;
			pProperties->nSurfaceShaderID = FSHADERS_epBASE_ADD_bSPEC;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;

		case APE_SHADER_TYPE_ADD_BASE:
			pProperties->nSurfaceShaderID = FSHADERS_ADD_BASE;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;

		case APE_SHADER_TYPE_ADD_vBASE:
			pProperties->nSurfaceShaderID = FSHADERS_ADD_vBASE;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;

		//////////////////
		// 2 layer shaders
		//////////////////
		case APE_SHADER_TYPE_oBASE_LERP_pLAYER:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_pLAYER;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			break;

		case APE_SHADER_TYPE_cBASE_LERP_pLAYER:
			pProperties->nTransparencyMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_cBASE_LERP_pLAYER;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC_CO;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;

		case APE_SHADER_TYPE_oBASE_LERP_vLAYER:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_vLAYER;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			break;

		case APE_SHADER_TYPE_cBASE_LERP_vLAYER:
			pProperties->nTransparencyMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_cBASE_LERP_vLAYER;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC_CO;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;
		
		case APE_SHADER_TYPE_oBASE_LERP_tLAYER:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_tLAYER;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			break;

		case APE_SHADER_TYPE_cBASE_LERP_tLAYER:
			pProperties->nTransparencyMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_cBASE_LERP_tLAYER;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC_CO;
			if( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;

		case APE_SHADER_TYPE_tBASE_LERP_tLAYER:
			pProperties->nTransparencyMask = 0;
			pProperties->nSurfaceShaderID = FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;

		case APE_SHADER_TYPE_oBASE_ADD_rbENV:
/*
			// This code will auto LOD the environment map off for LODs > 1
			if ( pProperties->nLODIndex > 1 )
			{
				pProperties->nSurfaceShaderID = FSHADERS_oBASE;
				pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			}
			else
*/
			{
				pProperties->nEnvironmentMap = 0;
				pProperties->bEnviroMotifOn = TRUE;
				pProperties->nSurfaceShaderID = FSHADERS_oBASE_ADD_rbENV;
				pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			}
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			break;

		case APE_SHADER_TYPE_oBASE_LERP_tLAYER_ADD_rbENV:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			pProperties->nEnvironmentMap = 0;
			pProperties->bEnviroMotifOn = TRUE;
			break;
		case APE_SHADER_TYPE_oBASE_LERP_vLAYER_ADD_rbENV:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			pProperties->nEnvironmentMap = 0;
			pProperties->bEnviroMotifOn = TRUE;
			break;
		case APE_SHADER_TYPE_oBASE_LERP_pLAYER_ADD_rbENV:
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC;
			} 
			pProperties->nEnvironmentMap = 0;
			pProperties->bEnviroMotifOn = TRUE;
			break;
		case APE_SHADER_TYPE_etBASE_ADD_rbENV:
			pProperties->nEnvironmentMap = 0;
			pProperties->nTransparencyMask = 0;
			pProperties->bEnviroMotifOn = TRUE;
			pProperties->nSurfaceShaderID = FSHADERS_etBASE_ADD_rbENV;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;

		case APE_SHADER_TYPE_oBASE_ADD_rbSREFLECT:
			pProperties->nEnvironmentMap = 0;
			pProperties->bEnviroMotifOn = TRUE;
			pProperties->nSurfaceShaderID = FSHADERS_oBASE_ADD_rbSREFLECT;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			if ( pProperties->bSpecularOn ) 
			{
				pProperties->nSpecularShaderID = FSHADERS_VSPEC_BASIC_CO;
			} 
			break;

		//LIQUID SHADERS
		case APE_SHADER_TYPE_LIQUID_ENV:
			pProperties->nEnvironmentMap = 0;
			pProperties->nSurfaceShaderID = FSHADERS_LIQUID_ENV;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;
		case APE_SHADER_TYPE_LIQUID_LAYER_ENV:
			pProperties->nEnvironmentMap = 0;
			pProperties->nSurfaceShaderID = FSHADERS_LIQUID_LAYER_ENV;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;
		case APE_SHADER_TYPE_LIQUID_TEXTURE:
			pProperties->nSurfaceShaderID = FSHADERS_LIQUID_TEXTURE;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;
		case APE_SHADER_TYPE_LIQUID_MOLTEN_1LAYER:
			pProperties->nEnvironmentMap = 0;
			pProperties->nSurfaceShaderID = FSHADERS_LIQUID_MOLTEN_1LAYER;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;
		case APE_SHADER_TYPE_LIQUID_MOLTEN_2LAYER:
			pProperties->nEnvironmentMap = 0;
			pProperties->nSurfaceShaderID = FSHADERS_LIQUID_MOLTEN_2LAYER;
			pProperties->nLightShaderID = FSHADERS_FULLBRIGHT;
			break;
		//

		case APE_SHADER_TYPE_oBASE_ADD_rbENV_MOD_SHADOWMAP:
			DEVPRINTF( "CApeToKongFormat::SetShaderTypes() - WARNING! - oBASE_ADD_rbENV_MOD_SHADOWMAP no longer supported\n" );
			pProperties->nSurfaceShaderID = FSHADERS_cBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			break;
		
		case APE_SHADER_TYPE_oBASE_MOD_SHADOWMAP:
			DEVPRINTF( "CApeToKongFormat::SetShaderTypes() - WARNING! - oBASE_MOD_SHADOWMAP no longer supported\n" );
			pProperties->nSurfaceShaderID = FSHADERS_cBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			break;

		case APE_SHADER_TYPE_cBASE_MOD_SHADOWMAP:
			DEVPRINTF( "CApeToKongFormat::SetShaderTypes() - WARNING! - cBASE_MOD_SHADOWMAP no longer supported\n" );
			pProperties->nSurfaceShaderID = FSHADERS_cBASE;
			pProperties->nLightShaderID = FSHADERS_VLIGHT_BASIC;
			break;

		default:
			break;
	} 

	KongLayer_t *pBaseLayer = &pProperties->aLayer[0];
	if( pProperties->nTransparencyMask != -1 )
	{
		// Make sure we use the Alpha texture, if one is specified
		if ( pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK][0] )
		{
			if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK]) == 0 )
			{
				pProperties->nTransparencyMask = APE_LAYER_TEXTURE_DIFFUSE;
			}
			else
			{
				pProperties->nTransparencyMask = APE_LAYER_TEXTURE_ALPHA_MASK;
			}
		}
		else
		{
			pProperties->nTransparencyMask = APE_LAYER_TEXTURE_DIFFUSE;
		}
	}

	if ( pProperties->bEmissiveOn )
	{
		// Make sure we use the emissive texture, if one is specified
		if ( pBaseLayer->szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK][0] )
		{
			if (pProperties->nLightShaderID == FSHADERS_VLIGHT_BASIC)
			{
				pProperties->nLightShaderID = FSHADERS_VLIGHT_MASK_EMISSIVE;
			}
			else if (pProperties->nLightShaderID == FSHADERS_VLIGHT_BASIC_CO)
			{
				pProperties->nLightShaderID = FSHADERS_VLIGHT_MASK_EMISSIVE_CO;
			}

			if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK]) == 0 )
			{
				pProperties->nEmissiveMask = APE_LAYER_TEXTURE_DIFFUSE;
			}
			else if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK]) == 0 )
			{
				pProperties->nEmissiveMask = APE_LAYER_TEXTURE_ALPHA_MASK;
			}
			else
			{
				pProperties->nEmissiveMask = APE_LAYER_TEXTURE_EMISSIVE_MASK;
			}
		}
		else if ( pProperties->nShaderNum == APE_SHADER_TYPE_beoBASE )
		{
			// Legacy support
			pProperties->nEmissiveMask = APE_LAYER_TEXTURE_DIFFUSE;
		}
		else
		{
			pProperties->nEmissiveMask = -1;

			//turn off lightmapping if the surface is uniformly emissive (no mask).
			if ( pProperties->bUseEmissiveColor ) 
			{
				fTotalEmissive = pProperties->EmissiveRGB.fRed + pProperties->EmissiveRGB.fGreen + pProperties->EmissiveRGB.fBlue;

				if ( pProperties->EmissiveRGB.fRed > 0.9f && pProperties->EmissiveRGB.fGreen > 0.9f && pProperties->EmissiveRGB.fBlue > 0.9f )
				{
					// Highly emissive materials will not accept lightmaps
					pProperties->nFlags |= APE_MAT_FLAGS_DO_NOT_LM;
				}
			}
		}
	}

	if ( pProperties->bSpecularOn )
	{
		// Make sure we use the specular texture, if one is specified
		if ( pBaseLayer->szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK][0] )
		{
			if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK]) == 0 )
			{
				pProperties->nSpecularMask = APE_LAYER_TEXTURE_DIFFUSE;
			}
			else if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK]) == 0 )
			{
				pProperties->nSpecularMask = APE_LAYER_TEXTURE_ALPHA_MASK;
			}
			else if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK]) == 0 )
			{
				pProperties->nSpecularMask = APE_LAYER_TEXTURE_EMISSIVE_MASK;
			}
			else
			{
				pProperties->nSpecularMask = APE_LAYER_TEXTURE_SPECULAR_MASK;
			}
		}
		else if ( pProperties->nShaderNum == APE_SHADER_TYPE_obsBASE || pProperties->nShaderNum == APE_SHADER_TYPE_etbsBASE || pProperties->nShaderNum == APE_SHADER_TYPE_epbsBASE )
		{
			// Legacy support
			pProperties->nEmissiveMask = APE_LAYER_TEXTURE_DIFFUSE;
		}
		else
		{
			pProperties->nSpecularMask = -1;
		}
	}

	if ( pProperties->nEnvironmentMap != -1 )
	{
		// Make sure we use the environment texture, if one is specified
		if ( pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT][0] )
		{
			if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT]) == 0 )
			{
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_DIFFUSE;
			}
			else if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT]) == 0 )
			{
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_ALPHA_MASK;
			}
			else if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT]) == 0 )
			{
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_EMISSIVE_MASK;
			}
			else if ( stricmp(pBaseLayer->szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK], pBaseLayer->szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT]) == 0 )
			{
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_SPECULAR_MASK;
			}
			else
			{
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_ENVIRONMENT;
			}
		}
		else
		{
			if ( pProperties->aLayer[1].szTexnames[APE_LAYER_TEXTURE_DIFFUSE][0] )
			{
				// This is the legacy method wherein the environment map is specified in the second material of the composite, in Max
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_DIFFUSE + APE_LAYER_TEXTURE_MAX;
			}
			else
			{
				pProperties->nEnvironmentMap = APE_LAYER_TEXTURE_DIFFUSE;
			}
		}
	}
}

