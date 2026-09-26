//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_GC.cpp - Classes used to convert generic mesh data into Fang GC specific data
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
//////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "stdafx.h"
#include <mmsystem.h>

// Fang Includes
#include "gc\fGCDisplayList.h"
#include "FkDOP.h"

// MeshLib Includes
#include "MLMesh_GC.h"
#include "MLHash.h"
#include "MLMaterial_GC.h"
//#include "GCMatrixStack.h"

// From NVidia stripping library
#include "NvTriStrip.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

// Conversion counters for GC material rendering
u32 GCMesh_nTotalBumpedVertsRendered = 0;
u32 GCMesh_nVertsRendered;
u32 GCMesh_nDLChanges;
u32 GCMesh_nGDBeginCalls;


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

// Private scratch data used for conversion to visual geometry
static u32				_nPosNormCount;
//static u32				_nPosNBTCount;
static FGCSkinPosNorm_t	_PosNorm[MLMESH_MAX_GC_ATTRIBUTE_INDICES];
//static FGCBumpVert_t	_PosNBT[MLMESH_MAX_GC_ATTRIBUTE_INDICES];

static u32				_nWeightsCount;
static FGCWeights_t		_Weights[MLMESH_MAX_GC_ATTRIBUTE_INDICES];

static u32				_nTransDescCount;
static FGCTransDesc_t	_TransDesc[MLMESH_MAX_GC_TRANS_DESC];

static u32				_nTransDesc1Matrix;
static u32				_nTransDesc2Matrix;
static u32				_nTransDesc3or4Matrix;

