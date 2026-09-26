//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_PC.cpp - Classes used to convert generic mesh data into Fang PC specific data
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
// 03/17/02 Lafleur		Created.
// 08/15/02 Lafleur		Adapted from platform specific GCMeshData.cpp
//////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "stdafx.h"
#include <mmsystem.h>

// Fang Includes
#include "FkDOP.h"

// MeshLib Includes
#include "MLMesh_PC.h"
#include "MLHash.h"
#include "MLMaterial_PC.h"

// From NVidia stripping library
#include "NvTriStrip.h"

#include "..\CompileDlg.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////

#define CONVERT_CFCOLOR_TO_D3D( x )		((u8)(x.fAlpha * 255) << 24) | ((u8)(x.fRed * 255) << 16) | ((u8)(x.fGreen * 255) << 8) | (u8)(x.fBlue * 255) 


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
MLResults* MLMesh_PC::GenerateExportData( BOOL bGenerateCollisionData )
{
	u32 nProcessStartTime;
	u32 nStartTime = timeGetTime();

	u32 i, ii, iii;

	// Reset Collision Counters
	MLMesh_nTotalNodeCount = 0;
	MLMesh_nTotalIntervalCount = 0;
	MLMesh_nTotalTriCount = 0;
	MLMesh_nTotalTriPackets = 0;
	MLMesh_nTotalTriDataBytes = 0;
	MLMesh_nTotalRootDOPVertBytes = 0;
	MLMesh_nTotalkDOPFaceCount = 0;

	MLManager.m_pDlg->WriteToLog( "ML - Generating Export Data.\n" );

	// Clear the results
	memset( &m_Results, 0, sizeof( MLResults ) );

	// Determine the number of things to store in the export buffer
	u32 nSegments		= m_FMesh.nSegCount;
	u32 nTexLayerIDs	= m_FMesh.nTexLayerIDCount;
	u32 nMeshLights		= m_FMesh.nLightCount;
	u32 nBones			= m_FMesh.nBoneCount;
	u32 nMaterials		= m_nMaterialCount;

	// If we don't have any segments, we have a problem
	if ( !nSegments || !m_pFirstSegment )
	{
		m_Results.nErrorCode = ML_NO_SEGMENTS_ESTABLISHED_FOR_EXPORT;
		return &m_Results;
	}

	m_Results.nErrorCode = CalculateMeshLODSwitches();
	if ( m_Results.nErrorCode )
	{
		return &m_Results;
	}

	MLManager.m_pDlg->WriteToLog( "ML - Processing Verts.\n" );

	// Cycle through the segments transforming the verts
	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		pTempSegment->ProcessVerts( m_pBoneArray );
		pTempSegment = pTempSegment->m_pNext;
	}

	// Resolve the materials
	nProcessStartTime = timeGetTime();
	m_Results.nErrorCode = ResolveMaterials();
	if ( m_Results.nErrorCode )
	{
		return &m_Results;
	}
	m_Results.fDLTime = (f32)(timeGetTime() - nProcessStartTime) * 0.001f;

	MLManager.m_pDlg->WriteToLog( "ML - Compressing Verts.\n" );

	// Separate vertices by VB type and remove dupes
	nProcessStartTime = timeGetTime();
	m_Results.nErrorCode = DivideVertsByVB();
	if ( m_Results.nErrorCode )
	{
		return &m_Results;
	}
	m_Results.fCompressTime = (f32)(timeGetTime() - nProcessStartTime) * 0.001f;

	// Calculate the number and size of VB's
	u32 nLightMapSTCount = 0;
	u32 nBasisVectorCount = 0;
	u32 nVBs = 0;
	u32 nVBSize = 0;
	u32 nCollVertCount = 0;
	u32 nTotalVertCount = 0;
	u8  nUseBasis=0;
	u8 anLightMapSTs[MLHASH_MAX_PC_TABLES];

	//pSampleTriCont->m_pMaterial->m_nBumpMapTexture

	for ( i = 0; i < MLManager.m_pPCVertHash->m_nHashTableCount; i++ )
	{
		// Set the index for the tri containers that reference this VB
		MLTriContainer *pSampleTriCont = NULL;
		pTempSegment = m_pFirstSegment;
		while ( pTempSegment )
		{
			MLTriContainer *pTempTriCont = pTempSegment->m_pFirstTriContainer;
			while ( pTempTriCont )
			{
				if (pTempTriCont->m_pMaterial->m_nBumpMapTexture > -1)
				{
					nUseBasis = 1;
					break;
				}

				pTempTriCont = pTempTriCont->m_pNext;
			}

			pTempSegment = pTempSegment->m_pNext;
		}
	}

	for ( i = 0; i < MLManager.m_pPCVertHash->m_nHashTableCount; i++ )
	{
		u32 nVertCount = MLManager.m_pPCVertHash->m_apHashTables[i]->m_nActiveNodeCount;

		// Set the index for the tri containers that reference this VB
		MLTriContainer *pSampleTriCont = NULL;
		pTempSegment = m_pFirstSegment;
		while ( pTempSegment )
		{
			MLTriContainer *pTempTriCont = pTempSegment->m_pFirstTriContainer;
			while ( pTempTriCont )
			{
				FASSERT( pTempTriCont->m_nHashTableIdx > -1 && pTempTriCont->m_nHashTableIdx < MLManager.m_pPCVertHash->m_nHashTableCount );
				if ( pTempTriCont->m_nHashTableIdx == i )
				{
					pSampleTriCont = pTempTriCont;
					pTempTriCont->m_nVBIndex = nVBs;
				}

				pTempTriCont = pTempTriCont->m_pNext;
			}

			pTempSegment = pTempSegment->m_pNext;
		}

		FASSERT( pSampleTriCont );

		anLightMapSTs[nVBs] = pSampleTriCont->m_nLightMapSTSets;
		nCollVertCount += nVertCount;
		nTotalVertCount += nVertCount;
		nVBSize	+= nVertCount * FDX8VB_InfoTable[pSampleTriCont->m_nVBKey].nVtxBytes;
		nLightMapSTCount += nVertCount * pSampleTriCont->m_nLightMapSTSets;
		nBasisVectorCount += nVertCount*nUseBasis;

		nVBs++;
	}

	// Initialize the stripper
	InitStripper();

	MLManager.m_pDlg->WriteToLog( "ML - Stripping.\n" );

	// Gather info from the materials
	nMaterials = 0;
	u32 nShRegisterCount = 0;
	u32 nShMotifCount = 0;
	u32 nShTexInstCount = 0;
	u32 nClusterCount = 0;
	u32 nCurrentIndexCount = 65536;
	u32 nIndexBufferCount = 0;
	u32 nTotalVertIdxCount = 0;
	u32 nTotalStripCount = 0;
	MLMaterial_PC *pTempMat = (MLMaterial_PC *)m_pFirstMaterial;
	while ( pTempMat )
	{
		if ( pTempMat->m_pKongMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
		{
			// Skip meshes that are flagged as not visible
			pTempMat = (MLMaterial_PC *)pTempMat->m_pNext;
			continue;
		}

		u32 nStrippingTime = timeGetTime();
		pTempMat->BuildPrimitiveGroups( 12, 24, (TRUE & MLManager.m_bGenerateMeshStrips) );
		m_Results.fStrippingTime += (timeGetTime() - nStrippingTime) / 1000.f;
		m_Results.nStripCount += pTempMat->m_nStripCount;
		m_Results.nStripTriCount += pTempMat->m_nStripTriCount;
		m_Results.nListTriCount += pTempMat->m_nListTriCount;

		// Generate bounding sphere
		pTempMat->CalculateBoundingSphere( &m_FMesh.BoundSphere_MS );

		// Determine the number of clusters needed (this equals the sum of 
		// the number of different matrices in each material's DL containers):
		MLDLContainer *pDLCont = pTempMat->m_pFirstDLContainer;
		while ( pDLCont )
		{
			nTotalStripCount += pDLCont->nStripGroupCount;
			nClusterCount += pDLCont->nMatrixCount;

			// Determine the indices needed
			if ( nCurrentIndexCount + pDLCont->nTotalVertIndices > 65535 )
			{
				nIndexBufferCount++;
				nCurrentIndexCount = 0;
			}

			nTotalVertIdxCount += pDLCont->nTotalVertIndices;
			nCurrentIndexCount += pDLCont->nTotalVertIndices;

			pDLCont = pDLCont->pNextDLContainer;
		}

		// Calculate required shader registers, motifs and texture layers
		pTempMat->AddShaderMSRCount( &nShMotifCount, &nShTexInstCount, &nShRegisterCount );

		// Set the temporary material mesh idx
		pTempMat->m_nTempMeshIdx = nMaterials++;

		pTempMat = (MLMaterial_PC *)pTempMat->m_pNext;
	}

	// Uninitialize the stripper
	UninitStripper();

	MLManager.m_pDlg->WriteToLog( "ML - Creating Collision Data.\n" );

	// Create the collision data
	if ( bGenerateCollisionData )
	{
		nProcessStartTime = timeGetTime();
		m_Results.nErrorCode = CreateMeshCollData( m_nCollkDOPType );
		if ( m_Results.nErrorCode )
		{
			return &m_Results;
		}
		m_Results.fCollisionTime = (f32)(timeGetTime() - nProcessStartTime) * 0.001f;
	}


	MLManager.m_pDlg->WriteToLog( "ML - Preparing Contiguous memory for export.\n" );

	nProcessStartTime = timeGetTime();
	// Determine the relative addresses (offsets) of the data in the export buffer (relative to the start)
	u32 ADR_Current			=	0;

	u32 ADR_FMesh			=	RoundUp4B( ADR_Current );
	u32 MEM_FMesh			=	sizeof(FMesh_t);
	ADR_Current				=	ADR_FMesh + MEM_FMesh;

	u32 ADR_FPCMesh			=	RoundUp4B( ADR_Current );
	u32 MEM_FPCMesh			=	sizeof(FDX8Mesh_t);
	ADR_Current				=	ADR_FPCMesh + MEM_FPCMesh;

	u32 ADR_FMeshSeg		=	RoundUp4B( ADR_Current );
	u32 MEM_FMeshSeg		=	nSegments * sizeof(FMeshSeg_t);
	ADR_Current				=	ADR_FMeshSeg + MEM_FMeshSeg;

	u32 ADR_FMeshBone		=	RoundUp16B( ADR_Current );
	u32 MEM_FMeshBone		=	nBones * sizeof(FMeshBone_t);
	ADR_Current				=	ADR_FMeshBone + MEM_FMeshBone;
	if ( MEM_FMeshBone == 0 )
		ADR_FMeshBone = 0;

	u32 ADR_FSkeleton		=	RoundUp4B( ADR_Current );
	u32 MEM_FSkeleton		=	m_nSkeletonArraySize * sizeof(u8);
	ADR_Current				=	ADR_FSkeleton + MEM_FSkeleton;
	if ( MEM_FSkeleton == 0 )
		ADR_FSkeleton = 0;

	u32 ADR_FMeshLight		=	RoundUp4B( ADR_Current );
	u32 MEM_FMeshLight		=	nMeshLights * sizeof(FMeshLight_t);
	ADR_Current				=	ADR_FMeshLight + MEM_FMeshLight;
	if ( MEM_FMeshLight == 0 )
		ADR_FMeshLight = 0;

		// Material stuff:
	u32 ADR_FMeshMtl		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FMeshMtl		=	nMaterials * sizeof(FMeshMaterial_t);
	ADR_Current				=	ADR_FMeshMtl + MEM_FMeshMtl;
	if ( MEM_FMeshMtl == 0 )
		ADR_FMeshMtl = 0;

	u32 ADR_FMeshHWMtl		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FMeshHWMtl		=	nMaterials * sizeof(FDX8MeshMaterial_t);
	ADR_Current				=	ADR_FMeshHWMtl + MEM_FMeshHWMtl;
	if ( MEM_FMeshHWMtl == 0 )
		ADR_FMeshHWMtl = 0;

	u32 ADR_FTexLayerID		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FTexLayerID		=	nTexLayerIDs * sizeof(FMeshTexLayerID_t);
	ADR_Current				=	ADR_FTexLayerID + MEM_FTexLayerID;
	if ( MEM_FTexLayerID == 0 )
		ADR_FTexLayerID = 0;

	u32 ADR_FShTexInst		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FShTexInst		=	nShTexInstCount * sizeof(FShTexInst_t);
	ADR_Current				=	ADR_FShTexInst + MEM_FShTexInst;
	if ( MEM_FShTexInst == 0 )
		ADR_FShTexInst = 0;

	u32 ADR_FShMotif		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FShMotif		=	nShMotifCount * sizeof(CFColorMotif);
	ADR_Current				=	ADR_FShMotif + MEM_FShMotif;
	if ( MEM_FShMotif == 0 )
		ADR_FShMotif = 0;

	u32 ADR_FShRegister		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA?
	u32 MEM_FShRegister		=	nShRegisterCount * sizeof(u32);
	ADR_Current				=	ADR_FShRegister + MEM_FShRegister;
	if ( MEM_FShRegister == 0 )
		ADR_FShRegister = 0;

	u32 ADR_FTexName		=	RoundUp4B( ADR_Current );
	u32 MEM_FTexName		=	(nShTexInstCount * (FDATA_TEXNAME_LEN + 1));
	ADR_Current				=	ADR_FTexName + MEM_FTexName;
	if ( MEM_FTexName == 0 )
		ADR_FTexName = 0;

		// Geo Rendering stuff:
	u32 ADR_FMeshClusters	=	RoundUp4B( ADR_Current );
	u32 MEM_FMeshClusters	=	nClusterCount * sizeof(FDX8MeshCluster_t);
	ADR_Current				=	ADR_FMeshClusters + MEM_FMeshClusters;
	if ( MEM_FMeshClusters == 0 )
		ADR_FMeshClusters = 0;

	u32 ADR_FMeshStrips		=	RoundUp4B( ADR_Current );
	u32 MEM_FMeshStrips		=	nTotalStripCount * sizeof(FDX8MeshStrip_t);
	ADR_Current				=	ADR_FMeshStrips + MEM_FMeshStrips;
	if ( MEM_FMeshStrips == 0 )
		ADR_FMeshStrips = 0;

	u32 ADR_FVB				=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FVB				=	nVBs * sizeof(FDX8VB_t);
	ADR_Current				=	ADR_FVB + MEM_FVB;
	if ( MEM_FVB == 0 )
		ADR_FVB = 0;

	u32 ADR_FVertIBs		=	RoundUp4B( ADR_Current );
	u32 MEM_FVertIBs		=	nIndexBufferCount * sizeof(void *);
	ADR_Current				=	ADR_FVertIBs + MEM_FVertIBs;
	if ( MEM_FVertIBs == 0 )
		ADR_FVertIBs = 0;

	u32 ADR_FIBIndexCounts	=	RoundUp4B( ADR_Current );
	u32 MEM_FIBIndexCounts	=	nIndexBufferCount * sizeof(u16 *);
	ADR_Current				=	ADR_FIBIndexCounts + MEM_FIBIndexCounts;
	if ( MEM_FIBIndexCounts == 0 )
		ADR_FIBIndexCounts = 0;

		// Collision data

	u32 ADR_FCollTree	=	RoundUp4B( ADR_Current );
	u32 MEM_FCollTree	=	(MLMesh_nTotalTreeCount * sizeof(FkDOP_Tree_t));
	ADR_Current			=	ADR_FCollTree + MEM_FCollTree;
	if ( MEM_FCollTree == 0 )
		ADR_FCollTree = 0;

	u32 ADR_FkDOPNodes	=	RoundUp4B( ADR_Current );
	u32 MEM_FkDOPNodes	=	(MLMesh_nTotalNodeCount * sizeof(FkDOP_Node_t));
	ADR_Current			=	ADR_FkDOPNodes + MEM_FkDOPNodes;

	u32 ADR_FIntervals	=	RoundUp4B( ADR_Current );
	u32 MEM_FIntervals	=	(MLMesh_nTotalIntervalCount * sizeof(FkDOP_Interval_t));
	ADR_Current			=	ADR_FIntervals + MEM_FIntervals;

	u32 ADR_FTriPackets	=	RoundUp4B( ADR_Current );
	u32 MEM_FTriPackets	=	(MLMesh_nTotalTriPackets * sizeof(FkDOP_TriPacket_t));
	ADR_Current			=	ADR_FTriPackets + MEM_FTriPackets;

	u32 ADR_FTriData	=	RoundUp4B( ADR_Current );
	u32 MEM_FTriData	=	MLMesh_nTotalTriDataBytes;
	ADR_Current			=	ADR_FTriData + MEM_FTriData;

	u32 ADR_FRootDOPVertData=	RoundUp4B( ADR_Current );
	u32 MEM_FRootDOPVertData=	MLMesh_nTotalRootDOPVertBytes;
	ADR_Current			=	ADR_FRootDOPVertData + MEM_FRootDOPVertData;

	m_Results.nCollMem = ADR_Current - ADR_FCollTree;

	// Since we will convert the rest of the memory to DX resources in the engine, record the
	// current offset as the offset to the disposable data (data which is disposed of post-conversion)
	m_FPCMesh.nDisposableOffset = ADR_Current;

	// On PC, this will have to be copied into an index buffer and will result in lost
	// memory (if we load the mesh into permanent memory.  On Xbox, we can write this
	// out as an Index Buffer and Register it with an existing resource.
	u32 ADR_FVertIndices	=	RoundUp4B( ADR_Current );
	u32 MEM_FVertIndices	=	nTotalVertIdxCount * sizeof(u16);
	ADR_Current				=	ADR_FVertIndices + MEM_FVertIndices;
	if ( MEM_FVertIndices == 0 )
		ADR_FVertIndices = 0;

	u32 ADR_DXVB			=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_DXVB			=	nVBSize;
	ADR_Current				=	ADR_DXVB + MEM_DXVB;
	if ( MEM_DXVB == 0 )
		ADR_DXVB = 0;

	u32 ADR_LightMapSTs		=	RoundUp4B( ADR_Current );
	u32 MEM_LightMapSTs		=	nLightMapSTCount * sizeof( FDX8LightMapST_t );
	ADR_Current				=	ADR_LightMapSTs + MEM_LightMapSTs;
	if ( MEM_LightMapSTs == 0 )
		ADR_LightMapSTs = 0;

	u32 ADR_BasisVectors	=   RoundUp4B( ADR_Current );
	u32 MEM_BasisVectors	=   nBasisVectorCount * sizeof( FDX8BasisVectors_t );
	ADR_Current				=   ADR_BasisVectors + MEM_BasisVectors;
	if ( MEM_BasisVectors == 0 )
		ADR_BasisVectors = 0;

		// Determine the size of the export buffer
	u32 nExportSize		=	RoundUp4B( ADR_Current );


	////////////////////////////////
	// ALLOCATE AND ZERO EXPORT BUFFER

	m_Results.pExportData = malloc( nExportSize );
	if ( !m_Results.pExportData )
	{
		m_Results.nErrorCode = ML_OUT_OF_MEMORY;
		return &m_Results;
	}

	// Set up a handy pointer to make export data easier to access
	u8 *pExportBuffer = (u8 *)m_Results.pExportData;
	memset( pExportBuffer, 0, nExportSize );


	////////////////////////////////
	// PACK MESH DATA INTO BUFFER
	{
		// Fill in FMesh data
		m_FMesh.nSegCount = (u8)nSegments;
		m_FMesh.aSeg = (FMeshSeg_t *)ADR_FMeshSeg;
		m_FMesh.pBoneArray = (FMeshBone_t *)ADR_FMeshBone;
		m_FMesh.pLightArray = (FMeshLight_t *)ADR_FMeshLight;
		m_FMesh.pnSkeletonIndexArray = (u8 *)ADR_FSkeleton;
		m_FMesh.pTexLayerIDArray = (FMeshTexLayerID_t *)ADR_FTexLayerID;
		m_FMesh.nMaterialCount = (u8)nMaterials;
		m_FMesh.aMtl = (FMeshMaterial_t *)ADR_FMeshMtl;
		m_FMesh.nCollTreeCount = (u8)MLMesh_nTotalTreeCount;
		m_FMesh.nMeshCollMask = 0;
		for ( i = 0; i < MLMesh_nTotalTreeCount; i++ )
		{
			m_FMesh.nMeshCollMask |= m_paCollTree[i].nMasterCollMask;
		}
		m_FMesh.pMeshIS = (FDX8Mesh_s *)ADR_FPCMesh;
		m_FMesh.paCollTree = (FkDOP_Tree_t *)ADR_FCollTree;

		// Fill in m_FPCMesh data
		m_FPCMesh.pMesh = (FMesh_t *)ADR_FMesh;
		FASSERT( nVBs < 256 );
		m_FPCMesh.nVBCount = (u8)nVBs;
		m_FPCMesh.aVB = (FDX8VB_t *)ADR_FVB;
		m_FPCMesh.nIBCount = nIndexBufferCount;
		m_FPCMesh.anIndicesCount = (u16 *)ADR_FIBIndexCounts;
		m_FPCMesh.apDXIB = (void **)ADR_FVertIBs;

		// Pack the meshes in
		fang_MemCopy( pExportBuffer + ADR_FMesh, &m_FMesh, sizeof( FMesh_t ) );
		fang_MemCopy( pExportBuffer + ADR_FPCMesh, &m_FPCMesh, sizeof( FDX8Mesh_t ) );
	}


	////////////////////////////////
	// PACK TEXLAYERID DATA INTO BUFFER
	if ( m_pFirstTexLayerID )
	{
		// Get a convenient pointer
		u8 *pTLIDData = pExportBuffer + ADR_FTexLayerID;

		MLTexLayerID *pTexLayerID = (MLTexLayerID *)m_pFirstTexLayerID;
		while ( pTexLayerID )
		{
			FASSERT( pTLIDData < (pExportBuffer + ADR_FTexLayerID + MEM_FTexLayerID) );

			fang_MemCopy( pTLIDData, &pTexLayerID->m_FMeshTexLayerID, sizeof( FMeshTexLayerID_t ) );
			pTLIDData += sizeof( FMeshTexLayerID_t );

			pTexLayerID = pTexLayerID->m_pNext;
		}
	}


	////////////////////////////////
	// PACK LIGHT DATA INTO BUFFER
	if ( nMeshLights )
	{
		// Get a convenient pointer
		u8 *pLightData = pExportBuffer + ADR_FMeshLight;

		MLMeshLight *pMLLight = m_pFirstLight;
		while ( pMLLight )
		{
			FASSERT( pLightData < (pExportBuffer + ADR_FMeshLight + MEM_FMeshLight) );

			if ( pMLLight->m_pszParentBoneName && m_pBoneArray )
			{
				for ( i = 0; i < nBones; i++ )
				{
					if ( stricmp( m_pBoneArray[i].szName, pMLLight->m_pszParentBoneName ) == 0 )
					{
						pMLLight->m_FMeshLight.LightInit.nParentBoneIdx = i;
						break;
					}
				}

				FASSERT ( i != nBones );
			}
			else
			{
				pMLLight->m_FMeshLight.LightInit.nParentBoneIdx = -1;
			}

			fang_MemCopy( pLightData, &pMLLight->m_FMeshLight, sizeof( FMeshLight_t ) );
			pLightData += sizeof( FMeshLight_t );

			pMLLight = pMLLight->m_pNext;
		}
	}


	////////////////////////////////
	// PACK BONE AND SKELETON DATA INTO BUFFER
	if ( m_pBoneArray )
	{
		// Put bone data in
		u8 *pBoneArray = pExportBuffer + ADR_FMeshBone;
		fang_MemCopy( pBoneArray, m_pBoneArray, sizeof( FMeshBone_t ) * nBones );

		// Put skeleton data in
		u8 *pSkeletonArray = pExportBuffer + ADR_FSkeleton;
		fang_MemCopy( pSkeletonArray, m_pSkeletonArray, m_nSkeletonArraySize );
	}


	/////////////////////////////////////////////////////////////
	// CREATE AND PACK THE VERTEX BUFFERS
	{
		FDX8VB_t *pVB = (FDX8VB_t *)(pExportBuffer + ADR_FVB);
		u8 *pDXVB = pExportBuffer + ADR_DXVB;
		u16 nVBCount = 0, nVertices = 0;
		KongVert_t *pKongVert;
		FDX8LightMapST_t *pLightMapUVs = (FDX8LightMapST_t *)(pExportBuffer + ADR_LightMapSTs);
		FDX8BasisVectors_t *pBasisVectors = (FDX8BasisVectors_t *)(pExportBuffer + ADR_BasisVectors);

		for ( i = 0; i < MLManager.m_pPCVertHash->m_nHashTableCount; i++ )
		{
			CHashTable *pTable = MLManager.m_pPCVertHash->m_apHashTables[i];
			if ( !pTable->m_nActiveNodeCount )
			{
				continue;
			}

			FASSERT( (u8 *)pVB < (pExportBuffer + ADR_FVB + MEM_FVB) );

			u32 nBaseSTSets = MLManager.m_pPCVertHash->m_pSampleHashVert[i]->nBaseUVCount;
			u32 nVBKey = fdx8vb_FindFormat( FALSE, 1, (u32)MLManager.m_pPCVertHash->m_pSampleHashVert[i]->nNumWeights, 1, nBaseSTSets );

			pVB->bDynamic = FALSE;
			pVB->bLocked = FALSE;
			pVB->bSoftwareVP = FALSE;
			pVB->hVertexShader = FDX8VB_InfoTable[nVBKey].nFVF;
			pVB->nBytesPerVertex = FDX8VB_InfoTable[nVBKey].nVtxBytes;
			pVB->nInfoIndex = (s8)nVBKey;
			pVB->nLockBytes = 0;
			pVB->nLockOffset = 0;
			pVB->nVtxCount = pTable->m_nActiveNodeCount;
			pVB->pLockBuf = NULL;
			pVB->pDXVB = (IDirect3DVertexBuffer8 *)(pDXVB - pExportBuffer);
			pVB->nLMTCCount = anLightMapSTs[i];
			if ( MLManager.m_pPCVertHash->m_pSampleHashVert[i]->nLightMapUVCount )
			{
				pVB->pLMUVStream = (FDX8LightMapST_t *)((u32)pLightMapUVs - (u32)pExportBuffer);
			}
			else
			{
				pVB->pLMUVStream = NULL;
			}
			pVB->pBasisStream = (FDX8BasisVectors_t *)((u32)pBasisVectors - (u32)pExportBuffer);

			nVBCount++;
			pVB++;

			// Fill in the vertices...
//			CHashNode *pVNode = pTable->m_pFirstActiveNode;
			u32 iNode;
			for ( iNode = 0; iNode < pTable->m_nActiveNodeCount; iNode++ )
//			while ( pVNode )
			{
				CHashNode *pVNode = &pTable->m_paHashNodes[iNode];
				FASSERT( pDXVB < (pExportBuffer + ADR_DXVB + MEM_DXVB) );

				nVertices++;
				fang_MemCopy( pDXVB, &((KongVert_t *)pVNode->m_pData)->Pos, sizeof( CFVec3 ) );
				pDXVB += sizeof( CFVec3 );

				u32 nColor = CONVERT_CFCOLOR_TO_D3D( ((KongVert_t *)pVNode->m_pData)->Color );
				pKongVert = (KongVert_t *)pVNode->m_pData;
				switch ( nVBKey )
				{
					case FDX8VB_TYPE_N1C1T1:
					{
						fang_MemCopy( pDXVB, &pKongVert->Norm, sizeof( CFVec3 ) );
						pDXVB += sizeof( CFVec3 );
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) );
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, pKongVert->aUV, sizeof( CFVec2 ) );
						pDXVB += sizeof( CFVec2 );
						break;
					}

					case FDX8VB_TYPE_N1C1T2:
					{
						fang_MemCopy( pDXVB, &pKongVert->Norm, sizeof( CFVec3 ) );
						pDXVB += sizeof( CFVec3 );
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) );
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, pKongVert->aUV, sizeof( CFVec2 ) * 2 );
						pDXVB += sizeof( CFVec2 ) * 2;
						break;
					}

					case FDX8VB_TYPE_N1W3C1T1:
					{
						u32 nDestWeight, nSourceWeight;
						for ( nDestWeight = 0; nDestWeight < 3; nDestWeight++ )
						{
							fang_MemSet( pDXVB, 0, sizeof( f32 ) );
							for ( nSourceWeight = 0; nSourceWeight < pKongVert->nNumWeights; nSourceWeight++ )
							{
								if ( pKongVert->nWeightBoneIdx[nSourceWeight] == (f32)nDestWeight )
								{
									fang_MemCopy( pDXVB, &pKongVert->fWeight[nSourceWeight], sizeof( f32 ) );
								}
							}
							pDXVB += sizeof( f32 );
						}
						fang_MemCopy( pDXVB, &pKongVert->Norm, sizeof( CFVec3 ) );
						pDXVB += sizeof( CFVec3 );
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) );
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, pKongVert->aUV, sizeof( CFVec2 ) );
						pDXVB += sizeof( CFVec2 );
						break;
					}

					case FDX8VB_TYPE_N1W3C1T2:
					{
						u32 nDestWeight, nSourceWeight;
						for ( nDestWeight = 0; nDestWeight < 3; nDestWeight++ )
						{
							fang_MemSet( pDXVB, 0, sizeof( f32 ) );
							for ( nSourceWeight = 0; nSourceWeight < pKongVert->nNumWeights; nSourceWeight++ )
							{
								if ( pKongVert->nWeightBoneIdx[nSourceWeight] == nDestWeight )
								{
									fang_MemCopy( pDXVB, &pKongVert->fWeight[nSourceWeight], sizeof( f32 ) );
								}
							}
							pDXVB += sizeof( f32 );
						}
						fang_MemCopy( pDXVB, &pKongVert->Norm, sizeof( CFVec3 ) );
						pDXVB += sizeof( CFVec3 );
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) );
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, pKongVert->aUV, sizeof( CFVec2 ) * 2 );
						pDXVB += sizeof( CFVec2 ) * 2;
						break;
					}

					case FDX8VB_TYPE_TL_C2T2:
					{
						fang_MemCopy( pDXVB, &pKongVert->fWeight[0], sizeof( f32 ) );
						pDXVB += sizeof( f32 );
						fang_MemCopy( pDXVB, &pKongVert->fWeight[1], sizeof( f32 ) );
						pDXVB += sizeof( f32 );
						fang_MemCopy( pDXVB, &pKongVert->fWeight[2], sizeof( f32 ) );
						pDXVB += sizeof( f32 );
						fang_MemCopy( pDXVB, &pKongVert->Norm, sizeof( CFVec3 ) );
						pDXVB += sizeof( CFVec3 );
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) );
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) ); // Specular
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, pKongVert->aUV, sizeof( CFVec2 ) * 2 );
						pDXVB += sizeof( CFVec2 ) * 2;
						break;
					}

					case FDX8VB_TYPE_C1:
					{
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) );
						pDXVB += sizeof( u32 );
						break;
					}

					case FDX8VB_TYPE_C1T1:
					{
						fang_MemCopy( pDXVB, &nColor, sizeof( u32 ) ); // Specular
						pDXVB += sizeof( u32 );
						fang_MemCopy( pDXVB, pKongVert->aUV, sizeof( CFVec2 ) );
						pDXVB += sizeof( CFVec2 );
						break;
					}

					default:
					{
						FASSERT_NOW;
						break;
					}
				}

				// Copy over the lightmap STs
				for ( ii = 0; ii < anLightMapSTs[i]; ii++ )
				{
					fang_MemCopy( pLightMapUVs, &pKongVert->aUV[nBaseSTSets + ii], sizeof( FDX8LightMapST_t ) );
					pLightMapUVs++;
				}

				//Copy over the basis vectors.
				if (nBasisVectorCount)
				{
					pBasisVectors->fTx = pKongVert->Tangent.x; pBasisVectors->fTy = pKongVert->Tangent.y; pBasisVectors->fTz = pKongVert->Tangent.z;
					pBasisVectors->fBx = pKongVert->Binorm.x; pBasisVectors->fBy = pKongVert->Binorm.y; pBasisVectors->fBz = pKongVert->Binorm.z;
					pBasisVectors++;
				}

