//////////////////////////////////////////////////////////////////////////////////////
// fdx8load.cpp - Fang DX module responsible for converting data fresh off the disk
//                into the engine format.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/22/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdata.h"
#include "fdx8load.h"
#include "fdx8vb.h"
#include "fdx8mesh.h"
#include "fmesh_coll.h"
#include "fshaders.h"
#include "fRenderSort.h"


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;


//
//
//
BOOL fdx8load_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;

	return TRUE;
}

//
//
//
void fdx8load_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}


//
//
//
FMesh_t *fdx8load_Create( FMesh_t *pLoadMesh, cchar *pszResName ) 
{
	u32 i, ii;
	HRESULT hResult;
	BYTE *pData;

	FResFrame_t Frame = fres_GetFrame();

	// Get the size of the non-disposable portion of the file (the portion that
	// won't be converted into DX resources)
	FDX8Mesh_t *pDX8Mesh = (FDX8Mesh_t *)((u32)pLoadMesh + (u32)pLoadMesh->pMeshIS);
	u32 nRetainBlockSize = pDX8Mesh->nDisposableOffset;

	// Allocate memory for the new mesh:
	FMesh_t *pNewMesh = (FMesh_t *)fres_AlignedAlloc( nRetainBlockSize, FCLASS_BYTE_ALIGN );
	if ( !pNewMesh )
	{
		fres_ReleaseFrame( Frame );
		return NULL;
	}
	fang_MemCopy( pNewMesh, pLoadMesh, nRetainBlockSize );

	#define __FIXUP_NEW_MESH_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pNewMesh + (u32)pointer)
	#define __FIXUP_LOAD_MESH_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pLoadMesh + (u32)pointer)

	// Fixup FMesh_t pointers:
	__FIXUP_NEW_MESH_POINTER( pNewMesh->aSeg, FMeshSeg_t );
	__FIXUP_NEW_MESH_POINTER( pNewMesh->aMtl, FMeshMaterial_t );
	__FIXUP_NEW_MESH_POINTER( pNewMesh->pBoneArray, FMeshBone_t );
	__FIXUP_NEW_MESH_POINTER( pNewMesh->pnSkeletonIndexArray, u8 );
	__FIXUP_NEW_MESH_POINTER( pNewMesh->pLightArray, FMeshLight_t );
	__FIXUP_NEW_MESH_POINTER( pNewMesh->pTexLayerIDArray, FMeshTexLayerID_t );
	__FIXUP_NEW_MESH_POINTER( pNewMesh->pMeshIS, FDX8Mesh_t );

	pDX8Mesh = pNewMesh->pMeshIS;

	// Track the usage of scrolling and flipping
	pNewMesh->nTexLayerIDCount_ST = 0;
	pNewMesh->nTexLayerIDCount_Flip = 0;
	for ( i = 0; i < pNewMesh->nTexLayerIDCount; i++ ) 
	{
		if ( pNewMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_USE_ST_INFO ) 
		{
			pNewMesh->nTexLayerIDCount_ST++;
		}
			
		if ( pNewMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_USE_FLIP_INFO ) 
		{
			pNewMesh->nTexLayerIDCount_Flip++;
		}
	}
	
	for ( i = 0; i < pNewMesh->nMaterialCount; i++ ) 
	{
		__FIXUP_NEW_MESH_POINTER( pNewMesh->aMtl[i].pPlatformData, void );
		__FIXUP_NEW_MESH_POINTER( pNewMesh->aMtl[i].pnShLightRegisters, u32 );
		__FIXUP_NEW_MESH_POINTER( pNewMesh->aMtl[i].pnShSurfaceRegisters, u32 );

		FDX8MeshMaterial_t *pDX8Mat = (FDX8MeshMaterial_t *)pNewMesh->aMtl[i].pPlatformData;
		__FIXUP_NEW_MESH_POINTER( pDX8Mat->aCluster, FDX8MeshCluster_t );
		for ( ii = 0; ii < pDX8Mat->nClusterCount; ii++ )
		{
			__FIXUP_NEW_MESH_POINTER( pDX8Mat->aCluster[ii].paStripBuffer, FDX8MeshStrip_t );
		}

		fshaders_FixupLightRegisters( pNewMesh, pNewMesh->aMtl[i].nLightShaderIdx, pNewMesh->aMtl[i].nSpecularShaderIdx, pNewMesh->aMtl[i].pnShLightRegisters );
		fshaders_FixupSurfaceRegisters( pNewMesh, pNewMesh->aMtl[i].nSurfaceShaderIdx, pNewMesh->aMtl[i].pnShSurfaceRegisters );
		
		pNewMesh->aMtl[i].nDLHashKey = frs_GenerateMaterialDLHash( &pNewMesh->aMtl[i] );
	}

	__FIXUP_NEW_MESH_POINTER( pNewMesh->paCollTree, FkDOP_Tree_t );
	if ( pNewMesh->paCollTree )
	{
		for ( i = 0; i < pNewMesh->nCollTreeCount; i++ )
		{
			FkDOP_Tree_t *pCollTree = &pNewMesh->paCollTree[i];
			
			__FIXUP_NEW_MESH_POINTER( pCollTree->pakDOPNodes, FkDOP_Node_t );
			for ( ii = 0; ii < pCollTree->nTreeNodeCount; ii++ )
			{
				FkDOP_Node_t *pNode = &pCollTree->pakDOPNodes[ii];
				
				__FIXUP_NEW_MESH_POINTER( pNode->paPackets, FkDOP_TriPacket_t );
				__FIXUP_NEW_MESH_POINTER( pNode->pTriData, void );
			}
			
			__FIXUP_NEW_MESH_POINTER( pCollTree->paRootkDOPVerts, CFVec3 );
//			__FIXUP_NEW_MESH_POINTER( pCollTree->pakDOPVerts, void );
			__FIXUP_NEW_MESH_POINTER( pCollTree->paIntervals, FkDOP_Interval_t );
		}
	}

	// Fixup FMesh_t::FGCMesh_t pointers:
	pDX8Mesh->pMesh = pNewMesh; // Manually fixup this pointer, because it is always originally NULL
	__FIXUP_NEW_MESH_POINTER( pDX8Mesh->aVB, FDX8VB_t );
	__FIXUP_NEW_MESH_POINTER( pDX8Mesh->anIndicesCount, u16 );
	__FIXUP_NEW_MESH_POINTER( pDX8Mesh->apDXIB, void* );
	for ( i = 0; i < pDX8Mesh->nIBCount; i++ )
	{
		__FIXUP_LOAD_MESH_POINTER( pDX8Mesh->apDXIB[i], void );

		if ( pDX8Mesh->apDXIB[i] && pDX8Mesh->anIndicesCount[i] )
		{
			// Build a DX index buffer
			IDirect3DIndexBuffer8 *pActualIB;
			u32 nIBSize = pDX8Mesh->anIndicesCount[i] * sizeof( u16 );
			hResult = FDX8_pDev->CreateIndexBuffer( nIBSize, 
										D3DUSAGE_WRITEONLY, 
										D3DFMT_INDEX16, 
										D3DPOOL_MANAGED, 
										&pActualIB );

			if ( hResult != D3D_OK )
			{
				fres_ReleaseFrame( Frame );
				return NULL;
			}

			if ( pActualIB->Lock( 0, 0, (void **)&pData, 0 ) != D3D_OK )
			{
				fres_ReleaseFrame( Frame );
				return NULL;
			}
			fang_MemCopy( pData, pDX8Mesh->apDXIB[i], nIBSize );
			pActualIB->Unlock();

			pDX8Mesh->apDXIB[i] = pActualIB;
		}
	}

	pDX8Mesh->apCollVertBuffer = (CFVec3 **)fres_Alloc( sizeof( CFVec3 *) * pDX8Mesh->nVBCount );

	for ( i = 0; i < pDX8Mesh->nVBCount; i++ )
	{
#if FANG_WINGC
		// The diffuse index table lives in the retained block, like aVB itself.
		__FIXUP_NEW_MESH_POINTER( pDX8Mesh->aVB[i].pGCDiffuseIndex, u16 );
#endif
		__FIXUP_LOAD_MESH_POINTER( pDX8Mesh->aVB[i].pLMUVStream, FDX8LightMapST_t );
		
		if ( pDX8Mesh->aVB[i].pLMUVStream )
		{
			// Build a DX vertex buffer
			u32 nSTBytes = sizeof(FDX8LightMapST_t) * pDX8Mesh->aVB[i].nLMTCCount * pDX8Mesh->aVB[i].nVtxCount;
			IDirect3DVertexBuffer8 *pSTBuffer;
			hResult = FDX8_pDev->CreateVertexBuffer( nSTBytes, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &pSTBuffer );
			if ( hResult != D3D_OK )
			{
				// Failed to create VB...
				fres_ReleaseFrame( Frame );
				return NULL;
			}

			pSTBuffer->Lock( 0, 0, (void **)&pData, 0 );
			if ( !pData )
			{
				fres_ReleaseFrame( Frame );
				return NULL;
			}
			fang_MemCopy( pData, pDX8Mesh->aVB[i].pLMUVStream, nSTBytes );

			pSTBuffer->Unlock();
			pDX8Mesh->aVB[i].pLMUVStream = pSTBuffer;
		}

		__FIXUP_LOAD_MESH_POINTER( pDX8Mesh->aVB[i].pBasisStream, FDX8BasisVectors_t );
		if ( pDX8Mesh->aVB[i].pBasisStream )
		{
			// Build a DX vertex buffer
			u32 nBasisBytes = sizeof(FDX8BasisVectors_t) * pDX8Mesh->aVB[i].nVtxCount;
			IDirect3DVertexBuffer8 *pBasisBuffer;
			hResult = FDX8_pDev->CreateVertexBuffer( nBasisBytes, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &pBasisBuffer );
			if ( hResult != D3D_OK )
			{
				// Failed to create VB...
				fres_ReleaseFrame( Frame );
				return NULL;
			}

			pBasisBuffer->Lock( 0, 0, (void **)&pData, 0 );
			if ( !pData )
			{
				fres_ReleaseFrame( Frame );
				return NULL;
			}
			fang_MemCopy( pData, pDX8Mesh->aVB[i].pBasisStream, nBasisBytes );

			pBasisBuffer->Unlock();
			pDX8Mesh->aVB[i].pBasisStream = pBasisBuffer;
		}

		__FIXUP_LOAD_MESH_POINTER( pDX8Mesh->aVB[i].pDXVB, IDirect3DVertexBuffer8 );

		// Build a DX vertex buffer
		u8 *pActualVerts = (u8 *)pDX8Mesh->aVB[i].pDXVB;
		u32 nVBSize = pDX8Mesh->aVB[i].nVtxCount * pDX8Mesh->aVB[i].nBytesPerVertex;
		if( !fdx8vb_CreateFVF( &pDX8Mesh->aVB[i], 
								pDX8Mesh->aVB[i].nVtxCount, 
								pDX8Mesh->aVB[i].nInfoIndex, 
								FALSE, 
								FALSE ) ) 
		{
				// Failed to create VB...
				fres_ReleaseFrame( Frame );
				return NULL;
		}

		// Copy the vert data into the vert buffer
		pData = (BYTE *)fdx8vb_LockEntireVB( &pDX8Mesh->aVB[i] );
		if ( !pData )
		{
			fres_ReleaseFrame( Frame );
			return NULL;
		}
		fang_MemCopy( pData, pActualVerts, nVBSize );
		fdx8vb_Unlock( &pDX8Mesh->aVB[i] );

		// Setup the collision vert buffer
		pDX8Mesh->apCollVertBuffer[i] = (CFVec3 *)fres_Alloc( sizeof(CFVec3) * pDX8Mesh->aVB[i].nVtxCount );
		if ( !pDX8Mesh->apCollVertBuffer[i] )
		{
			fres_ReleaseFrame( Frame );
			return NULL;
		}
		CFVec3 *pVert = pDX8Mesh->apCollVertBuffer[i];
		u8 *pSource = (u8 *)pActualVerts;
		for ( ii = 0; ii < pDX8Mesh->aVB[i].nVtxCount; ii++, pVert++ )
		{
			(*pVert) = *((CFVec3 *)pSource);
			pSource += pDX8Mesh->aVB[i].nBytesPerVertex;
		}
	}

	#undef __FIXUP_NEW_MESH_POINTER
	#undef __FIXUP_LOADED_MESH_POINTER

	// Success!
	return pNewMesh;
}


//
//
//
void fdx8load_Destroy( FMesh_t *pMesh ) 
{
	FDX8Mesh_t *pMeshIS;

	FASSERT( pMesh );

	pMeshIS = pMesh->pMeshIS;
	if( pMeshIS == NULL ) 
	{
		return;
	}

	if( pMeshIS) 
	{
		u32 i;
		for( i = 0; i < pMeshIS->nVBCount; i++ ) 
		{
			fdx8vb_Destroy( &pMeshIS->aVB[i] );
		}

		for( i = 0; i < pMeshIS->nIBCount; i++ ) 
		{
			if( pMeshIS->apDXIB[i] ) 
			{
				((IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[i])->Release();
				pMeshIS->apDXIB[i] = NULL;
			}
		}
	}
}