static u32				_nMatrixSwitches;
static u32				_nMatrixLoads;


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
MLResults* MLMesh_GC::GenerateExportData( BOOL bGenerateCollisionData )
{
	u32 nProcessStartTime;
	u32 nStartTime = timeGetTime();

	u32 i, ii, iii;

	// Reset rendering vars
	GCMesh_nVertsRendered = 0;
	GCMesh_nDLChanges = 0;
	GCMesh_nGDBeginCalls = 0;
	_nMatrixSwitches = 0;
	_nMatrixLoads = 0;

	// Reset the scrap space
	_nPosNormCount = 0;
	_nWeightsCount = 0;
	_nTransDescCount = 0;
	_nTransDesc1Matrix = 0;
	_nTransDesc2Matrix = 0;
	_nTransDesc3or4Matrix = 0;

	// Reset Collision Counters
	MLMesh_nTotalNodeCount = 0;
	MLMesh_nTotalIntervalCount = 0;
	MLMesh_nTotalTriCount = 0;
	MLMesh_nTotalTriPackets = 0;
	MLMesh_nTotalTriDataBytes = 0;
	MLMesh_nTotalRootDOPVertBytes = 0;
	MLMesh_nTotalkDOPFaceCount = 0;

	memset( &m_Results, 0, sizeof( MLResults ) );

	// Determine the number of things to store in the export buffer
	u32 nSegments		= m_FMesh.nSegCount;
	u32 nTexLayerIDs	= m_FMesh.nTexLayerIDCount;
	u32 nMeshLights		= m_FMesh.nLightCount;
	u32 nBones			= m_FMesh.nBoneCount;
	u8  nUseBasis		= 0;

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

	// Cycle through the segments transforming the verts and quantizing
	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		pTempSegment->ProcessVerts( m_pBoneArray );
		pTempSegment->Quantize( 10 - m_nCompressionBias, m_FMesh.nUsedBoneCount );
		pTempSegment = pTempSegment->m_pNext;
	}

	// Calculate the number of display lists required
	nProcessStartTime = timeGetTime();
	u32 nDLConts = 0;
	m_Results.nErrorCode = ResolveMaterials( &nDLConts );
	if ( m_Results.nErrorCode )
	{
		return &m_Results;
	}
	m_Results.fDLTime = (f32)(timeGetTime() - nProcessStartTime) * 0.001f;

	// Calculate the number of and create the vertex buffers
	u32 nVBs = 0;
	m_Results.nErrorCode = CreateVertexBuffers( &nVBs );
	if ( m_Results.nErrorCode )
	{
		return &m_Results;
	}

	// Initialize the stripper
	InitStripper();

	nProcessStartTime = timeGetTime();
	// Gather info from the materials
	u32 nMaterials = 0;
	u32 nShRegisterCount = 0;
	u32 nShMotifCount = 0;
	u32 nShTexInstCount = 0;
	MLMaterial_GC *pTempMat = (MLMaterial_GC *)m_pFirstMaterial;
	while ( pTempMat )
	{
		if ( pTempMat->m_pKongMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
		{
			// Skip meshes that are flagged as not visible
			pTempMat = (MLMaterial_GC *)pTempMat->m_pNext;
			continue;
		}

		u32 nStrippingTime = timeGetTime();
		pTempMat->BuildPrimitiveGroups( 5, 64, (TRUE & MLManager.m_bGenerateMeshStrips) );
		m_Results.fStrippingTime += (timeGetTime() - nStrippingTime) / 1000.f;
		m_Results.nStripCount += pTempMat->m_nStripCount;
		m_Results.nStripTriCount += pTempMat->m_nStripTriCount;
		m_Results.nListTriCount += pTempMat->m_nListTriCount;

		// Generate bounding sphere
		pTempMat->CalculateBoundingSphere( &m_FMesh.BoundSphere_MS );

		// Calculate required shader registers, motifs and texture layers
		pTempMat->AddShaderMSRCount( &nShMotifCount, &nShTexInstCount, &nShRegisterCount );

		nMaterials++;
		pTempMat = (MLMaterial_GC *)pTempMat->m_pNext;
	}

	// Uninitialize the stripper
	UninitStripper();

	// Create the display lists
	m_Results.nErrorCode = CreateDLContainers( nDLConts );
	if ( m_Results.nErrorCode )
	{
		return &m_Results;
	}
	m_Results.fDLTime += (f32)(timeGetTime() - nProcessStartTime) * 0.001f;

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

	nProcessStartTime = timeGetTime();
	// Determine the relative addresses (offsets) of the data in the export buffer (relative to the start)
	u32 ADR_Current			=	0;

	u32 ADR_FMesh			=	RoundUp4B( ADR_Current );
	u32 MEM_FMesh			=	sizeof(FMesh_t);
	ADR_Current				=	ADR_FMesh + MEM_FMesh;

	u32 ADR_FGCMesh			=	RoundUp4B( ADR_Current );
	u32 MEM_FGCMesh			=	sizeof(FGCMesh_t);
	ADR_Current				=	ADR_FGCMesh + MEM_FGCMesh;

	u32 ADR_FTexName		=	RoundUp4B( ADR_Current );
	u32 MEM_FTexName		=	(nShTexInstCount * (FDATA_TEXNAME_LEN + 1));
	ADR_Current				=	ADR_FTexName + MEM_FTexName;
	if ( MEM_FTexName == 0 )
		ADR_FTexName = 0;

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

	u32 ADR_FGCMeshSkin = 0, MEM_FGCMeshSkin = 0;
	u32 ADR_FGCTransDesc = 0, MEM_FGCTransDesc = 0;
	if ( _nTransDescCount )
	{
		ADR_FGCMeshSkin				=	RoundUp4B( ADR_Current );
		MEM_FGCMeshSkin				=	sizeof(FGCMeshSkin_t);
		ADR_Current					=	ADR_FGCMeshSkin + MEM_FGCMeshSkin;

		ADR_FGCTransDesc			=	RoundUp4B( ADR_Current );
		MEM_FGCTransDesc			=	_nTransDescCount * sizeof(FGCTransDesc_t);
		ADR_Current					=	ADR_FGCTransDesc + MEM_FGCTransDesc;
	}

		// Material addresses:
	u32 ADR_FMeshMtl		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FMeshMtl		=	nMaterials * sizeof(FMeshMaterial_t);
	ADR_Current				=	ADR_FMeshMtl + MEM_FMeshMtl;
	if ( MEM_FMeshMtl == 0 )
		ADR_FMeshMtl = 0;

	u32 ADR_FMeshHWMtl		=	RoundUp4B( ADR_Current );  // THIS COULD BE SHARED DATA
	u32 MEM_FMeshHWMtl		=	nMaterials * sizeof(FGCMeshMaterial_t);
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

		// Vertex Buffers
	u32 ADR_FVB				=	RoundUp32B( ADR_Current );
	u32 MEM_FVB				=	nVBs * sizeof(FGCVB_t);
	ADR_Current				=	ADR_FVB + MEM_FVB;

		// Attribute data for the vertex buffers
	u32 ADR_FPosNorm	=	RoundUp8B( ADR_Current );
	u32 MEM_FPosNorm	=	(_nPosNormCount * sizeof(FGCSkinPosNorm_t));
	ADR_Current			=	ADR_FPosNorm + MEM_FPosNorm;

	u32 ADR_FWeights	=	RoundUp8B( ADR_Current );
	u32 MEM_FWeights	=	(_nWeightsCount * sizeof(FGCWeights_t));
	ADR_Current			=	ADR_FWeights + MEM_FWeights;

	u32 ADR_FNBT		=   RoundUp8B( ADR_Current );
	u32 MEM_FNBT		=   (MLManager.m_pGCVertHash->m_nNBT8Count * sizeof(FGCNBT8_t));
	ADR_Current			=   ADR_FNBT + MEM_FNBT;
	if ( MEM_FNBT == 0 )
		ADR_FNBT = 0;

	u32 ADR_FPosF32		=	RoundUp32B( ADR_Current );
	u32 MEM_FPosF32		=	(MLManager.m_pGCVertHash->m_nPosF32Count * sizeof(FGCPosF32_t));
	ADR_Current			=	ADR_FPosF32 + MEM_FPosF32;

	u32 ADR_FPosS16		=	RoundUp32B( ADR_Current );
	u32 MEM_FPosS16		=	(MLManager.m_pGCVertHash->m_nPosS16Count * sizeof(FGCPosS16_t));
	ADR_Current			=	ADR_FPosS16 + MEM_FPosS16;

	u32 ADR_FPosS8		=	RoundUp32B( ADR_Current );
	u32 MEM_FPosS8		=	(MLManager.m_pGCVertHash->m_nPosS8Count * sizeof(FGCPosS8_t));
	ADR_Current			=	ADR_FPosS8 + MEM_FPosS8;

	u32 ADR_FColor		=	RoundUp32B( ADR_Current );
	u32 MEM_FColor		=	(MLManager.m_pGCVertHash->m_nColorCount * sizeof(FGCColor_t));
	ADR_Current			=	ADR_FColor + MEM_FColor;

	u32 ADR_FST16		=	RoundUp32B( ADR_Current );
	u32 MEM_FST16		=	(MLManager.m_pGCVertHash->m_nST16Count * sizeof(FGCST16_t));
	ADR_Current			=	ADR_FST16 + MEM_FST16;

	m_Results.nVertAttrMem = ADR_Current - ADR_FPosNorm;

		// Display list data for rendering

	u32 ADR_FDLCont		=	RoundUp32B( ADR_Current );
	u32 MEM_FDLCont		=	(nDLConts * sizeof(FGC_DLCont_t));
	ADR_Current			=	ADR_FDLCont + MEM_FDLCont;

	u32 ADR_FDLData;
	u32 MEM_FDLData;
	if ( !MLManager.m_bAccumStreamingData )
	{
		ADR_FDLData		=	RoundUp32B( ADR_Current );
		MEM_FDLData		=	(m_nDLDataSize);
		ADR_Current		=	ADR_FDLData + MEM_FDLData;

		m_Results.nDLMem = ADR_Current - ADR_FDLCont;
	}

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

		// Determine the size of the export buffer
	u32 nExportSize		=	RoundUp4B( ADR_Current );
/*
	DEVPRINTF( "\nTree:		 %d\n", MEM_FCollTree );
	DEVPRINTF( "Nodes:		 %d\n", MEM_FkDOPNodes );
	DEVPRINTF( "Intervals:	 %d\n", MEM_FIntervals );
	DEVPRINTF( "TriPackets:	 %d\n", MEM_FTriPackets );
	DEVPRINTF( "TriData:	 %d\n", MEM_FTriData );
	DEVPRINTF( "Dop Verts:	 %d\n", MEM_FDOPVertData );
	DEVPRINTF( "Root Verts:	 %d\n", MEM_FRootDOPVertData );
*/
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

	u8 *pAccumDataBase = NULL;
	if ( MLManager.m_bAccumStreamingData )
	{
		u32 ADR_AccumCurrent = 0;
		ADR_FDLData			=	RoundUp32B( ADR_AccumCurrent );
		MEM_FDLData			=	(m_nDLDataSize);
		ADR_AccumCurrent	=	ADR_FDLData + MEM_FDLData;

		m_Results.nDLMem = ADR_Current - ADR_FDLCont;

		ADR_AccumCurrent	=	RoundUp4B(ADR_AccumCurrent);

		u8 *pAccumDataBuffer = NULL;
		MLManager.AppendToStreamingData( ADR_AccumCurrent, (void **)&pAccumDataBuffer, (void **)&pAccumDataBase );

		// Fixup to reflect the offset from the base
		u32 nOffset = (u32)pAccumDataBuffer - (u32)pAccumDataBase;
		ADR_FDLData += nOffset;
	}


	////////////////////////////////
	// PACK MESH DATA INTO BUFFER
	{
		// Fill in FMesh data
		m_FMesh.nSegCount = (u8)nSegments;
		m_FMesh.aSeg = (FMeshSeg_t *)ADR_FMeshSeg;
		m_FMesh.pBoneArray = (FMeshBone_t *)ADR_FMeshBone;
		m_FMesh.pLightArray = (FMeshLight_t *)ADR_FMeshLight;
		m_FMesh.pnSkeletonIndexArray = (u8 *)ADR_FSkeleton;
		m_FMesh.nMaterialCount = (u8)nMaterials;
		m_FMesh.aMtl = (FMeshMaterial_t *)ADR_FMeshMtl;
		m_FMesh.pTexLayerIDArray = (FMeshTexLayerID_t *)ADR_FTexLayerID;
		m_FMesh.nCollTreeCount = (u8)MLMesh_nTotalTreeCount;
		m_FMesh.nMeshCollMask = 0;
		for ( i = 0; i < MLMesh_nTotalTreeCount; i++ )
		{
			m_FMesh.nMeshCollMask |= m_paCollTree[i].nMasterCollMask;
		}
		m_FMesh.pMeshIS = (FDX8Mesh_s *)ADR_FGCMesh; // This is cast as a DX8 mesh * because Fang is compiled in PC mode
		m_FMesh.paCollTree = (FkDOP_Tree_t *)ADR_FCollTree;

		// Fill in m_FGCMesh data
		m_FGCMesh.pMesh = (FMesh_t *)ADR_FMesh;
		m_FGCMesh.nVBCount = (u8)nVBs;
		m_FGCMesh.aVB = (FGCVB_t *)ADR_FVB;
		m_FGCMesh.pMeshSkin = (FGCMeshSkin_t *)ADR_FGCMeshSkin;

		m_FMesh.ChangeEndian();
		memcpy( pExportBuffer + ADR_FMesh, &m_FMesh, sizeof( FMesh_t ) );

		m_FGCMesh.ChangeEndian();
		memcpy( pExportBuffer + ADR_FGCMesh, &m_FGCMesh, sizeof( FGCMesh_t ) );
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

			pTexLayerID->m_FMeshTexLayerID.ChangeEndian();
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

			if ( pMLLight->m_pszParentBoneName )
			{
				for ( i = 0; i < nBones; i++ )
				{
					if ( stricmp( m_pBoneArray[i].szName, pMLLight->m_pszParentBoneName ) == 0 )
					{
						pMLLight->m_FMeshLight.LightInit.nParentBoneIdx = i;
						break;
					}
				}
			}
			else
			{
				pMLLight->m_FMeshLight.LightInit.nParentBoneIdx = -1;
			}

			FASSERT ( i != nBones );

			pMLLight->m_FMeshLight.LightInit.ChangeEndian();
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
		for ( i = 0; i < nBones; i++, pBoneArray += sizeof( FMeshBone_t ) )
		{
			// Endian Fix
			m_pBoneArray[i].ChangeEndian();

			// Copy the data to the storage space
			memcpy( pBoneArray, &m_pBoneArray[i], sizeof( FMeshBone_t ) );
		}

		// Put skeleton data in
		u8 *pSkeletonArray = pExportBuffer + ADR_FSkeleton;
		memcpy( pSkeletonArray, m_pSkeletonArray, m_nSkeletonArraySize );
	}


	////////////////////////////////
	// PACK SKINNED VERT DATA
	if ( ADR_FGCMeshSkin )
	{
		// Pack the Mesh skin data
		FGCMeshSkin_t FMeshSkin;
		FASSERT( _nTransDescCount == (_nTransDesc1Matrix + _nTransDesc2Matrix + _nTransDesc3or4Matrix) );
		FMeshSkin.nTransDescCount = (u16)_nTransDescCount;
		FMeshSkin.nTD1MtxCount = (u16)_nTransDesc1Matrix;
		FMeshSkin.nTD2MtxCount = (u16)_nTransDesc2Matrix;
		FMeshSkin.nTD3or4MtxCount = (u16)_nTransDesc3or4Matrix;
		FMeshSkin.pTransDesc = (FGCTransDesc_s *)ADR_FGCTransDesc;
		FMeshSkin.nSkinnedVerts = _nPosNormCount;
		FMeshSkin.pSkinnedVerts = (FGCSkinPosNorm_t *)ADR_FPosNorm;
		FMeshSkin.pSkinWeights = (FGCWeights_s *)ADR_FWeights;
		FMeshSkin.ChangeEndian();
		memcpy( pExportBuffer + ADR_FGCMeshSkin, &FMeshSkin, sizeof(FGCMeshSkin_t) );

		// Pack vertex transform data
		u8 *pTransDescArray = pExportBuffer + ADR_FGCTransDesc;
		for ( i = 0; i < _nTransDescCount; i++, pTransDescArray += sizeof( FGCTransDesc_t ) )
		{
			_TransDesc[i].ChangeEndian();
			memcpy( pTransDescArray, &_TransDesc[i], sizeof( FGCTransDesc_t ) );
		}
	}

	/////////////////////////////////////////////////////////////
	// PACK VERTEX ATTRIBUTE DATA INTO BUFFER
	for ( i = 0; i < _nPosNormCount; i++ )
	{
		_PosNorm[i].x = fang_ConvertEndian( _PosNorm[i].x );
		_PosNorm[i].y = fang_ConvertEndian( _PosNorm[i].y );
		_PosNorm[i].z = fang_ConvertEndian( _PosNorm[i].z );
		_PosNorm[i].nx = fang_ConvertEndian( _PosNorm[i].nx );
		_PosNorm[i].ny = fang_ConvertEndian( _PosNorm[i].ny );
		_PosNorm[i].nz = fang_ConvertEndian( _PosNorm[i].nz );
	}
	memcpy( pExportBuffer + ADR_FPosNorm,	_PosNorm,	_nPosNormCount * sizeof(FGCSkinPosNorm_t) );

	for ( i = 0; i < MLManager.m_pGCVertHash->m_nPosF32Count; i++ )
	{
		MLManager.m_pGCVertHash->m_PosF32[i].ChangeEndian();
	}
	memcpy( pExportBuffer + ADR_FPosF32, MLManager.m_pGCVertHash->m_PosF32, MLManager.m_pGCVertHash->m_nPosF32Count * sizeof(FGCPosF32_t) );

	for ( i = 0; i < MLManager.m_pGCVertHash->m_nPosS16Count; i++ )
	{
		MLManager.m_pGCVertHash->m_PosS16[i].ChangeEndian();
	}
	memcpy( pExportBuffer + ADR_FPosS16, MLManager.m_pGCVertHash->m_PosS16, MLManager.m_pGCVertHash->m_nPosS16Count * sizeof(FGCPosS16_t) );

	memcpy( pExportBuffer + ADR_FPosS8,	MLManager.m_pGCVertHash->m_PosS8, MLManager.m_pGCVertHash->m_nPosS8Count * sizeof(FGCPosS8_t) );

	if ( ADR_FNBT )
	{
		memcpy( pExportBuffer + ADR_FNBT,	MLManager.m_pGCVertHash->m_NBT8, MLManager.m_pGCVertHash->m_nNBT8Count * sizeof(FGCNBT8_t) );
	}

	memcpy( pExportBuffer + ADR_FWeights,	_Weights,	_nWeightsCount * sizeof(FGCWeights_t) );


	FGCColor_t *pColorDest = (FGCColor_t *)(pExportBuffer + ADR_FColor);
	for ( i = 0; i < MLManager.m_pGCVertHash->m_nColorCount; i++, pColorDest++ )
	{
		memcpy( pColorDest, &MLManager.m_pGCVertHash->m_Color[i].Color, sizeof(FGCColor_t) );
	}

	for ( i = 0; i < MLManager.m_pGCVertHash->m_nST16Count; i++ )
	{
		MLManager.m_pGCVertHash->m_ST16[i].ChangeEndian();
	}
	memcpy( pExportBuffer + ADR_FST16, MLManager.m_pGCVertHash->m_ST16, MLManager.m_pGCVertHash->m_nST16Count * sizeof(FGCST16_t) );


	/////////////////////////////////////////////////////////////
	// PACK DISPLAY LIST CONTAINER DATA INTO BUFFER

	// The original display list container pBuffer pointers were based on
	// the offset from the beginning of the mesh DL data.  Since we know
	// the location of the mesh DL data, now, we need to update the pointers:
	FGC_DLCont_t *pDLCont = (FGC_DLCont_t *)m_pDLContainerBuffer;
	for ( i = 0; i < nDLConts; i++ )
	{
		if ( MLManager.m_bAccumStreamingData )
		{
			pDLCont[i].nFlags |= FGCDL_FLAGS_STREAMING;
			pDLCont[i].pBuffer = (void *)( (u8 *)pDLCont[i].pBuffer + ADR_FDLData);
		}
		else
		{
			pDLCont[i].pBuffer = (void *)( (u8 *)pDLCont[i].pBuffer + ADR_FDLData );
		}
		pDLCont[i].ChangeEndian();
	}
	memcpy( pExportBuffer + ADR_FDLCont, m_pDLContainerBuffer, nDLConts * sizeof(FGC_DLCont_t) );
	if ( MLManager.m_bAccumStreamingData )
	{
		memcpy( pAccumDataBase + ADR_FDLData, m_pDLDataBuffer, m_nDLDataSize );
	}
	else
	{
		memcpy( pExportBuffer + ADR_FDLData, m_pDLDataBuffer, m_nDLDataSize );
	}

	/////////////////////////////////////////////////////////////
	// PACK VB DATA INTO BUFFER
	{
		u8 *pVBData = pExportBuffer + ADR_FVB;

		for ( i = 0; i < nVBs; i++ )
		{
			// First fix it up
			m_paFGCVB[i].pDiffuse = (FGCColor_t *)ADR_FColor;
			m_paFGCVB[i].nDiffuseCount =(u16) MLManager.m_pGCVertHash->m_nColorCount;

			if ( m_paFGCVB[i].nFlags & FGCVB_NORM_NBT )
			{
				m_paFGCVB[i].pNBT = (FGCNBT8_t *)ADR_FNBT;
			}
			else
			{
				m_paFGCVB[i].pNBT = NULL;
			}

			switch( m_paFGCVB[i].nPosType )
			{
				case GX_F32:
					m_paFGCVB[i].pPosition = (void *)ADR_FPosF32;
					m_paFGCVB[i].nPosCount = (u16)MLManager.m_pGCVertHash->m_nPosF32Count;
					break;

				case GX_S16:
					// If the pointers are already setup, then it is a skinned 
					// vert VB and should be left as-is
					if ( m_paFGCVB[i].nFlags & FGCVB_SKINNED )
					{
						m_paFGCVB[i].pPosition = (void *)ADR_FPosNorm;
						m_paFGCVB[i].nPosCount = (u16)_nPosNormCount;
					}
					else
					{
						m_paFGCVB[i].pPosition = (void *)ADR_FPosS16;
						m_paFGCVB[i].nPosCount = (u16)MLManager.m_pGCVertHash->m_nPosS16Count;
					}
					break;

				case GX_S8:
					m_paFGCVB[i].pPosition = (void *)ADR_FPosS8;
					m_paFGCVB[i].nPosCount = (u16)MLManager.m_pGCVertHash->m_nPosS8Count;
					break;

				default:
					FASSERT_NOW;
					break;
			}

			m_paFGCVB[i].pST = (FGCST16_t *)ADR_FST16;
//			m_paFGCVB[i].nSTCount[ii] = (u16)MLManager.m_pGCVertHash->m_nST16Count;

			// Pack the VB
			m_paFGCVB[i].ChangeEndian();
			memcpy( pVBData, &m_paFGCVB[i], sizeof( FGCVB_t ) );

			// Increment the VB pointer
			pVBData += sizeof( FGCVB_t );
		}
	}


	/////////////////////////////////////////////////////////////
	// PACK HW MATERIAL DATA INTO BUFFER 
	{
		// Get a convenient pointer
		u8 *pHWMaterialArray = pExportBuffer + ADR_FMeshHWMtl;
		u8 *pMaterialArray = pExportBuffer + ADR_FMeshMtl;
		u8 *pShMotifData = pExportBuffer + ADR_FShMotif;
		u8 *pShTexInstData = pExportBuffer + ADR_FShTexInst;
		u8 *pShRegisterData = pExportBuffer + ADR_FShRegister;
		u8 *pShTexNames = pExportBuffer + ADR_FTexName;

		MLMaterial_GC *pTempMat = (MLMaterial_GC *)m_pFirstMaterial;
		while ( pTempMat )
		{
			if ( pTempMat->m_pKongMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
			{
				// Skip meshes that are flagged as not visible
				pTempMat = (MLMaterial_GC *)pTempMat->m_pNext;
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
			pTempMat->CopyTextureData( &pShTexInstData, &pShTexNames, pExportBuffer, TRUE );

			// Set a pointer to the current register data entry
			pTempMat->m_FMaterial.pnShSurfaceRegisters = (u32 *)(pShRegisterData - pExportBuffer);

			// Copy over the surface registers, motif, ShTexInsts
			pTempMat->CopySurfaceShaderMSR( (u32 **)&pShRegisterData, &pShMotifData, pExportBuffer, TRUE );

			// Set a pointer to the current register data entry
			pTempMat->m_FMaterial.pnShLightRegisters = (u32 *)(pShRegisterData - pExportBuffer);

			// Copy over the light registers, motif, ShTexInsts
			pTempMat->CopyLightShaderMSR( (u32 **)&pShRegisterData, &pShMotifData, pExportBuffer, TRUE );

			// Set platform data pointer
			pTempMat->m_FMaterial.pPlatformData = (void *)(pHWMaterialArray - pExportBuffer);

			// Copy in the actual material
			pTempMat->m_FMaterial.ChangeEndian();
			fang_MemCopy( pMaterialArray, &pTempMat->m_FMaterial, sizeof( FMeshMaterial_t ) );

			// Increment data pointer
			pMaterialArray += sizeof( FMeshMaterial_t );

			// Pack hardware specific data
			if ( (u32)pTempMat->m_FGCMaterial.aDLContainer == 0xffffffff )
			{
				pTempMat->m_FGCMaterial.aDLContainer = NULL;
			}
			else
			{
				pTempMat->m_FGCMaterial.aDLContainer = (FGC_DLCont_t *)((u32)pTempMat->m_FGCMaterial.aDLContainer + ADR_FDLCont);
			}
			pTempMat->m_FGCMaterial.ChangeEndian();
			fang_MemCopy( pHWMaterialArray, &pTempMat->m_FGCMaterial, sizeof( FGCMeshMaterial_t ) );

			// Increment data pointer
			pHWMaterialArray += sizeof( FGCMeshMaterial_t );

			pTempMat = (MLMaterial_GC *)pTempMat->m_pNext;
		}
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
			pTempSegment->m_FMeshSeg.ChangeEndian();
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
						pPacketArray->ChangeEndian();
						pPacketArray++;
					}

					// Record the packet offset
					pNodeArray->paPackets = (FkDOP_TriPacket_t *)((u32)pPacketStart - (u32)pExportBuffer);

					// Calculate the size of the tri data
					u32 nTriVertDataSize, nFaceNormalDataSize;
					nTriVertDataSize = RoundUp4B( pNodeArray->nTriCount * sizeof(u16) * 3);
					nFaceNormalDataSize = RoundUp8B( pNodeArray->nTriCount * sizeof( FkDOP_CNormal_t ) );

					// Copy the tri data
					memcpy( pTriData, pNodeArray->pTriData, nTriVertDataSize + nFaceNormalDataSize );
					pNodeArray->pTriData = (void *)((u32)pTriData - (u32)pExportBuffer);

					// Change the endian of the tri data
					u16 *pVertIdx = (u16 *)pTriData;
					FkDOP_CNormal_t *pNormal = (FkDOP_CNormal_t *)((u32)pTriData + nTriVertDataSize);
					for ( iii = 0; iii < pNodeArray->nTriCount; iii++ )
					{
						pVertIdx[0] = fang_ConvertEndian( pVertIdx[0] );
						pVertIdx[1] = fang_ConvertEndian( pVertIdx[1] );
						pVertIdx[2] = fang_ConvertEndian( pVertIdx[2] );
						pVertIdx += 3;
						pNormal->ChangeEndian();
						pNormal++;
					}

					pTriData += nTriVertDataSize + nFaceNormalDataSize;
				}

				pNodeArray->ChangeEndian();
				pNodeArray++;
			}

			if ( pCollArray->nTreeNodeCount )
				pCollArray->pakDOPNodes = (FkDOP_Node_t *)((u32)pNodeStart - (u32)pExportBuffer);
			else
				pCollArray->pakDOPNodes = NULL;

			// Pack the intervals
			u32 nIntervalCount = FkDOP_aDesc[m_nCollkDOPType].nNormalCount * pCollArray->nTreeNodeCount;
			memcpy( pIntervalArray, pCollArray->paIntervals, nIntervalCount * sizeof( FkDOP_Interval_t ) );
			for ( ii = 0; ii < nIntervalCount; ii++ )
			{
				pIntervalArray[ii].ChangeEndian();
			}
			pCollArray->paIntervals = (FkDOP_Interval_t *)((u32)pIntervalArray - (u32)pExportBuffer);
			pIntervalArray += nIntervalCount;

			// Pack the Root kDOP vert data
			if ( pCollArray->nRootkDOPVertCount )
			{
				for ( ii = 0; ii < pCollArray->nRootkDOPVertCount; ii++ )
				{
					pCollArray->paRootkDOPVerts[ii].ChangeEndian();
				}
				memcpy( pRootDOPVertData, pCollArray->paRootkDOPVerts, pCollArray->nRootkDOPVertCount * sizeof( CFVec3 ) );
				pCollArray->paRootkDOPVerts = (CFVec3 *)((u32)pRootDOPVertData - (u32)pExportBuffer);
				pRootDOPVertData += pCollArray->nRootkDOPVertCount;
			}
			else
			{
				pCollArray->paRootkDOPVerts = NULL;
			}

			// Change the endian for the CollInfo and progress to the next
			pCollArray->ChangeEndian();
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
		// For GC we have only one color stream which is referenced by all VB's, so pass in 1 for the VB count
		WriteVertRemapFile( 1 );
	}

	// Fill out results info:
	m_Results.fTotalTime = (f32)(timeGetTime() - nStartTime) * 0.001f;

	m_Results.nExportDataSize = nExportSize;

	m_Results.nPos32Count = MLManager.m_pGCVertHash->m_nPosF32Count;
	m_Results.nPos16Count = MLManager.m_pGCVertHash->m_nPosS16Count + _nPosNormCount;// + _nPosNBTCount;
	m_Results.nNormal16Count = _nPosNormCount;// + _nNBTCount;
	m_Results.nPos8Count =  MLManager.m_pGCVertHash->m_nPosS8Count;
	m_Results.nST16Count = MLManager.m_pGCVertHash->m_nST16Count;
	m_Results.nColorCount = MLManager.m_pGCVertHash->m_nColorCount;
	m_Results.nWeightCount = _nWeightsCount;

	m_Results.nBoneCount = nBones;
	m_Results.nSegmentCount = nSegments;
	m_Results.nTransDescCount = _nTransDescCount;
	m_Results.nMaterialCount = nMaterials;
	m_Results.nFShTexInstCount = nMaterials;
	m_Results.nTexLyrIDCount = nTexLayerIDs;
	m_Results.nMeshLightCount = nMeshLights;
	m_Results.nVBCount = nVBs;
	m_Results.nDLCount = nDLConts;

	m_Results.nFMeshMem = MEM_FMesh;
	m_Results.nFPlatMeshMem = MEM_FGCMesh;
	m_Results.nFGCMeshSkinMem = MEM_FGCMeshSkin + MEM_FGCTransDesc;
	m_Results.nBoneMem = MEM_FMeshBone;
	m_Results.nSkeletonMem = MEM_FSkeleton;
	m_Results.nSegmentMem = MEM_FMeshSeg;
	m_Results.nShaderMem = MEM_FShMotif + MEM_FShRegister + MEM_FShTexInst;
	m_Results.nMaterialMem = MEM_FMeshHWMtl + MEM_FTexName;
	m_Results.nTexLyrIDMem = MEM_FTexLayerID;
	m_Results.nMeshLightMem = MEM_FMeshLight;
	m_Results.nVBMem = MEM_FVB;

	m_Results.nVertsRendered =  GCMesh_nVertsRendered;
	m_Results.nMatrixSwitches = _nMatrixSwitches;
	m_Results.nMatrixLoads = _nMatrixLoads;
	m_Results.nDLChanges = GCMesh_nDLChanges;
	m_Results.nGDBeginCalls = GCMesh_nGDBeginCalls;
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
/*
	u32 nOtherMemory = nExportSize - (m_Results.nDLMem + m_Results.nCollMem + m_Results.nVertAttrMem + m_Results.nVBMem + m_Results.nMaterialMem);
	DEVPRINTF( "\nMLMesh_GC.cpp Memory Totals: %s\n", m_FMesh.szName );
	DEVPRINTF( "	Material:	%4.2f%%	(%d)\n", ((f32)m_Results.nMaterialMem / (f32)nExportSize) * 100.f, m_Results.nMaterialMem );
	DEVPRINTF( "	Vert + VB:	%4.2f%%	(%d)\n", (((f32)m_Results.nVertAttrMem + (f32)m_Results.nVBMem) / (f32)nExportSize) * 100.f, m_Results.nVertAttrMem + m_Results.nVBMem );
	DEVPRINTF( "	Collision:	%4.2f%%	(%d)\n", ((f32)m_Results.nCollMem / (f32)nExportSize) * 100.f, m_Results.nCollMem );
	DEVPRINTF( "	DL:			%4.2f%%	(%d)\n", ((f32)m_Results.nDLMem / (f32)nExportSize) * 100.f, m_Results.nDLMem );
	DEVPRINTF( "	Other:		%4.2f%%	(%d)\n", ((f32)nOtherMemory / (f32)nExportSize) * 100.f, nOtherMemory );
	DEVPRINTF( "	TOTAL-------------	(%d)\n", nExportSize );

	DEVPRINTF( "\nMLMesh_GC.cpp Collision Breakdown: %s\n", m_FMesh.szName );
	DEVPRINTF( "	Tree:		%4.2f%%	(%d)\n", ((f32)MEM_FCollTree / (f32)m_Results.nCollMem) * 100.f, MEM_FCollTree );
	DEVPRINTF( "	Nodes:		%4.2f%%	(%d)\n", ((f32)MEM_FkDOPNodes / (f32)m_Results.nCollMem) * 100.f, MEM_FkDOPNodes );
	DEVPRINTF( "	Intervals:	%4.2f%%	(%d)\n", ((f32)MEM_FIntervals / (f32)m_Results.nCollMem) * 100.f, MEM_FIntervals );
	DEVPRINTF( "	TriPackets:	%4.2f%%	(%d)\n", ((f32)MEM_FTriPackets / (f32)m_Results.nCollMem) * 100.f, MEM_FTriPackets );
	DEVPRINTF( "	TriData:	%4.2f%%	(%d)\n", ((f32)MEM_FTriData / (f32)m_Results.nCollMem) * 100.f, MEM_FTriData );
	DEVPRINTF( "	RootVert:	%4.2f%%	(%d)\n", ((f32)MEM_FRootDOPVertData / (f32)m_Results.nCollMem) * 100.f, MEM_FRootDOPVertData );

	DEVPRINTF( "Total bump verts rendered: %d\n", GCMesh_nTotalBumpedVertsRendered );
*/
	return &m_Results;
}