//				pVNode = pVNode->m_pNext;
			}
		}
	}


	/////////////////////////////////////////////////////////////
	// PACK HW MATERIAL DATA INTO BUFFER 
	{
		// Get a convenient pointer
		u8 *pMaterialArray = pExportBuffer + ADR_FMeshMtl;
		u8 *pHWMaterialArray = pExportBuffer + ADR_FMeshHWMtl;
		u8 *pShMotifData = pExportBuffer + ADR_FShMotif;
		u8 *pShTexInstData = pExportBuffer + ADR_FShTexInst;
		u8 *pShRegisterData = pExportBuffer + ADR_FShRegister;
		u8 *pShTexNames = pExportBuffer + ADR_FTexName;
		u8 *pMeshClusters = pExportBuffer + ADR_FMeshClusters;

		MLMaterial_PC *pTempMat = (MLMaterial_PC *)m_pFirstMaterial;
		while ( pTempMat )
		{
			if ( pTempMat->m_pKongMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
			{
				// Skip meshes that are flagged as not visible
				pTempMat = (MLMaterial_PC *)pTempMat->m_pNext;
				continue;
			}

			FASSERT( pTempMat->m_nSurfaceShaderID != -1 );
			FASSERT( pTempMat->m_nLightShaderID != -1 );
			FASSERT( pMaterialArray < (pExportBuffer + ADR_FMeshHWMtl + MEM_FMeshHWMtl) );
			FASSERT( pShMotifData < (pExportBuffer + ADR_FShMotif + MEM_FShMotif) );
			FASSERT( pShTexInstData < (pExportBuffer + ADR_FShTexInst + MEM_FShTexInst) );
			FASSERT( pShRegisterData < (pExportBuffer + ADR_FShRegister + MEM_FShRegister) );
			FASSERT( pShTexNames < (pExportBuffer + ADR_FTexName + MEM_FTexName) );

			// Copy over the surface registers, motif, ShTexInsts
			pTempMat->CopyTextureData( &pShTexInstData, &pShTexNames, pExportBuffer, FALSE );

			// Set a pointer to the current register data entry
			pTempMat->m_FMaterial.pnShSurfaceRegisters = (u32 *)(pShRegisterData - pExportBuffer);

			// Copy over the surface registers, motif, ShTexInsts
			pTempMat->CopySurfaceShaderMSR( (u32 **)&pShRegisterData, &pShMotifData, pExportBuffer, FALSE );

			// Set a pointer to the current register data entry
			pTempMat->m_FMaterial.pnShLightRegisters = (u32 *)(pShRegisterData - pExportBuffer);

			// Copy over the light registers, motif, ShTexInsts
			pTempMat->CopyLightShaderMSR( (u32 **)&pShRegisterData, &pShMotifData, pExportBuffer, FALSE );

			// Set platform data pointer
			pTempMat->m_FMaterial.pPlatformData = (void *)(pHWMaterialArray - pExportBuffer);

			// Copy in the actual material
			fang_MemCopy( pMaterialArray, &pTempMat->m_FMaterial, sizeof( FMeshMaterial_t ) );

			// Increment data pointer and index
			pMaterialArray += sizeof( FMeshMaterial_t );

			// We have one cluster per display list
			pTempMat->m_FPCMaterial.aCluster = (FDX8MeshCluster_t *)(pMeshClusters - pExportBuffer);
			pTempMat->m_FPCMaterial.nClusterCount = pTempMat->m_nDLContainerCount;
			pMeshClusters += pTempMat->m_nDLContainerCount * sizeof( FDX8MeshCluster_t );

			// Copy in the actual material
			fang_MemCopy( pHWMaterialArray, &pTempMat->m_FPCMaterial, sizeof( FDX8MeshMaterial_t ) );

			// Increment data pointer and index
			pHWMaterialArray += sizeof( FDX8MeshMaterial_t );

			pTempMat = (MLMaterial_PC *)pTempMat->m_pNext;
		}
	}
	

	/////////////////////////////////////////////////////////////
	// CREATE AND PACK THE CLUSTER DATA
	{
		// Get convenient pointers
		u16 nStripCounter = 0;
		u8 *pClusters = pExportBuffer + ADR_FMeshClusters;
		FDX8MeshStrip_t *pStrips = (FDX8MeshStrip_t *)(pExportBuffer + ADR_FMeshStrips);
		u16 *anIBIndexCounts = (u16 *)(pExportBuffer + ADR_FIBIndexCounts);
		u16 **apIndexBuffer = (u16 **)(pExportBuffer + ADR_FVertIBs);
		u16 *pVertIndices = (u16 *)(pExportBuffer + ADR_FVertIndices);
		u16 nCurrentVertIndex = 0;
		u16 nCurrentIBIndex = 0;

		apIndexBuffer[nCurrentIBIndex] = (u16 *)((u32)pVertIndices - (u32)pExportBuffer);
		anIBIndexCounts[nCurrentIBIndex] = 0;

		MLMaterial_PC *pTempMat = (MLMaterial_PC *)m_pFirstMaterial;
		while ( pTempMat )
		{
			if ( pTempMat->m_pKongMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
			{
				// Skip meshes that are flagged as not visible
				pTempMat = (MLMaterial_PC *)pTempMat->m_pNext;
				continue;
			}

			MLDLContainer *pDLCont = pTempMat->m_pFirstDLContainer;
			while ( pDLCont )
			{
				FASSERT( pDLCont->pTris->m_nSegmentIdx != -1 );
//				FASSERT( nFinalVBIndex[pDLCont->pTris->m_nVBKey] != -1 );

				// Make sure we don't exceed the 16-bit limit of the vertex IB
				if ( anIBIndexCounts[nCurrentIBIndex] + pDLCont->nTotalVertIndices > 65535 )
				{
					// Move the index buffer base pointer to the end of the current index list
					pVertIndices += nCurrentVertIndex;
					nCurrentVertIndex = 0;
					nCurrentIBIndex++;
					apIndexBuffer[nCurrentIBIndex] = (u16 *)((u32)pVertIndices - (u32)pExportBuffer);
					anIBIndexCounts[nCurrentIBIndex] = 0;
				}

				anIBIndexCounts[nCurrentIBIndex] += (u16)pDLCont->nTotalVertIndices;

				// Create a cluster for this display list
				FDX8MeshCluster_t Cluster;
				Cluster.nFlags = 0;
				Cluster.nSegmentIdx = (u8)pDLCont->pTris->m_nSegmentIdx;
				Cluster.nStripCount = 0;
				Cluster.pPushBuffer = NULL;
				Cluster.paStripBuffer = (FDX8MeshStrip_t *)((u32)pStrips - (u32)pExportBuffer);
				Cluster.nVBIndex = (u8)pDLCont->pTris->m_nHashTableIdx;
				Cluster.nIBIndex = (u8)nCurrentIBIndex;
				Cluster.TriList.nStartVindex = nCurrentVertIndex;
				Cluster.nPartID = (u8)pDLCont->nPartID;
				Cluster.nLODID = (u8)pDLCont->nLODID;

				if ( !pDLCont->bFacingDirLight )
				{
					Cluster.nFlags |= CLUSTER_FACING_OPP_DIR_LIGHT;
				}

				u16 nMinVertIdx = 0xffff;
				u16 nMaxVertIdx = 0;
				u16 nGroup;

				// First, advance through all of the primitive groups creating the appropriate tri LIST data
				for ( nGroup = 0; nGroup < pDLCont->nPrimGroupCount; nGroup++ )
				{
					if ( pDLCont->paPrimGroups[nGroup].type != PT_LIST )
					{
						continue;
					}

					// Add this group to the tri list:
					PCVertAbstr_t *pAbstr = (PCVertAbstr_t *)pDLCont->pTris->m_pVertAbstr;
					u32 nNumIndices = pDLCont->paPrimGroups[nGroup].numIndices;
					u16 *paIndices = pDLCont->paPrimGroups[nGroup].indices;
					for ( i = 0; i < nNumIndices; i++ )
					{
						FASSERT( nCurrentVertIndex < 65536 );

						u16 nIdx = pAbstr[paIndices[i]].nVBIdx;
						pVertIndices[nCurrentVertIndex++] = nIdx;
						if ( nIdx < nMinVertIdx )
						{
							nMinVertIdx = nIdx;
						}
						if ( nIdx > nMaxVertIdx )
						{
							nMaxVertIdx = nIdx;
						}
					}
				}

				if ( nMinVertIdx == 0xffff && nMaxVertIdx == 0 )
				{
					// There are no tris in the tri list
					Cluster.TriList.nStartVindex = 0;
					Cluster.TriList.nVtxIndexMin = 0;
					Cluster.TriList.nVtxIndexRange = 0;
					Cluster.TriList.nTriCount = 0;
				}
				else
				{
					Cluster.TriList.nVtxIndexMin = nMinVertIdx;
					Cluster.TriList.nVtxIndexRange = (nMaxVertIdx - nMinVertIdx) + 1;
					Cluster.TriList.nTriCount = (nCurrentVertIndex - Cluster.TriList.nStartVindex) / 3;
				}

				// Second, advance through all of the draw links creating the appropriate tri STRIP data
				for ( nGroup = 0; nGroup < pDLCont->nPrimGroupCount; nGroup++ )
				{
					if ( pDLCont->paPrimGroups[nGroup].type != PT_STRIP )
					{
						continue;
					}

					FASSERT( nStripCounter != nTotalStripCount );
					FASSERT( pDLCont->paPrimGroups[nGroup].numIndices - 2 < 256 );
					FASSERT( (u8 *)pStrips < (pExportBuffer + ADR_FMeshStrips + MEM_FMeshStrips) );

					// Add this primitive group to the strips:
					pStrips->nStartVindex = nCurrentVertIndex;
					pStrips->nTriCount = (u8)pDLCont->paPrimGroups[nGroup].numIndices - 2;

					PCVertAbstr_t *pAbstr = (PCVertAbstr_t *)pDLCont->pTris->m_pVertAbstr;
					u32 nNumIndices = pDLCont->paPrimGroups[nGroup].numIndices;
					u16 *paIndices = pDLCont->paPrimGroups[nGroup].indices;
					for ( i = 0; i < nNumIndices; i++ )
					{
						FASSERT( nCurrentVertIndex < 65536 );

						u16 nIdx = pAbstr[paIndices[i]].nVBIdx;
						pVertIndices[nCurrentVertIndex++] = nIdx;
						if ( nIdx < nMinVertIdx )
						{
							nMinVertIdx = nIdx;
						}
						if ( nIdx > nMaxVertIdx )
						{
							nMaxVertIdx = nIdx;
						}
					}

					pStrips->nVtxIndexMin = nMinVertIdx;
					pStrips->nVtxIndexRange = (nMaxVertIdx - nMinVertIdx) + 1;

					// Increment to the next strip
					Cluster.nStripCount++;
					nStripCounter++;
					pStrips++;
				}

				FASSERT( pClusters < (pExportBuffer + ADR_FMeshClusters + MEM_FMeshClusters ) );

				// Pack the cluster
				fang_MemCopy( pClusters, &Cluster, sizeof( FDX8MeshCluster_t ) );
				pClusters += sizeof( FDX8MeshCluster_t );

				// Move on to the next display list
				pDLCont = pDLCont->pNextDLContainer;
			}

			pTempMat = (MLMaterial_PC *)pTempMat->m_pNext;
		}

		FASSERT( nStripCounter == nTotalStripCount );
	}
	

	/////////////////////////////////////////////////////////////
	// PACK SEGMENT DATA INTO BUFFER
	{
		// Put the cross-platform segments in
		u8 *pSegmentArray = pExportBuffer + ADR_FMeshSeg;
		MLSegment *pTempSegment = m_pFirstSegment;
		while ( pTempSegment )
		{
			FASSERT( pSegmentArray < (pExportBuffer + ADR_FMeshSeg + MEM_FMeshSeg) );

			fang_MemCopy( pSegmentArray, &pTempSegment->m_FMeshSeg, sizeof( FMeshSeg_t ) );

			// Increment data pointer
			pSegmentArray += sizeof( FMeshSeg_t );

			pTempSegment = pTempSegment->m_pNext;
		}
	}
  
	/////////////////////////////////////////////////////////////
	// PACK COLLISION DATA INTO BUFFER
	if ( m_paCollTree )
	{
		FkDOP_Tree_t *pCollArray = (FkDOP_Tree_t *)(pExportBuffer + ADR_FCollTree);
		FkDOP_Node_t *pNodeArray = (FkDOP_Node_t *)(pExportBuffer + ADR_FkDOPNodes);
		FkDOP_TriPacket_t *pPacketArray = (FkDOP_TriPacket_t *)(pExportBuffer + ADR_FTriPackets);
		FkDOP_Interval_t *pIntervalArray = (FkDOP_Interval_t *)(pExportBuffer + ADR_FIntervals);
		u8 *pTriData = (u8 *)(pExportBuffer + ADR_FTriData);
		CFVec3 *pRootDOPVertData = (CFVec3 *)(pExportBuffer + ADR_FRootDOPVertData);

		for ( i = 0; i < m_nCollTreeCount; i++ )
		{
			memcpy( pCollArray, &m_paCollTree[i], sizeof( FkDOP_Tree_t ) );

			FkDOP_Node_t *pNodeStart = pNodeArray;

			for ( ii = 0; ii < pCollArray->nTreeNodeCount; ii++ )
			{
				memcpy( pNodeArray, &pCollArray->pakDOPNodes[ii], sizeof( FkDOP_Node_t ) );

				// If this is a leaf node, we need to pack away the geometry data
				if ( pNodeArray->paPackets )
				{
					FkDOP_TriPacket_t *pPacketStart = pPacketArray;

					// Change the endian of and pack the packets:
					for ( iii = 0; iii < pNodeArray->nTriPacketCount; iii++ )
					{
						memcpy( pPacketArray, &pNodeArray->paPackets[iii], sizeof( FkDOP_TriPacket_t ) );
						pPacketArray++;
					}

					// Record the packet offset
					pNodeArray->paPackets = (FkDOP_TriPacket_t *)((u32)pPacketStart - (u32)pExportBuffer);

					// Calculate the size of the tri data
					u32 nTriVertDataSize, nFaceNormalDataSize;
					nTriVertDataSize = RoundUp4B( pNodeArray->nTriCount * sizeof(u16) * 3);
					nFaceNormalDataSize = RoundUp8B( pNodeArray->nTriCount * sizeof( FkDOP_Normal_t ) );

					// Copy the tri data
					memcpy( pTriData, pNodeArray->pTriData, nTriVertDataSize + nFaceNormalDataSize );
					pNodeArray->pTriData = (void *)((u32)pTriData - (u32)pExportBuffer);

					pTriData += nTriVertDataSize + nFaceNormalDataSize;
				}

				pNodeArray++;
			}

			if ( pCollArray->nTreeNodeCount )
				pCollArray->pakDOPNodes = (FkDOP_Node_t *)((u32)pNodeStart - (u32)pExportBuffer);
			else
				pCollArray->pakDOPNodes = NULL;

			// Pack the intervals
			u32 nIntervalCount = FkDOP_aDesc[m_nCollkDOPType].nNormalCount * pCollArray->nTreeNodeCount;
			memcpy( pIntervalArray, pCollArray->paIntervals, nIntervalCount * sizeof( FkDOP_Interval_t ) );
			pCollArray->paIntervals = (FkDOP_Interval_t *)((u32)pIntervalArray - (u32)pExportBuffer);
			pIntervalArray += nIntervalCount;

			// Pack the Root kDOP vert data
			if ( pCollArray->nRootkDOPVertCount )
			{
				memcpy( pRootDOPVertData, pCollArray->paRootkDOPVerts, pCollArray->nRootkDOPVertCount * sizeof( CFVec3 ) );
				pCollArray->paRootkDOPVerts = (CFVec3 *)((u32)pRootDOPVertData - (u32)pExportBuffer);
				pRootDOPVertData += pCollArray->nRootkDOPVertCount;
			}
			else
			{
				pCollArray->paRootkDOPVerts = NULL;
			}

			// Change the endian for the CollInfo and progress to the next
			pCollArray++;
		}

	}
	m_Results.fDataGeneration = (f32)(timeGetTime() - nProcessStartTime) * 0.001f;

	/////////////////////////
	// SUCCESSFULLY COMPLETED
	/////////////////////////

	FreeScratchMemory();

	if ( m_paVertRemaps )
	{
		WriteVertRemapFile( nVBs );
	}

	// Fill out results info:
	m_Results.fTotalTime = (f32)(timeGetTime() - nStartTime) * 0.001f;

	m_Results.nExportDataSize = nExportSize;

	m_Results.nPos32Count = nTotalVertCount;

	m_Results.nBoneCount = nBones;
	m_Results.nSegmentCount = nSegments;
	m_Results.nMaterialCount = nMaterials;
	m_Results.nFShTexInstCount = nMaterials;
	m_Results.nTexLyrIDCount = nTexLayerIDs;
	m_Results.nMeshLightCount = nMeshLights;
	m_Results.nVBCount = nVBs;

	m_Results.nFMeshMem = MEM_FMesh;
	m_Results.nFPlatMeshMem = MEM_FPCMesh;
	m_Results.nBoneMem = MEM_FMeshBone;
	m_Results.nSkeletonMem = MEM_FSkeleton;
	m_Results.nSegmentMem = MEM_FMeshSeg;
	m_Results.nShaderMem = MEM_FShMotif + MEM_FShRegister + MEM_FShTexInst;
	m_Results.nMaterialMem = MEM_FMeshHWMtl + ADR_FTexName;
	m_Results.nTexLyrIDMem = MEM_FTexLayerID;
	m_Results.nMeshLightMem = MEM_FMeshLight;
	m_Results.nVBMem = MEM_FVB;

	m_Results.nOrigVertAttrMem =  (m_Results.nOrigPosCount * 4 * 3) 
								+ (m_Results.nOrigNormalCount * 4 * 3)
								+ (m_Results.nOrigSTCount * 4 * 2)
								+ (m_Results.nOrigColorCount * 4)
								+ (m_Results.nOrigWeightCount * 4 * 4);

//	DEVPRINTF( "Total MeshLib Time: %5.3f - Compression: %5.3f - Collision: %5.3f - Display Lists: %5.3f - Export Data: %5.3f\n", m_Results.fTotalTime, m_Results.fCompressTime, m_Results.fCollisionTime, m_Results.fDLTime, m_Results.fDataGeneration );
	MLManager.m_Results.fTotalTime += m_Results.fTotalTime;
	MLManager.m_Results.fCompressTime += m_Results.fCompressTime;
	MLManager.m_Results.fCollisionTime += m_Results.fCollisionTime;
	MLManager.m_Results.fDLTime += m_Results.fDLTime;
	MLManager.m_Results.fDataGeneration += m_Results.fDataGeneration;

	MLManager.m_pDlg->WriteToLog( "ML - Export data successfully generated.\n" );

	return &m_Results;
}


//
//	Divides the verts by PC vertex buffer type
//
u32 MLMesh_PC::DivideVertsByVB( void )
{
	u32 i;
	u32 uMultipleAddCount = 0;
	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		MLTriContainer *pTempTriCont = pTempSegment->m_pFirstTriContainer;
		while ( pTempTriCont )
		{
//			m_nTotalVertIdxCount += pTempTriCont->m_nVertCount;

			m_Results.nOrigPosCount += pTempTriCont->m_nVertCount;
			m_Results.nOrigNormalCount += pTempTriCont->m_nVertCount;
			m_Results.nOrigSTCount += pTempTriCont->m_nVertCount * (pTempTriCont->m_nBaseSTSets + pTempTriCont->m_nLightMapSTSets);
			m_Results.nOrigColorCount += pTempTriCont->m_nVertCount;

			FASSERT( pTempTriCont->m_nVBKey >= 0 && pTempTriCont->m_nVBKey < FDX8VB_TYPE_COUNT );
			FASSERT( MLManager.m_pPCVertHash );

//			if ( pTempTriCont->m_bStrip )
//			{
//				m_nStripCount++;
//			}

			// Allocate memory for the vertex abstractions
			FASSERT( !pTempTriCont->m_pVertAbstr );
			pTempTriCont->m_pVertAbstr = (PCVertAbstr_t *)fang_Malloc( pTempTriCont->m_nVertCount * sizeof( PCVertAbstr_t ), 4 );
			if ( !pTempTriCont->m_pVertAbstr )
			{
				return ML_OUT_OF_MEMORY;
			}

			// Cycle through all of the verts adding them to the appropriate collection
			PCVertAbstr_t *pAbstraction = (PCVertAbstr_t *)pTempTriCont->m_pVertAbstr;
			for ( i = 0; i < pTempTriCont->m_nVertCount; i++ )
			{
				u16 nVBIndex;
				pAbstraction[i].nVBIdx = MLManager.m_pPCVertHash->AddVert( &pTempTriCont->m_paKongVerts[i], nVBIndex );

				if ( pAbstraction[i].nVBIdx == 0xffff )
				{
					return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
				}

				if ( m_paVertRemaps )
				{
					if (    (m_paVertRemaps[pTempTriCont->m_paKongVerts[i].nMeshVertIdx].nVBIndex != 0xffff 
							&& m_paVertRemaps[pTempTriCont->m_paKongVerts[i].nMeshVertIdx].nVBIndex != nVBIndex)
							|| (m_paVertRemaps[pTempTriCont->m_paKongVerts[i].nMeshVertIdx].nVertColorIndex != 0xffff 
							&& m_paVertRemaps[pTempTriCont->m_paKongVerts[i].nMeshVertIdx].nVertColorIndex != pAbstraction[i].nVBIdx) )
					{
						DEVPRINTF( "MLMesh_PC::DivideVertsByVB() - Vert %d added to VB already!\n", pTempTriCont->m_paKongVerts[i].nMeshVertIdx );
						uMultipleAddCount++;
					}

					m_paVertRemaps[pTempTriCont->m_paKongVerts[i].nMeshVertIdx].nVertColorIndex = pAbstraction[i].nVBIdx;
					m_paVertRemaps[pTempTriCont->m_paKongVerts[i].nMeshVertIdx].nVBIndex = nVBIndex;
				}
			}

			pTempTriCont->m_nHashTableIdx = MLManager.m_pPCVertHash->GetTableIdx( pTempTriCont->m_paKongVerts );
			FASSERT( pTempTriCont->m_nHashTableIdx > -1 );

			pTempTriCont = pTempTriCont->m_pNext;
		}

		pTempSegment = pTempSegment->m_pNext;
	}

	if ( uMultipleAddCount )
	{
		DEVPRINTF( "MLMesh_PC::DivideVertsByVB() - %d verts already added when assigned to remap.\n", uMultipleAddCount );
	}

	return ML_NO_ERROR;
}