//
//
//
u32 MLMesh_GC::CreateVertexAttributeArrays( void )
{
	// Allocate memory for the vertex abstractions
	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		MLTriContainer *pTempTriCont = pTempSegment->m_pFirstTriContainer;
		while ( pTempTriCont )
		{
			if ( pTempTriCont->m_pVertAbstr )
			{
				FASSERT_NOW;
				return ML_GENERAL_ERROR;
			}

			pTempTriCont->m_pVertAbstr = (GCVertAbstr_t *)fang_Malloc( pTempTriCont->m_nVertCount * sizeof( GCVertAbstr_t ), 4 );
			if ( !pTempTriCont->m_pVertAbstr )
			{
				return ML_OUT_OF_MEMORY;
			}

			memset( pTempTriCont->m_pVertAbstr, 0, sizeof( GCVertAbstr_t ) * pTempTriCont->m_nVertCount );

			pTempTriCont = pTempTriCont->m_pNext;
		}

		pTempSegment = pTempSegment->m_pNext;
	}

	// Store the skinned verts in the tri containers that 1 or less matrices
	if ( !StoreTriContainerSkinned( 1, &_nTransDesc1Matrix ) )
	{
		return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
	}

	// Store the skinned verts that are left in the tri containers 
	// that have two or less matrices
	if ( !StoreTriContainerSkinned( 2, &_nTransDesc2Matrix ) )
	{
		return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
	}

	// Store the skinned verts that are left in the tri containers 
	// that have less than 5 matrices (all remaining skinned verts)
	if ( !StoreTriContainerSkinned( 4, &_nTransDesc3or4Matrix ) )
	{
		return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
	}

	pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		MLTriContainer *pTempTriCont = pTempSegment->m_pFirstTriContainer;
		while ( pTempTriCont )
		{
			if ( pTempTriCont->m_nMtxWeightCount < 2 )
			{
				if ( !ResolveTriContainerNormals( pTempTriCont ) )
				{
					return ML_UNABLE_TO_RESOLVE_VERT_NORMAL;
				}
			}

			if ( !StoreTriContainer( pTempTriCont ) )
			{
				return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
			}

			pTempTriCont = pTempTriCont->m_pNext;
		}

		pTempSegment = pTempSegment->m_pNext;
	}

	return ML_NO_ERROR;
}


//
//
//
BOOL MLMesh_GC::StoreTriContainerSkinned( u32 nMatrixCountLimit, u32 *pProcessedCounter )
{
	// GC can only handle up to 4 skin weights
	FASSERT( MAX_WEIGHTS_PER_VERT <= 4 );

	// First add in all skinned tri positions/normals that use only one matrix
	MLSegment *pTempSegment, *pTempSegment2;
	MLTriContainer *pTempTC, *pTempTC2;
	for ( pTempSegment = m_pFirstSegment; pTempSegment != NULL; pTempSegment = pTempSegment->m_pNext )
	{
		if ( pTempSegment->m_nMtxWeightCount < 2 ) 
		{
			continue;
		}

		for ( pTempTC = pTempSegment->m_pFirstTriContainer; pTempTC != NULL; pTempTC = pTempTC->m_pNext )
		{
			for ( u32 i = 0; i < pTempTC->m_nVertCount; i++ )
			{
				// We set the number of weights to -1 for verts that have already been considered.
				if ( pTempTC->m_paKongVerts[i].nNumWeights < -1 )	
				{
					continue;
				}

				// Check only verts that are within the matrix limit
				if ( pTempTC->m_paKongVerts[i].nNumWeights > nMatrixCountLimit )	
				{
					continue;
				}

				// Make sure we don't exceed the max allowable:
				FASSERT( _nTransDescCount < MLMESH_MAX_GC_TRANS_DESC );
				if ( _nTransDescCount == MLMESH_MAX_GC_TRANS_DESC )
				{
					return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
				}

				// Create a new transform description and weight array for the vert:
				f32 fWeights[4];
				u32 nMtxCount;
				for ( nMtxCount = 0; nMtxCount < pTempTC->m_paKongVerts[i].nNumWeights; nMtxCount++ )
				{
					fWeights[nMtxCount] = pTempTC->m_paKongVerts[i].fWeight[nMtxCount];
					FASSERT( fWeights[nMtxCount] );
					_TransDesc[_nTransDescCount].nMtxIdx[nMtxCount] = (u8)pTempTC->m_paKongVerts[i].nWeightBoneIdx[nMtxCount];
				}

				_TransDesc[ _nTransDescCount ].nMatrixCount = (u8)nMtxCount;

				// Sanity check to ensure that the actual matrix (weight) count is below the limit.
				FASSERT( nMtxCount <= nMatrixCountLimit );

				// Fill in the rest of the matrix indices with 255
				while ( nMtxCount < 4 )
				{
					fWeights[nMtxCount] = 0.f;
					_TransDesc[ _nTransDescCount ].nMtxIdx[ nMtxCount++ ] = 255;
				}

				if ( !AddSkinnedApeVert( pTempTC, i, fWeights, _TransDesc[ _nTransDescCount ].nMatrixCount ) )
				{
					return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
				}
				_TransDesc[ _nTransDescCount ].nVertCount = 1;

				// Now cycle through the other segments looking for verts using the same matrices
				for ( pTempSegment2 = pTempSegment; pTempSegment2 != NULL; pTempSegment2 = pTempSegment2->m_pNext )
				{
					if ( pTempSegment2->m_nMtxWeightCount < 2 )
					{
						continue;
					}

					for ( pTempTC2 = pTempSegment2->m_pFirstTriContainer; pTempTC2 != NULL; pTempTC2 = pTempTC2->m_pNext )
					{
						for ( u32 ii = 0; ii < pTempTC2->m_nVertCount; ii++ )
						{
							// Check only verts that have the same matrix count
							if ( pTempTC2->m_paKongVerts[ii].nNumWeights != _TransDesc[ _nTransDescCount ].nMatrixCount )
							{
								continue;
							}

							u32 bMatchingMatrix = TRUE;

							// See if the matrix indices are the same
							for ( u32 nMatrix = 0; nMatrix < _TransDesc[ _nTransDescCount ].nMatrixCount; nMatrix++ )
							{
								u8 nMtxIdx = _TransDesc[ _nTransDescCount ].nMtxIdx[ nMatrix ];
								if ( nMtxIdx == 255 )
								{
									break;
								}

								for ( u32 nWeightIdx = 0; nWeightIdx < pTempTC2->m_paKongVerts[ii].nNumWeights; nWeightIdx++ )
								{
									FASSERT( pTempTC2->m_paKongVerts[ii].fWeight[ nWeightIdx ] );
									if ( pTempTC2->m_paKongVerts[ii].nWeightBoneIdx[ nWeightIdx ] == nMtxIdx )
									{
										fWeights[nMatrix] = pTempTC2->m_paKongVerts[ii].fWeight[ nWeightIdx ];
										break;
									}
								}

								// If we didn't find a match, then bail out
								if ( nWeightIdx == pTempTC2->m_paKongVerts[ii].nNumWeights )
								{
									bMatchingMatrix = FALSE;
									break;
								}
							}

							// If all of the matrices match, then add this vert
							if ( bMatchingMatrix )
							{
								// Make sure the rest of the weights are recorded as zero
								while ( nMatrix < 4 )
									fWeights[nMatrix++] = 0.f;

								if ( !AddSkinnedApeVert( pTempTC2, ii, fWeights, _TransDesc[ _nTransDescCount ].nMatrixCount ) )
								{
									return ML_MESH_VERTEX_DATA_EXCEEDED_LIB_BUFFER;
								}
								_TransDesc[ _nTransDescCount ].nVertCount++;
							}
						}
					}
				}

				(*pProcessedCounter)++;
				_nTransDescCount++;
			}
		}
	}

	return TRUE;
}


//
//
//
BOOL MLMesh_GC::AddSkinnedApeVert( MLTriContainer *pTriCont, u32 i, f32 *pWeights, u32 nWeights )
{
	if ( _nPosNormCount == MLMESH_MAX_GC_ATTRIBUTE_INDICES || _nWeightsCount == MLMESH_MAX_GC_ATTRIBUTE_INDICES )
	{
		return FALSE;
	}

	// For speed of processing, we do not get rid of duplicate skinned verts
	_PosNorm[_nPosNormCount].x = (u16)(pTriCont->m_paKongVerts[i].Pos.x * 64);
	_PosNorm[_nPosNormCount].y = (u16)(pTriCont->m_paKongVerts[i].Pos.y * 64);
	_PosNorm[_nPosNormCount].z = (u16)(pTriCont->m_paKongVerts[i].Pos.z * 64);
	_PosNorm[_nPosNormCount].nx = (u16)(pTriCont->m_paKongVerts[i].Norm.x * 16384);
	_PosNorm[_nPosNormCount].ny = (u16)(pTriCont->m_paKongVerts[i].Norm.y * 16384);
	_PosNorm[_nPosNormCount].nz = (u16)(pTriCont->m_paKongVerts[i].Norm.z * 16384);

	// Fix the position index for this vert
	((GCVertAbstr_t *)pTriCont->m_pVertAbstr)[i].nPosIdx = (u16)_nPosNormCount;
	((GCVertAbstr_t *)pTriCont->m_pVertAbstr)[i].nNormIdx = (u16)_nPosNormCount;

	_nPosNormCount++;

	// Set the weight, now
	if ( nWeights > 1 )
	{
		_Weights[_nWeightsCount].w0 = (u8)(pWeights[0] * 255);
		_Weights[_nWeightsCount].w1 = (u8)(pWeights[1] * 255);
		_Weights[_nWeightsCount].w2 = (u8)(pWeights[2] * 255);
		_Weights[_nWeightsCount].w3 = (u8)(pWeights[3] * 255);
		_nWeightsCount++;
	}

	// Mark the weights as -1 so that we know we've added this one already
	pTriCont->m_paKongVerts[i].nNumWeights = -1;

	m_nIndexedVertCount++;

	return TRUE;
}


//
//
//
BOOL MLMesh_GC::StoreTriContainer( MLTriContainer *pTriCont )
{
	u32 i, ii;
	s32 nNewIndex;

	m_nIndexedVertCount += pTriCont->m_nVertCount;

	m_Results.nOrigPosCount += pTriCont->m_nVertCount;
	m_Results.nOrigNormalCount += pTriCont->m_nVertCount;
	m_Results.nOrigSTCount += pTriCont->m_nVertCount * (pTriCont->m_nBaseSTSets + pTriCont->m_nLightMapSTSets);
	m_Results.nOrigColorCount += pTriCont->m_nVertCount;
	if ( pTriCont->m_nMtxWeightCount > 1 )
	{
		m_Results.nOrigWeightCount += pTriCont->m_nVertCount;
	}

	GCVertAbstr_t *pAbstr = (GCVertAbstr_t *)pTriCont->m_pVertAbstr;

	if ( pTriCont->m_nMtxWeightCount < 2 )
	{
		// Store positions in the temp arrays based on amount of quantize...
		switch ( pTriCont->m_nPosType )
		{
			case GX_S8:
			{
				u32 nPosShift = 1 << pTriCont->m_nPosFrac;

				// Cycle through all of the positions, adding them in
				FGCPosS8_t vPos;
				for ( i = 0; i < pTriCont->m_nVertCount; i++ )
				{
					vPos.x = (s8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Pos.x * nPosShift);
					vPos.y = (s8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Pos.y * nPosShift);
					vPos.z = (s8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Pos.z * nPosShift);

					nNewIndex = MLManager.m_pGCVertHash->AddVert( &vPos );
					if ( nNewIndex == -1 )
					{
						return FALSE;
					}

					// Fix the position index for this vert
					pAbstr[i].nPosIdx = (u16)nNewIndex;
				}

				break;
			}

			case GX_S16:
			{
				u32 nPosShift = 1 << pTriCont->m_nPosFrac;

				// Cycle through all of the positions, adding them in
				FGCPosS16_t vPos;
				for ( i = 0; i < pTriCont->m_nVertCount; i++ )
				{
					vPos.x = (s16)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Pos.x * nPosShift);
					vPos.y = (s16)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Pos.y * nPosShift);
					vPos.z = (s16)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Pos.z * nPosShift);

					nNewIndex = MLManager.m_pGCVertHash->AddVert( &vPos );
					if ( nNewIndex == -1 )
					{
						return FALSE;
					}

					// Fix the position index for this vert
					pAbstr[i].nPosIdx = (u16)nNewIndex;
				}

				break;
			}

			case GX_F32:
			{
				// Cycle through all of the positions, adding them in
				FGCPosF32_t vPos;
				for ( i = 0; i < pTriCont->m_nVertCount; i++ )
				{
					vPos.x = pTriCont->m_paKongVerts[i].Pos.x;
					vPos.y = pTriCont->m_paKongVerts[i].Pos.y;
					vPos.z = pTriCont->m_paKongVerts[i].Pos.z;

					nNewIndex = MLManager.m_pGCVertHash->AddVert( &vPos );
					if ( nNewIndex == -1 )
					{
						return FALSE;
					}

					// Fix the position index for this vert
					pAbstr[i].nPosIdx = (u16)nNewIndex;
				}

				break;
			}

			default:
			{
				break;
			}
		}
	}

	FGCNBT8_t NBT;
#if MLMESHGC_USE_NORMAL_SPHERE_FOR_STREAMING_NBTS
	if ( pTriCont->m_bBumpMapped && !MLManager.m_bAccumStreamingData )
#else
	if ( pTriCont->m_bBumpMapped )
#endif
	{
		for ( i = 0; i < pTriCont->m_nVertCount; i++ )
		{
			NBT.nx = (u8)(pTriCont->m_paKongVerts[i].Norm.x * 64);
			NBT.ny = (u8)(pTriCont->m_paKongVerts[i].Norm.y * 64);
			NBT.nz = (u8)(pTriCont->m_paKongVerts[i].Norm.z * 64);
			NBT.bx = (u8)(pTriCont->m_paKongVerts[i].Binorm.x * 64);
			NBT.by = (u8)(pTriCont->m_paKongVerts[i].Binorm.y * 64);
			NBT.bz = (u8)(pTriCont->m_paKongVerts[i].Binorm.z * 64);
			NBT.tx = (u8)(pTriCont->m_paKongVerts[i].Tangent.x * 64);
			NBT.ty = (u8)(pTriCont->m_paKongVerts[i].Tangent.y * 64);
			NBT.tz = (u8)(pTriCont->m_paKongVerts[i].Tangent.z * 64);

			nNewIndex = MLManager.m_pGCVertHash->AddNBT8( &NBT );
			if ( nNewIndex == -1 )
			{
				return FALSE;
			}

			// Fix the color index for this vert
			pAbstr[i].nNormIdx = (u16)nNewIndex;
		}
	}

	// Add the colors in - We remove dupes but we don't quantize
	FGCColor_t	Color;
	for ( i = 0; i < pTriCont->m_nVertCount; i++ )
	{
		FMATH_CLAMP( pTriCont->m_paKongVerts[i].Color.fRed, 0.f, 1.f );
		FMATH_CLAMP( pTriCont->m_paKongVerts[i].Color.fGreen, 0.f, 1.f );
		FMATH_CLAMP( pTriCont->m_paKongVerts[i].Color.fBlue, 0.f, 1.f );
		FMATH_CLAMP( pTriCont->m_paKongVerts[i].Color.fAlpha, 0.f, 1.f );
		Color.r = (u8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Color.fRed * 255.f);
		Color.g = (u8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Color.fGreen * 255.f);
		Color.b = (u8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Color.fBlue * 255.f);
		Color.a = (u8)fmath_FloatToS32(pTriCont->m_paKongVerts[i].Color.fAlpha * 255.f);

		if ( MLManager.m_bVertexRadiosityLit )
		{
			nNewIndex = MLManager.m_pGCVertHash->AddColor( pTriCont->m_nVBIndex, pAbstr[i].nPosIdx, pAbstr[i].nNormIdx, &Color );
		}
		else
		{
			nNewIndex = MLManager.m_pGCVertHash->AddColor( &Color );
		}
		if ( nNewIndex == -1 )
		{
			return FALSE;
		}

		// Fix the color index for this vert
		pAbstr[i].nDiffuseIdx = (u16)nNewIndex;

		if ( m_paVertRemaps )
		{
			if ( nNewIndex >= (s32)m_nRemapCount )
			{
				DEVPRINTF( "MLMesh_PC::DivideVertsByVB() - Color index exceeded vertex count!\n" );
				FASSERT_NOW;
			}
			else
			{
				if (    (m_paVertRemaps[pTriCont->m_paKongVerts[i].nMeshVertIdx].nVBIndex != 0xffff 
						&& m_paVertRemaps[pTriCont->m_paKongVerts[i].nMeshVertIdx].nVBIndex != pTriCont->m_nVBIndex)
						|| (m_paVertRemaps[pTriCont->m_paKongVerts[i].nMeshVertIdx].nVertColorIndex != 0xffff 
						&& m_paVertRemaps[pTriCont->m_paKongVerts[i].nMeshVertIdx].nVertColorIndex != nNewIndex) )
				{
					DEVPRINTF( "MLMesh_PC::DivideVertsByVB() - Vert %d added to VB already!\n", pTriCont->m_paKongVerts[i].nMeshVertIdx );
				}

				m_paVertRemaps[pTriCont->m_paKongVerts[i].nMeshVertIdx].nVBIndex = 0;
				m_paVertRemaps[pTriCont->m_paKongVerts[i].nMeshVertIdx].nVertColorIndex = nNewIndex;
			}
		}
	}

	// Add the ST's in - We remove dupes and quantize to u16 or u8
	for ( i = 0; i < (u32)(pTriCont->m_nBaseSTSets + pTriCont->m_nLightMapSTSets); i++ )
	{
		u32 nSTShift = 1 << pTriCont->m_nSTFrac[i];

		FGCST16_t STs;
		for ( ii = 0; ii < pTriCont->m_nVertCount; ii++ )
		{
			STs.s = (s16)(pTriCont->m_paKongVerts[ii].aUV[i].x * nSTShift);
			STs.t = (s16)(pTriCont->m_paKongVerts[ii].aUV[i].y * nSTShift);

			nNewIndex = MLManager.m_pGCVertHash->AddST( &STs );
			if ( nNewIndex == -1 )
			{
				return FALSE;
			}

			// Fix the st index for this vert
			pAbstr[ii].nSTIdx[i] = (u16)nNewIndex;
		}
	}

	return TRUE;
}


//
//
//
BOOL MLMesh_GC::ResolveTriContainerNormals( MLTriContainer *pTriCont )
{
	// Cycle through all of the normals, resolving them against
	// the Fang GameCube Unit Normal Sphere
	for ( u32 i = 0; i < pTriCont->m_nVertCount; i++ )
	{
		u16 nNewIndex = fmesh_GetNormalSphereIndex( pTriCont->m_paKongVerts[i].Norm.x,
											pTriCont->m_paKongVerts[i].Norm.y,
											pTriCont->m_paKongVerts[i].Norm.z );

		// Bail if we didn't get a normal
		if ( nNewIndex == FMESH_NORMAL_SPHERE_NORMAL_NULL )
		{
			return FALSE;
		}

		// Fix the normal index for this vert
		((GCVertAbstr_t *)pTriCont->m_pVertAbstr)[i].nNormIdx = nNewIndex;

#if MLMESHGC_USE_NORMAL_SPHERE_FOR_STREAMING_NBTS
		if ( pTriCont->m_bBumpMapped && MLManager.m_bAccumStreamingData )
		{
			nNewIndex = fmesh_GetNormalSphereIndex( pTriCont->m_paKongVerts[i].Binorm.x,
												pTriCont->m_paKongVerts[i].Binorm.y,
												pTriCont->m_paKongVerts[i].Binorm.z );

			// Bail if we didn't get a normal
			if ( nNewIndex == FMESH_NORMAL_SPHERE_NORMAL_NULL )
			{
				return FALSE;
			}

			// Fix the normal index for this vert
			((GCVertAbstr_t *)pTriCont->m_pVertAbstr)[i].nBinormIdx = nNewIndex;

			nNewIndex = fmesh_GetNormalSphereIndex( pTriCont->m_paKongVerts[i].Tangent.x,
												pTriCont->m_paKongVerts[i].Tangent.y,
												pTriCont->m_paKongVerts[i].Tangent.z );

			// Bail if we didn't get a normal
			if ( nNewIndex == FMESH_NORMAL_SPHERE_NORMAL_NULL )
			{
				return FALSE;
			}

			// Fix the normal index for this vert
			((GCVertAbstr_t *)pTriCont->m_pVertAbstr)[i].nTangIdx = nNewIndex;
		}
#endif
	}

	return TRUE;
}


//
//
//
#define ESTIMATED_DL_BYTES_PER_INDEX	22
u32 MLMesh_GC::CreateDLContainers( u32 nDLCount )
{
	u32 nCurrentOffset = 0;
	u32 nCurrentDL = 0;

	// Allocate buffer for the display lists.  This should be exact.
	m_pDLContainerBuffer = (u8 *)fang_Malloc( nDLCount * sizeof ( FGC_DLCont_t ), 32 );
	if ( !m_pDLContainerBuffer )
	{
		return ML_OUT_OF_MEMORY;
	}
	u8 *pCurrentDLCont = m_pDLContainerBuffer;

	// Allocate buffer to contain the DL data.  This is an estimate 
	// which may need to be modified in the future.
	u32 nDataBufferBytes = m_nIndexedVertCount * ESTIMATED_DL_BYTES_PER_INDEX;
	if ( nDataBufferBytes < 256 )
	{
		nDataBufferBytes = 256;
	}
	u32 nDataBufferBytesLeft = nDataBufferBytes;
	m_pDLDataBuffer = (u8 *)fang_Malloc( nDataBufferBytes, 32 );
	if ( !m_pDLDataBuffer )
	{
		return ML_OUT_OF_MEMORY;
	}
	u8 *pCurrentDataBuffer = m_pDLDataBuffer;

	// Cycle through the materials adding in the display lists 
	MLMaterial_GC *pTempMat = (MLMaterial_GC *)m_pFirstMaterial;
	while ( pTempMat )
	{
		// Render the material's tris into the display list
		m_Results.nErrorCode = pTempMat->GenerateDisplayList( (u32)pCurrentDLCont - (u32)m_pDLContainerBuffer, 
															&pCurrentDLCont, &pCurrentDataBuffer, &nCurrentOffset, 
															&nDataBufferBytesLeft, &nCurrentDL, m_paFGCVB );

		if ( m_Results.nErrorCode )
		{
			 return m_Results.nErrorCode;
		}

		if ( nCurrentDL > nDLCount )
		{
			return ML_DISPLAY_LIST_CONTAINER_BUFFER_OVERRUN;
		}

		pTempMat = (MLMaterial_GC *)pTempMat->m_pNext;
	}

	// Record the excess memory buffer for later optimization
	m_Results.nDLMemEstimateExcess = nDataBufferBytesLeft;

	m_nDLDataSize = nCurrentOffset;

	FASSERT( nCurrentDL == nDLCount );

	return ML_NO_ERROR;
}


//
//
//
u32 MLMesh_GC::CreateVertexBuffers( u32 *pVBCount )
{
	#define _MAX_VBS	1024

	if ( !m_pFirstSegment )
	{
		return ML_NO_SEGMENTS_ESTABLISHED_FOR_EXPORT;
	}

	MLTriContainer *pUniqueTris[ _MAX_VBS ];
	u32	nVBCount = 0;
	u32 i, ii;

	// Cycle through the segments adding up the number of unique VBKeys
	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		MLTriContainer *pTC = pTempSegment->m_pFirstTriContainer;
		while ( pTC )
		{
			for ( i = 0; i < nVBCount; i++ )
			{
				if ( pUniqueTris[i]->m_nVBKey == pTC->m_nVBKey )
				{
					pTC->m_nVBIndex = i;

					break;
				}
			}

			// If we didn't find a matching key, add this one in
			if ( pTC->m_nVBIndex == -1 )
			{
				FASSERT( nVBCount < _MAX_VBS );
				pUniqueTris[ nVBCount ] = pTC;
				pTC->m_nVBIndex = nVBCount;
				nVBCount++;
			}

			pTC = pTC->m_pNext;
		}

		pTempSegment = pTempSegment->m_pNext;
	}

	if ( !nVBCount )
	{
		return ML_NO_VERTEX_BUFFERS_ESTABLISHED_FOR_EXPORT;
	}

	if ( nVBCount > 255 )
	{
		// Cannot handle > 255 vertex buffers per mesh
		return ML_NO_VERTEX_BUFFERS_ESTABLISHED_FOR_EXPORT;
	}

	(*pVBCount) = nVBCount;

	// Remove duplicate vertex attributes and place attributes in the appropriate arrays
	u32 nProcessStartTime = timeGetTime();
	m_Results.nErrorCode = CreateVertexAttributeArrays();
	if ( m_Results.nErrorCode )
	{
		return m_Results.nErrorCode;
	}
	m_Results.fCompressTime = (f32)(timeGetTime() - nProcessStartTime) * 0.001f;

	// Allocate the fang GameCube vertex buffers
	m_paFGCVB = (FGCVB_t *)malloc( nVBCount * sizeof( FGCVB_t ) );
	if ( !m_paFGCVB )
	{
		return ML_NO_VERTEX_BUFFERS_ESTABLISHED_FOR_EXPORT;
	}

	// Clear the memory
	memset( m_paFGCVB, 0, nVBCount * sizeof( FGCVB_t ) );

	if ( nVBCount > 255 )
	{
		return ML_GENERAL_ERROR;
	}

	// Fill in the VB info that we currently know based on the sample tri container
	for ( i = 0; i < nVBCount; i++ )
	{
		m_paFGCVB[i].nPosType = (u8)pUniqueTris[i]->m_nPosType;
		m_paFGCVB[i].nPosFrac = (u8)pUniqueTris[i]->m_nPosFrac;

		// Fix-up the counts
		m_paFGCVB[i].nDiffuseCount =(u16)MLManager.m_pGCVertHash->m_nColorCount;

		switch( m_paFGCVB[i].nPosType )
		{
			case GX_F32:
				m_paFGCVB[i].nPosCount = (u16)MLManager.m_pGCVertHash->m_nPosF32Count;
				m_paFGCVB[i].nPosStride = sizeof( FGCPosF32_t );
				m_paFGCVB[i].nGCVertexFormat = FGCDATA_VARIABLE_VERTEX_FORMAT;
				for( ii = 0; ii < FGCDATA_FIXED_VERTEX_FORMATS; ii++ )
				{
					if ( FGCData_VertexFormatDesc[ii].nPosBitDepth == 32 )
					{
						m_paFGCVB[i].nGCVertexFormat = (u8)ii;
						break;
					}
				}
				break;

			case GX_S16:
				if ( pUniqueTris[i]->m_nMtxWeightCount > 1 )
				{
					m_paFGCVB[i].nFlags |= FGCVB_SKINNED;
					m_paFGCVB[i].nPosCount = (u16)_nPosNormCount;
					m_paFGCVB[i].nPosStride = sizeof( FGCSkinPosNorm_t );
					m_paFGCVB[i].nGCVertexFormat = FGCDATA_VARIABLE_VERTEX_FORMAT;
				}
				else
				{
					m_paFGCVB[i].nPosCount = (u16)MLManager.m_pGCVertHash->m_nPosS16Count;
					m_paFGCVB[i].nPosStride = sizeof( FGCPosS16_t );
					m_paFGCVB[i].nGCVertexFormat = FGCDATA_VARIABLE_VERTEX_FORMAT;
					for( ii = 0; ii < FGCDATA_FIXED_VERTEX_FORMATS; ii++ )
					{
						if (	FGCData_VertexFormatDesc[ii].nPosBitDepth == 16
							&&  FGCData_VertexFormatDesc[ii].nPosFracBits == m_paFGCVB[i].nPosFrac )
						{
							m_paFGCVB[i].nGCVertexFormat = (u8)ii;
							break;
						}
					}
				}
				break;

			case GX_S8:
				m_paFGCVB[i].nPosCount = (u16)MLManager.m_pGCVertHash->m_nPosS8Count;
				m_paFGCVB[i].nPosStride = sizeof( FGCPosS8_t );
				m_paFGCVB[i].nGCVertexFormat = FGCDATA_VARIABLE_VERTEX_FORMAT;
				for( ii = 0; ii < FGCDATA_FIXED_VERTEX_FORMATS; ii++ )
				{
					if (	FGCData_VertexFormatDesc[ii].nPosBitDepth == 8
						&&  FGCData_VertexFormatDesc[ii].nPosFracBits == m_paFGCVB[i].nPosFrac )
					{
						m_paFGCVB[i].nGCVertexFormat = (u8)ii;
						break;
					}
				}
				break;

			default:
				FASSERT_NOW;
				break;
		}

		// Force bump mapping to use the variable format:
		if ( pUniqueTris[i]->m_bBumpMapped )
		{
			m_paFGCVB[i].nGCVertexFormat = FGCDATA_VARIABLE_VERTEX_FORMAT;
			m_paFGCVB[i].nFlags |= FGCVB_NORM_NBT;
		}

		if ( m_paFGCVB[i].nPosCount < 256 )
		{
			m_paFGCVB[i].nPosIdxType = GX_INDEX8;
		}
		else
		{
			m_paFGCVB[i].nPosIdxType = GX_INDEX16;
		}

		if ( m_paFGCVB[i].nDiffuseCount < 256 )
		{
			m_paFGCVB[i].nColorIdxType = GX_INDEX8;
		}
		else
		{
			m_paFGCVB[i].nColorIdxType = GX_INDEX16;
		}
	}

	return ML_NO_ERROR;
}


//
//
//
u32 MLMesh_GC::ResolveMaterials( u32 *pDLCount )
{
	u32 nError = MLMesh::ResolveMaterials( pDLCount );

	if ( nError != ML_NO_ERROR )
	{
		return nError;
	}

	// Optimize the materials
	MLMaterial_GC *pMat = (MLMaterial_GC *)m_pFirstMaterial;
	while ( pMat )
	{
		if (pMat->m_nBumpMapTexture > -1) { pMat->m_bUseNBT = TRUE; }

		if ( pDLCount )
		{
			(*pDLCount) += pMat->OptimizeDLs();
		}
		else
		{
			pMat->OptimizeDLs();
		}

		pMat = (MLMaterial_GC *)pMat->m_pNext;
	}

	return ML_NO_ERROR;
}


